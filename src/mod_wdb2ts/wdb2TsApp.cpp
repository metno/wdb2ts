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
#include <pgconpool/dbSetup.h>
#include <pgconpool/dbConnectionPool.h>
#include <contenthandlers/Location/LocationHandler.h>
#include <contenthandlers/LocationForecast/LocationForecastHandler.h>
#include <contenthandlers/LocationForecast/LocationForecastUpdateHandler.h>
#include <ParamDef.h>
#include <UpdateProviderReftimes.h>
#include <Map.h>
#include <ReadMapFile.h>
#include <ConfigParser.h>
#include <RequestHandlerFactory.h>
#include <transactor/Version.h>
#include <splitstr.h>
#include <Logger4cpp.h>

using namespace std;
 
namespace wdb2ts {

MISP_IMPL_APP( Wdb2TsApp );
   
Wdb2TsApp::
Wdb2TsApp()
   : webfw::App(), hightMap( 0 ), initHightMapTryed( false ), inInitHightMap( false )
{
}


void   
Wdb2TsApp::
initAction( webfw::RequestHandlerManager&  reqHandlerMgr,
            webfw::Logger &logger )
{
	ostringstream log;
	miutil::pgpool::DbDefList dbsetup;
	string buf;
	
	log << "WDB2TS_DEFAULT_SYSCONFDIR=" << WDB2TS_DEFAULT_SYSCONFDIR << ". " ;
	log << "WDB2TS_DEFAULT_SYSLOGDIR=" << WDB2TS_DEFAULT_SYSLOGDIR << ". " ;
	log << "WDB2TS_LOCALSTATEDIR=" << WDB2TS_LOCALSTATEDIR << ". ";
	
	notes.setPersistentNotePath( WDB2TS_LOCALSTATEDIR );
	
	setConfDir( WDB2TS_DEFAULT_SYSCONFDIR );
	setLogDir( WDB2TS_DEFAULT_SYSLOGDIR );
	
	logger.info( log.str() );
	log.str("");

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
         
         dbManager = DbManagerPtr( new DbManager( dbsetup ) );
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
	
	paramDefs_ = config->paramDefs;
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
			
			toConfigure->doConfigure( (*itVer)->actionParam, query, (*itVer)->wdbDB.asString("") );
			
			reqHandlerMgr.addRequestHandler( handler, it->second->path.asString() );
		}
		
		if( ! it->second->requestDefault.version.invalid() ) {
			wdb2ts::config::Version ver=it->second->requestDefault.version;
			log.str("");
			log << "Path: " << it->second->path.asString() << " Seting default version: " 
			    << ver;
			
			if( ver.majorVer>=0 && ver.minorVer>=0 ) {
				reqHandlerMgr.setDefaultRequestHandler( it->second->path.asString(),
						                                  ver.majorVer, ver.minorVer  );
			}
			
		}
	}
}

int 
Wdb2TsApp::
getHight( float latitude, float longitude )
{
	if( ! hightMap ) {
		if( initHightMapTryed ) 
			return INT_MIN;
		
		if( inInitHightMap )
			throw InInit("The HightMap file is loading!");
			
		//Try to load the mapfile.
		initHightMap();
		
		if( ! hightMap )
			return INT_MIN;
	}
	
	int alt;
	bool error; //Dummy for now.
	
	if( ! hightMap->altitude( latitude,  longitude, alt, error ) )
		return INT_MIN;
	
	if( alt == hightMap->noDataValue() )
		return 0;
	
	return alt;
}

void 
Wdb2TsApp::
initHightMap()
{
	boost::mutex::scoped_lock lock( mapMutex );
	initHightMapImpl();
}

void 
Wdb2TsApp::
initHightMapImpl()
{
	if( hightMap )
		return;
	
	if( initHightMapTryed )
		return;

	WEBFW_USE_LOGGER( "main" );
	
	string error;
	inInitHightMap = true;

	WEBFW_LOG_DEBUG( "Loading map File: '" << 	WDB2TS_MAP_FILE << "'." );
	hightMap = readMapFile( WDB2TS_MAP_FILE, error );
	
	if( hightMap ) {
		WEBFW_LOG_INFO( "Map File: '" << 	WDB2TS_MAP_FILE << "' loaded." );
	}
	else {
		WEBFW_LOG_WARN( "No Map File: '" << 	WDB2TS_MAP_FILE << "'." );
	}
	inInitHightMap = false;
	initHightMapTryed = true;
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
	
	if( major == 0 && minor <= 7 )
		return 1;
	
	if( major == 0 && minor == 8 )
		return 2;
		
	if( major == 0 && minor == 9 && patch > 1 && patch <= 4)
		return 3;
	
	if( major == 0 && minor == 9 && patch >= 5 )
		return 4;

	return 4;
}



miutil::pgpool::DbConnectionPtr
Wdb2TsApp::
newConnection(const std::string &dbid)
{
   if( ! dbManager )
      throw logic_error("The database setup is missing!");
   
   return dbManager->newConnection( dbid );

}


WciConnectionPtr
Wdb2TsApp::
newWciConnection(const std::string &dbid)
{
   if( ! dbManager )
   	throw logic_error("The database setup is missing!");
   
   return dbManager->newWciConnection( dbid );

}

} // namespace
