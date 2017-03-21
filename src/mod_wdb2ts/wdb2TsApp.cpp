/*
    wdb - weather and water data storage

    Copyright (C) 2007 met.no

    Contact information:
    Norwegian Meteorological Institute
    Box 43 Blindern
    0313 OSLO
    NORWAY
    E-mail: wdb@met.no
  
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, 
    MA  02110-1301, USA
*/
#include <boost/date_time/posix_time/posix_time.hpp>
#include <wdb2TsApp.h>
#include <sstream>
#include "../pgconpool/dbSetup.h"
#include "../pgconpool/dbConnectionPool.h"
#include <contenthandlers/Location/LocationHandler.h>
#include <contenthandlers/LocationForecast/LocationForecastHandler.h>
#include <contenthandlers/LocationForecast/LocationForecastUpdateHandler.h>
#include <ParamDef.h>
#include <UpdateProviderReftimes.h>
#include <Map.h>
#include <MapLoader.h>
#include <ReadMapFile.h>
#include <ConfigParser.h>
#include <RequestHandlerFactory.h>
#include <transactor/Version.h>
#include <splitstr.h>
#include <replace.h>
#include <Logger4cpp.h>

using namespace std;
 
namespace wdb2ts {

MISP_IMPL_APP( Wdb2TsApp );
   
Wdb2TsApp::
Wdb2TsApp()
   : webfw::App(), hightMap( 0 ), loadMapThread( 0 ), initHightMapTryed( false ), inInitHightMap( false ), statsd()
{
}

void
Wdb2TsApp::
onShutdown()
{
   cerr << "----  Wdb2ts::onShutdown: called." << endl;
   boost::mutex::scoped_lock lock( mutex );
   if( dbManager ) {
      cerr << "----  Wdb2ts::onShutdown: Shutdown the dbManager. Started." << endl;
      dbManager->disable();
      cerr << "----  Wdb2ts::onShutdown: Shutdown the dbManager. Completed." << endl;
   }

}

void   
Wdb2TsApp::
initAction( webfw::RequestHandlerManager&  reqHandlerMgr,
            webfw::Logger &logger )
{
	ostringstream log;
	//miutil::pgpool::DbDefList dbsetup;
	string buf;
	
	logger.info( string("WDB2TS_DEFAULT_SYSCONFDIR=") + WDB2TS_DEFAULT_SYSCONFDIR + ".");
	logger.info( string("WDB2TS_DEFAULT_SYSLOGDIR=") + WDB2TS_DEFAULT_SYSLOGDIR + "." );
	logger.info( string("WDB2TS_DEFAULT_SYSTMPDIR=") + WDB2TS_DEFAULT_SYSTMPDIR + "." );
	logger.info( string("WDB2TS_LOCALSTATEDIR=") +  WDB2TS_LOCALSTATEDIR + ".");
	
	notes.setPersistentNotePath( WDB2TS_LOCALSTATEDIR );
	
	setConfDir( WDB2TS_DEFAULT_SYSCONFDIR );
	setLogDir( WDB2TS_DEFAULT_SYSLOGDIR );
	setTmpDir( WDB2TS_DEFAULT_SYSTMPDIR );
	
	//Start loading of the topography file.
	initHightMap();

	if( readConfFile("metno-wdb2ts-db.conf", buf) ) {
		try{
			miutil::pgpool::parseDbSetup( buf, dbsetup );
			log << "-- initAction: dbsetup " << endl;
         for( miutil::pgpool::CIDbDefList it=dbsetup.begin(); it != dbsetup.end(); ++it){
         	log.str("");
            log << "Db: " <<it->first << endl
                << "-------------------------" << endl
                << it->second << endl << endl;
         }
         
         logger.info( log.str() );
         
         //dbManager = DbManagerPtr( new DbManager( dbsetup ) );
      }
      catch( logic_error &ex ) {
         logger.warning(" -- initAction (dbsetup): Exception: " + string(ex.what()) +"\n" );
      }
   }
	
	readConfiguration( reqHandlerMgr, logger );
}

void 
Wdb2TsApp::
readConfiguration( webfw::RequestHandlerManager&  reqHandlerMgr, 
		             webfw::Logger &logger )
{
	string content;
	
	if( ! readConfFile( "metno-wdb2ts-conf.xml", content ) ) {
		logger.fatal("Cant read congiguration file <wdb2ts-conf.xml>.");
		return;
	}
	
	wdb2ts::config::ConfigParser parser;
	wdb2ts::config::Config *config = parser.parseBuf( content );
	
	if( ! config ) {
		ostringstream log;
		log << "Failed to parse <wdb2ts-conf.xml>. Reason: " << parser.getErrMsg();
		
		logger.fatal( log.str() );
		return;
	}
	cerr << "Wdb2TsApp::readConfiguration:  checkpoint 1.\n";
	paramDefs_ = config->paramdef.paramDefs();
	cerr << "Wdb2TsApp::readConfiguration:  checkpoint 2.\n";
	configureRequestsHandlers( config, reqHandlerMgr, logger );
}

void 
Wdb2TsApp::
configureRequestsHandlers( wdb2ts::config::Config *config,
		                     webfw::RequestHandlerManager&  reqHandlerMgr,
  	                        webfw::Logger &logger )
{
	using namespace wdb2ts::config;
	ostringstream log;
	
	for( RequestMap::const_iterator it = config->requests.begin();
		  it != config->requests.end();
		  ++it ) {
		
		if( ! it->second->path.defined() ) {
			logger.fatal( "A request tag without a path definition." );
			continue;
		}
		
		log.str("");
		log << "Configure: Path: " << it->second->path.asString();
		logger.info( log.str() );
		
		for( Request::RequestVersions::const_iterator itVer=it->second->requestVersions.begin();
			  itVer != it->second->requestVersions.end();
			  ++itVer ) {
			webfw::RequestHandler *handler = requestHandlerFactory( (*itVer)->action.asString(),
					                                                  (*itVer)->version );
			
			log.str("");
			
			if( ! handler ) {
				log << "No request handler with name <" << (*itVer)->action.asString() << ">.";
				logger.error( log.str() );
				continue;
			}

			handler->setPaths( getConfDir(), getLogDir(), getTmpDir() );
			handler->setModuleName( moduleName() );
			
			log << "Configure request handler <" << (*itVer)->action.asString() << "> version: " 
			    << (*itVer)->version << ".";
			logger.info( log.str() );
			
			HandlerBase *toConfigure = dynamic_cast<HandlerBase*>( handler );
			
			if( ! toConfigure ) {
				log.str("");
				log << "Dynamic cast failed (IConfigure*) <" << (*itVer)->action.asString() << ">.";
				logger.debug( log.str() );
				continue;
			}
			
			if( ! (*itVer)->queryid.defined() ) {
				log.str("");
				log << "No queryid defined for request handler <" << (*itVer)->action.asString() << "> version: " 
				    << (*itVer)->version << ".";
					logger.warning( log.str() );
			}
			
			Config::Query query = config->query( (*itVer)->queryid.asString("") );
			
			if( query.empty() ) {
				log.str("");
				log << "No db query defined for request handler <" << (*itVer)->action.asString() << "> version: " 
			       << (*itVer)->version << ".";
				logger.warning( log.str() );
			}
			
			if( ! (*itVer)->wdbDB.defined() ) {
				log.str("");
				log << "No wdb database defined for request handler <" << (*itVer)->action.asString() << "> version: " 
			       << (*itVer)->version << ".";
				logger.warning( log.str() );
			}
			
			std::string logprefix = it->second->path.asString();
			miutil::replaceString( logprefix, " ", "_");
			miutil::replaceString( logprefix, "/", ".");

			if( ! logprefix.empty() && logprefix[0]=='.' )
				logprefix.erase( 0, 1 );

			logprefix += "_" + (*itVer)->version.asString();
			toConfigure->setLogprefix( logprefix );

			if( ! reqHandlerMgr.addRequestHandler( handler, it->second->path.asString() ) ) {
				log.str("");
				log << "Failed to add request handler: '" << it->second->path.asString()
					<< "' <" << (*itVer)->action.asString() << "> version: "
					<< (*itVer)->version << ".";
				logger.error( log.str() );
				delete handler;
				continue;
			}

			toConfigure->doConfigure( *itVer, query );
		}
		
		if( ! it->second->requestDefault.version.invalid() ) {
			wdb2ts::config::Version ver=it->second->requestDefault.version;
			log.str("");
			log << "Path: " << it->second->path.asString() << " Seting default version: " 
			    << ver;
			
			if( ver.majorVer>=0 && ver.minorVer>=0 ) {
				if( ! reqHandlerMgr.setDefaultRequestHandler( it->second->path.asString(),
						                                       ver.majorVer, ver.minorVer  ) ) {
					log << " Failed!";
				}
			}
			
			logger.info( log.str() );
		}
	}
}



std::string
Wdb2TsApp::
getMapFilePath()const
{
   return WDB2TS_MAP_FILE;
}

int 
Wdb2TsApp::
getHight( float latitude, float longitude )
{

   //initHightMap();

   { //Create a scope for the lock
      boost::mutex::scoped_lock lock( mapMutex );
      if( ! hightMap ) {
         if( inInitHightMap )
            throw InInit("The HightMap file is loading!");

         return INT_MIN;
      }
   }

	int alt;
	bool error; //Dummy for now.
	
	if( ! hightMap->altitude( latitude,  longitude, alt, error ) )
		return INT_MIN;
	
	if( alt == hightMap->noDataValue() )
		return 0;
	
	return alt;
}

/**
 * This function shoul ONLY be called by initAction.
 * It starts a background thread that load the MapFile.
 */
void 
Wdb2TsApp::
initHightMap()
{
   if( ! hightMap ) {
      boost::mutex::scoped_lock lock( mapMutex );

      if( inInitHightMap )
         return;

      if( hightMap || initHightMapTryed )
         return;

      inInitHightMap = true;

      //Start a detached tread to load the topography data
      //into memory. The thread calls back on setHeightMap
      //when finished to set the hightMap and initHightMapTryed
      //status.

      boost::thread mapLoader( MapLoader( this ) );
   }
}


/**
 * This method is only called from the MapLoader thread.
 * @param map A pointer to the map.
 * @param itIsTryedToLoadTheMap True if the MapLoader thread has tryed to load the file. It my have failed.
 */
void 
Wdb2TsApp::
setHeightMap( Map *map, bool itIsTryedToLoadTheMap )
{
	boost::mutex::scoped_lock lock( mapMutex );

	hightMap = map;
	inInitHightMap = false;
	initHightMapTryed = itIsTryedToLoadTheMap;
}

int
Wdb2TsApp::
wciProtocol( const std::string &wdbid )
{
	const int defaultProtocol=1;
	Version versionTransactor;
	string version;
	WEBFW_USE_LOGGER( "main" );
	
	try {
		WciConnectionPtr wciConnection = newWciConnection( wdbid );
		wciConnection->perform( versionTransactor );
		version = *versionTransactor.result();
	}
	catch( const exception &ex ) {
		WEBFW_LOG_ERROR( "wciProtocol exception: " << ex.what() );;
		return -1;
	}
	
	WEBFW_LOG_DEBUG( "wciProtocol: version '" << version << "'." );
	
	string::size_type i = version.find_first_of("0123456789");
	
	if( i == string::npos ) {
		WEBFW_LOG_WARN( "wciProtocol: Cant find start of version number." );
		return defaultProtocol;
	}
	
	std::vector<std::string> vstr = miutil::splitstr( version.substr( i ), '.' );
	
	if( vstr.size() < 2 ) {
		WEBFW_LOG_WARN( "wciProtocol: To few version elements." );
		return defaultProtocol;
	}
	
	int major;
	int minor;
	int patch = -1;
	
	if( sscanf( vstr[0].c_str(), "%d", &major ) != 1 ||
		 sscanf( vstr[1].c_str(), "%d", &minor )	!= 1  ) {
		WEBFW_LOG_WARN( "wciProtocol: The version elements must be numbers." );
		return defaultProtocol;
	}
	
	if( vstr.size() > 2 ) {
		if( sscanf( vstr[2].c_str(), "%d", &patch ) != 1 )
			patch = -1;
	}
	
	WEBFW_LOG_INFO( "wciProtocol: Version ("<< version<<") major: " << major << " minor: " << minor << " patch: " << patch  );


	if( major == 0 && minor <= 7 )
		return 1;
	
	if( major == 0 && minor == 8 )
		return 2;
		
	if( major == 0 && minor == 9 && patch > 1 && patch <= 4)
		return 3;
	
	if( major == 0 && minor == 9 && patch == 5 )
		return 4;

	if( major == 0 && minor == 9 && patch == 6 )
		return 5;

	if( major == 0 && minor == 9 && patch >= 7 )
	   return 6;

	if( major == 1 && minor == 0 /* && patch == 0  */ )
	   return 6;


	return 6;
}


void
Wdb2TsApp::
sendMetric(const miutil::Metric &metric) {

}

void
Wdb2TsApp::
sendMetric(const ConfigDataPtr metric)
{
	miutil::CollectMetric m("wdb2ts");

	m.addTimerMetric(metric->requestMetric);
	m.addTimerMetric(metric->dbMetric);
	m.addTimerMetric(metric->validateMetric);
	m.addTimerMetric(metric->decodeMetric);
	m.addTimerMetric(metric->encodeMetric);

	if( m.hasMetrics()) {
		if( statsd.send(m.getStatdMetric())< 0 ) {
			WEBFW_USE_LOGGER( "main" );
			WEBFW_LOG_ERROR( "Metrics: can't send metrics: " << statsd.errmsg() << "\n");
			std::cerr << "Metrics (wdb2ts): can't send metrics: " << statsd.errmsg() << "\n";
		}
	}
}


void
Wdb2TsApp::
initDbPool()
{
   boost::mutex::scoped_lock lock( mutex );

   if( ! dbManager ) {
      WEBFW_USE_LOGGER( "main" );
      WEBFW_LOG_INFO("Intitializing the database pool.");

      try{
         dbManager = DbManagerPtr( new DbManager( dbsetup ) );
      }
      catch( const std::exception &ex) {
         throw logic_error( string(string("Cant initialize the database pool. Reason: ") + string(ex.what())).c_str() );
      }

      if( ! dbManager )
         throw logic_error("Cant initialize the database pool." );
   }
}


miutil::pgpool::DbConnectionPtr
Wdb2TsApp::
newConnection( const std::string &dbid, unsigned int timeoutInMilliSecound )
{
   initDbPool();
   return dbManager->newConnection( dbid );
}


WciConnectionPtr
Wdb2TsApp::
newWciConnection( const std::string &dbid, unsigned int timeoutInMilliSecound)
{
   initDbPool();
   return dbManager->newWciConnection( dbid );
}

} // namespace
