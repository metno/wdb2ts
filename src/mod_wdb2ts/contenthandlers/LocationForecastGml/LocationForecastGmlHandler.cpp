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

#include <limits.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/assign/list_inserter.hpp>
#include <boost/lexical_cast.hpp>
#include <contenthandlers/LocationForecastGml/LocationForecastGmlHandler.h>
#include <WciWebQuery.h>
#include <replace.h>
#include <splitstr.h>
#include <trimstr.h>
#include <exception.h>
#include <UrlQuery.h>
#include <contenthandlers/LocationForecastGml/EncodeLocationForecastGml2.h>
#include <wdb2TsApp.h>
#include <PointDataHelper.h>
#include <preprocessdata.h>
#include <wdb2tsProfiling.h>
#include <NoteStringList.h>
#include <NoteProviderList.h>
#include <NoteString.h>
#include <NoteProviderReftime.h>
#include <SymbolGenerator.h>
#include <transactor/WciReadLocationForecast.h>
#include <transactor/Topography.h>
#include <WebQuery.h>
#include <Logger4cpp.h>
#include <RequestIterator.h>

DECLARE_MI_PROFILE;

using namespace std;

namespace wdb2ts {

LocationForecastGmlHandler::
LocationForecastGmlHandler()
	:projectionHelperIsInitialized( false),
	 precipitationConfig( 0 ),
	 expireRand( 120 )
{
	// NOOP
}

LocationForecastGmlHandler::
LocationForecastGmlHandler( int major, int minor, const std::string &note_ )
	: HandlerBase( major, minor), note( note_ ), 
	  providerPriorityIsInitialized( false ),	
	  projectionHelperIsInitialized( false ), 
	  precipitationConfig( 0 ),
	  wciProtocolIsInitialized( false ),
	  expireRand( 120 )
{
	//NOOP
}

LocationForecastGmlHandler::
~LocationForecastGmlHandler()
{
	// NOOP
} 

void 
LocationForecastGmlHandler::
configureWdbProjection( const wdb2ts::config::ActionParam &params, Wdb2TsApp *app )
{
	projectionHelperIsInitialized = wdb2ts::configureWdbProjection( projectionHelper,
			                                                          params, 
			                                                          wciProtocol, 
			                                                          wdbDB, 
			                                                          app );
}

NoteProviderList*
LocationForecastGmlHandler::
configureProviderPriority( const wdb2ts::config::ActionParam &params, Wdb2TsApp *app )
{
   providerPriority_ = wdb2ts::configureProviderList( params, wdbDB, app );

   providerPriorityIsInitialized = true;

   if( app && ! updateid.empty() )
      return new NoteProviderList( providerPriority_, true );
   else
      return 0;
}


NoteProviderList*
LocationForecastGmlHandler::
doExtraConfigure(  const wdb2ts::config::ActionParam &params, Wdb2TsApp *app )
{
   NoteProviderList *noteProviderList=0;
   boost::mutex::scoped_lock lock( mutex );

   if( ! wciProtocolIsInitialized ) {
      WEBFW_USE_LOGGER( "handler" );

      wciProtocol = app->wciProtocol( wdbDB );

      WEBFW_LOG_DEBUG("WCI protocol: " << wciProtocol);

      if( wciProtocol > 0 )
         wciProtocolIsInitialized = true;
      else
         wciProtocol = 1;
   }

   if( ! projectionHelperIsInitialized ) {
      configureWdbProjection( params, app );
   }

   if( ! providerPriorityIsInitialized ) {
      noteProviderList = configureProviderPriority( params, app );
      paramDefsPtr_->resolveProviderGroups( *app, wdbDB );
      symbolConf_ = symbolConfProviderWithPlacename( params, wdbDB, app);
   }

   return noteProviderList;
}


void
LocationForecastGmlHandler::
extraConfigure( const wdb2ts::config::ActionParam &params, Wdb2TsApp *app )
{
   NoteProviderList *noteProviderList = doExtraConfigure( params, app );

   if( noteProviderList )
      app->notes.setNote( updateid + ".LocationProviderList",
                          noteProviderList );
}




bool 
LocationForecastGmlHandler::
configure( const wdb2ts::config::ActionParam &params,
		     const wdb2ts::config::Config::Query &query,
			  const std::string &wdbDB_ )
{
	const string MODEL_TOPO_PROVIDER_KEY("model_topo_provider-");
	
	Wdb2TsApp *app=Wdb2TsApp::app();
	actionParams = params;
	
	WEBFW_USE_LOGGER( "handler" );
	
	wdb2ts::config::ActionParam::const_iterator it=params.find("expire_rand");
	
	if( it != params.end() )  {
		try {
			expireRand = it->second.asInt();
			expireRand = abs( expireRand );
			
			WEBFW_LOG_DEBUG( "Config: expire_rand: " << expireRand );
		}
		catch( const std::logic_error &ex ) {
			WEBFW_LOG_ERROR( "expire_rand: not convertible to an int. Value given '" << ex.what() <<"'." );
		}
	}
	
	it=params.find("updateid");
	
	if( it != params.end() )  
		updateid = it->second.asString();
	
	symbolGenerator.readConf( app->getConfDir()+"/qbSymbols.def" );
	
	if( ! updateid.empty() ) {
		string noteName = updateid+".LocationProviderReftimeList";
		WEBFW_LOG_DEBUG( "LocationForecastGmlHandler::configure: updateid: '" << updateid << "' noteName '" << noteName << "'.");
		app->notes.registerPersistentNote( noteName, new NoteProviderReftimes() );
		app->notes.registerNoteListener( noteName, this );

	   wdb2ts::config::ActionParam::const_iterator it = params.find("provider_priority");

	   if( it != params.end() )
	      app->notes.setNote( updateid + ".LocationProviderPriority",
	                          new NoteString( it->second.asString() ) );


		app->notes.checkForUpdatedPersistentNotes();
	}
	
	wdbDB = wdbDB_;
	urlQuerys = query;

	if( app && ! updateid.empty() ) 
		app->notes.setNote( updateid+".LocationForecastWdbId", new NoteString( wdbDB ) );
	
	modelTopoProviders = configureModelTopographyProvider( params );
	topographyProviders = configureTopographyProvider( params );
	nearestHeights = NearestHeight::configureNearestHeight( params );
	
	configureSymbolconf( params, symbolConf_ );	
	metaModelConf = wdb2ts::configureMetaModelConf( params );
	precipitationConfig = ProviderPrecipitationConfig::configure( params, app );
	paramDefsPtr_.reset( new ParamDefList( app->getParamDefs() ) );
	paramDefsPtr_->setProviderList( providerListFromConfig( params ).providerWithoutPlacename() );
	
	return true;
}


void 
LocationForecastGmlHandler::
noteUpdated( const std::string &noteName, 
             boost::shared_ptr<NoteTag> note )
{
	WEBFW_USE_LOGGER( "handler" );

	if( updateid.empty() ) {
		WEBFW_LOG_DEBUG( "LocationForecastGmlHandler::noteUpdated: updateid is empty." );
		return;
	}

	string testId( updateid+".LocationProviderReftimeList" );
	boost::mutex::scoped_lock lock( mutex );
	
	if( noteName == testId ) {
		NoteProviderReftimes *refTimes = dynamic_cast<NoteProviderReftimes*>( note.get() );
	
		if( ! refTimes ) {
			WEBFW_LOG_ERROR( "LocationForecastGmlHandler::noteUpdated: Not a note we are expecting." );
			return;
		}
	
		providerReftimes.reset( new  ProviderRefTimeList( *refTimes ) );
		
		//Read in the projection data again. It may have come new models.
		projectionHelperIsInitialized = false;
		
		//Resolve the priority list again.
		providerPriorityIsInitialized = false;
		
		paramDefsPtr_.reset( new ParamDefList( *paramDefsPtr_ ) );

		std::ostringstream logMsg;
		logMsg << "noteUpdated: ProviderReftimes:\n";
		for( ProviderRefTimeList::iterator it = providerReftimes->begin(); it != providerReftimes->end(); ++it ) {
			logMsg << "        " <<  it->first << ": " << it->second.refTime  << '\n';
		}
		WEBFW_LOG_DEBUG(logMsg.str());
	}
}


void 
LocationForecastGmlHandler::
getProtectedData( SymbolConfProvider &symbolConf, 
		            ProviderList &providerList ,
		            ParamDefListPtr &paramDefsPtr)
{
	boost::mutex::scoped_lock lock( mutex );
	
	paramDefsPtr = paramDefsPtr_;
	providerList = providerPriority_;
	symbolConf = symbolConf_;
}



PtrProviderRefTimes 
LocationForecastGmlHandler::
getProviderReftimes() 
{
	NoteProviderReftimes *tmp=0;
	
	Wdb2TsApp *app=Wdb2TsApp::app();
	
	WEBFW_USE_LOGGER( "handler" );

	{
		boost::mutex::scoped_lock lock( mutex );
	
		if( ! providerReftimes ) {
			try {
				app=Wdb2TsApp::app();
			
				providerReftimes.reset( new ProviderRefTimeList() ); 
				
				WciConnectionPtr wciConnection = app->newWciConnection( wdbDB );
				
				//miutil::pgpool::DbConnectionPtr con=app->newConnection( wdbDB );
				
				if( ! updateProviderRefTimes( wciConnection, *providerReftimes, providerPriority_, wciProtocol ) ) { 
					WEBFW_LOG_ERROR( "LocationForecastGmlHandler::getProviderReftime: Failed to set providerReftimes." );
				} else {
					tmp = new NoteProviderReftimes( *providerReftimes );
				}
			}
			catch( exception &ex ) {
				WEBFW_LOG_ERROR( "EXCEPTION: LocationForecastGmlHandler::getProviderReftime: " << ex.what() );
			}
			catch( ... ) {
				WEBFW_LOG_ERROR( "EXCEPTION: LocationForecastGmlHandler::getProviderReftime: unknown exception " );
			}
		
			std::ostringstream logMsg;
			logMsg << "ProviderReftimes:\n";
			for( ProviderRefTimeList::const_iterator it = providerReftimes->begin(); it != providerReftimes->end(); ++it )
			{
				logMsg << "        " <<  it->first << ": " << it->second.refTime << '\n';
			}
			WEBFW_LOG_DEBUG(logMsg.str());
		} 
	}
	
	if( tmp ) 
		app->notes.setNote( updateid+".LocationProviderReftimeList", tmp );
	
		
	return providerReftimes;	
}


void 
LocationForecastGmlHandler::
get( webfw::Request  &req, 
     webfw::Response &response, 
     webfw::Logger   &logger )   
{
	using namespace boost::posix_time;
	ostringstream ost;
	int   altitude;
	PtrProviderRefTimes refTimes;
	ParamDefListPtr     paramDefPtr;
	ProviderList        providerPriority;
	SymbolConfProvider  symbolConf;
	WebQuery            webQuery;

	WEBFW_USE_LOGGER( "handler" );
	
	// Initialize Profile
	INIT_MI_PROFILE(100);
	USE_MI_PROFILE;
	MARK_ID_MI_PROFILE("LocationForecastGmlHandler");
     
	ost << endl << "\tURL:   " << req.urlPath() << endl
	    << "\tQuery: " << req.urlQuery();
     
	WEBFW_LOG_DEBUG( ost.str() );
    
//	WEBFW_LOG_DEBUG( "LocationForecastGmlHandler: " << ost.str() );
	try { 
		MARK_ID_MI_PROFILE("decodeQuery");
		webQuery = WebQuery::decodeQuery( req.urlQuery() );
		altitude = webQuery.altitude();

		if( !webQuery.isPolygon() && webQuery.skip()<0 )
		   WEBFW_LOG_ERROR("Ignoring skip. Only valid for polygon request.");
		MARK_ID_MI_PROFILE("decodeQuery");
	}
	catch( const std::exception &ex ) {
		logger.error( ex.what() );
		response.errorDoc( ost.str() );
		response.status( webfw::Response::INVALID_QUERY );
		return;
	}
     
	Wdb2TsApp *app=Wdb2TsApp::app();

	extraConfigure( actionParams, app );
	app->notes.checkForUpdatedPersistentNotes();
	
	if( altitude == INT_MIN ) {
		try {
			MARK_ID_MI_PROFILE("getHight");
    		altitude = app->getHight( webQuery.latitude(), webQuery.longitude() );
    		MARK_ID_MI_PROFILE("getHight");
    	}
    	catch( const InInit &ex ) {
    		using namespace boost::posix_time;
   		
    		ptime retryAfter( second_clock::universal_time() );
    		retryAfter += seconds( 30 );
    		response.serviceUnavailable( retryAfter );
    		response.status( webfw::Response::SERVICE_UNAVAILABLE );
    		logger.info( "INIT: Loading map file." );
    		MARK_ID_MI_PROFILE("getHight");
    		return;
    	}
    	catch( const logic_error &ex ) {
    		response.status( webfw::Response::INTERNAL_ERROR );
    		response.errorDoc( ex.what() );
    		logger.error( ex.what() );
    		MARK_ID_MI_PROFILE("getHight");
    		return;
    	}
	}
	
	refTimes = getProviderReftimes();
	getProtectedData( symbolConf, providerPriority, paramDefPtr );
	removeDisabledProviders( providerPriority, *refTimes );
    
	try{

	   LocationPointListPtr locationPoints;

	   if( webQuery.isPolygon() )
	      locationPoints = getPolygonPoints( webQuery, providerPriority, *refTimes, paramDefPtr );
	   else
	      locationPoints = LocationPointListPtr( new LocationPointList( webQuery.locationPoints() ) );

	   {
	      ostringstream log;
	      log << "Requested Locations: ";
	      for( LocationPointList::iterator it=locationPoints->begin(); it != locationPoints->end(); ++it )
	         log << endl << "\t" << it->latitude() << " " << it->longitude();

	      WEBFW_LOG_DEBUG( log.str() );
	   }

	   RequestIterator reqit( app, wdbDB, wciProtocol, urlQuerys, nearestHeights, locationPoints, webQuery.to(),
	                          webQuery.isPolygon(), altitude, refTimes, paramDefPtr, providerPriority );

		EncodeLocationForecastGml2 encode( reqit,
		                                   &projectionHelper,
		                                   webQuery.from(),
		                                   metaModelConf,
		                                   precipitationConfig,
		                                   modelTopoProviders,
		                                   topographyProviders,
		                                   symbolConf,
		                                   expireRand );
		encode.schema( schema );
		MARK_ID_MI_PROFILE("encodeXML");  
		encode.encode( response );
		MARK_ID_MI_PROFILE("encodeXML");
	}
	catch( const webfw::IOError &ex ) {
		response.errorDoc( ex.what() );
        
		if( ex.isConnected() ) {
			response.status( webfw::Response::INTERNAL_ERROR );
			WEBFW_LOG_ERROR( "get: Exception: webfw::IOError: " << ex.what() );
		}
	}
	catch( const std::ios_base::failure &ex ) {
		response.errorDoc( ex.what() );
		response.status( webfw::Response::INTERNAL_ERROR );
		WEBFW_LOG_ERROR( "get: Exception: std::ios_base::failure: " << ex.what() );
	}
	catch( const logic_error &ex ){
		response.errorDoc( ex.what() );
		response.status( webfw::Response::INTERNAL_ERROR );
		WEBFW_LOG_ERROR( "get: Exception: std::logic_error: " << ex.what() );
	}
	catch( const std::exception &ex ) {
		response.errorDoc( ex.what() );
		response.status( webfw::Response::INTERNAL_ERROR );
		WEBFW_LOG_ERROR( "get: Exception: std::exception: " << ex.what() );
	}
	catch( ... ) {
		response.errorDoc("Unexpected exception!");
    	response.status( webfw::Response::INTERNAL_ERROR );
	}
	MARK_ID_MI_PROFILE("LocationForecastGmlHandler");
  	
	// TODO: Pass a logging stream to this
	PRINT_MI_PROFILE_SUMMARY( cerr );
}

LocationPointDataPtr
LocationForecastGmlHandler::
requestWdb( const LocationPointList &locationPoints,
		    const boost::posix_time::ptime &to,
		    bool isPolygon, int altitude,
		    PtrProviderRefTimes refTimes,
		    ParamDefListPtr paramDefs,
		    const ProviderList &providerPriority
          ) const
{	
	Wdb2TsApp *app=Wdb2TsApp::app();
	
	WciConnectionPtr wciConnection = app->newWciConnection( wdbDB );
	WciReadLocationForecast readLocationForecastTransactor(
			                               locationPoints, to, isPolygon, altitude,
			                               *paramDefs, refTimes, providerPriority, urlQuerys,
			                               wciProtocol
			                           );
	try { 
		wciConnection->perform( readLocationForecastTransactor );
		return readLocationForecastTransactor.result();
	}
	catch( const exception &ex ) {
	
		/*NOTE: fix for bug #149. Remove when fixed. */
		string exWhat( ex.what() );
					
		string::size_type i = exWhat.find( "nvalid seek offset:" );
					
		
		if( i != string::npos ) {
			WEBFW_USE_LOGGER( "handler" );
			WEBFW_LOG_WARN( "requestWdb: Temporary fix for seek bug #149 (" << ex.what() <<")." );
			return LocationPointDataPtr( new LocationPointData() );
		}
		
		throw;
	}
	
}


LocationPointListPtr
LocationForecastGmlHandler::
getPolygonPoints( const WebQuery &webQuery,
                  const ProviderList &providerPriority,
                  const ProviderRefTimeList &refTimes,
                  ParamDefListPtr params)
{
   WEBFW_USE_LOGGER( "handler" );
	Wdb2TsApp *app=Wdb2TsApp::app();
	ParamDefPtr itParam;
	ParamDef modelTopoParam;
	LocationPointListPtr modelTopoLocations;
	boost::posix_time::ptime dataRefTime;
	boost::posix_time::ptime modelTopoRefTime;
	boost::posix_time::ptime topoRefTime;


	TopoProviderMap::const_iterator itTopoProvider;
	string provider;
	WciConnectionPtr wciConnection = app->newWciConnection( wdbDB );

	LocationPointListPtr locations( new LocationPointList( webQuery.locationPoints() ) );

	for( ProviderList::const_iterator it = providerPriority.begin(); it != providerPriority.end(); ++it ) {
        itTopoProvider = modelTopoProviders.find( it->providerWithPlacename() );

        if( itTopoProvider != modelTopoProviders.end() )
           provider = ProviderList::decodeItem( *itTopoProvider->second.begin() ).provider;

        if( provider.empty() ) {
           itTopoProvider = modelTopoProviders.find( it->provider );

           if( itTopoProvider != modelTopoProviders.end() )
              provider = ProviderList::decodeItem( *itTopoProvider->second.begin() ).provider;
        }

        if( provider.empty() )
           provider = it->provider;

        if( provider.empty() )
           continue;



        if( ! params->findParam( itParam,  "MODEL.TOPOGRAPHY", provider ) ) {
           WEBFW_LOG_WARN( "getPolygonPoints: No parameter definition for MODEL.TOPOGRAPHY, provider '" << provider << "'.");
           continue;
        }

        modelTopoParam = *itParam;

        if( ! refTimes.providerReftime( provider, modelTopoRefTime ) ) {
           WEBFW_LOG_WARN( "getPolygonPoints: No reference times found for provider '" << provider << "'.");
           continue;
        }


        try{
           Topography topographyTransactor( locations,
                                            webQuery.skip(),
                                            modelTopoParam,
                                            provider,
                                            modelTopoRefTime,
                                            wciProtocol );

           wciConnection->perform( topographyTransactor );
           modelTopoLocations = topographyTransactor.locations();

           if( modelTopoLocations->size() == 0 ) {
              WEBFW_LOG_WARN( "getPolygonPoints: No modelTopo location found for MODEL.TOPOGRAPHY, provider '" << provider << "'.");
              continue;
           }

           return modelTopoLocations;
        }
        catch( const exception &ex ) {
           WEBFW_LOG_WARN( "getPolygonPoints: EXCEPTION: " << ex.what()  );
           continue;
        }
	}

	return LocationPointListPtr( new LocationPointList( ) );
}


}
