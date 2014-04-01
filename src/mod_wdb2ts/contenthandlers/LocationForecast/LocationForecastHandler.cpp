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
#include <stdlib.h>
#include <stdio.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/assign/list_inserter.hpp>
#include <boost/lexical_cast.hpp>
#include <contenthandlers/LocationForecast/LocationForecastHandler.h>
#include <transactor/ProviderRefTime.h>
#include <transactor/Topography.h>
#include <transactor/WciReadLocationForecast.h>
#include <transactor/LocationPointRead.h>
#include <WciWebQuery.h>
#include <replace.h>
#include <splitstr.h>
#include <trimstr.h>
#include <exception.h>
#include <UrlQuery.h>
#include <contenthandlers/LocationForecast/EncodeLocationForecast.h>
#include <contenthandlers/LocationForecast/EncodeLocationForecast2.h>
#include <contenthandlers/LocationForecast/EncodeLocationForecast3.h>
#include <wdb2TsApp.h>
#include <PointDataHelper.h>
#include <preprocessdata.h>
#include <wdb2tsProfiling.h>
#include <NoteStringList.h>
#include <NoteProviderList.h>
#include <NoteString.h>
#include <NoteProviderReftime.h>
#include <SymbolGenerator.h>
#include <WebQuery.h>
#include <Logger4cpp.h>
#include <NearestHeight.h>
#include <NearestLand.h>
#include <WdbDataRequest.h>
#include <LocationData.h>
#include <configdata.h>
#include <ConfigUtils.h>
#include <ProviderGroupsResolve.h>
#include <ProviderListConfigure.h>

DECLARE_MI_PROFILE;

using namespace std;

namespace wdb2ts {

LocationForecastHandler::
LocationForecastHandler()
	: subversion( 0 ),
      providerPriorityIsInitialized( false ),
	  projectionHelperIsInitialized( false), 
	  precipitationConfig( 0 ),
	  expireRand( 120 )
{
	// NOOP
}

LocationForecastHandler::
LocationForecastHandler( int major, int minor, const std::string &note_ )
	: HandlerBase( major, minor), note( note_ ),
	  subversion( 0 ),
	  providerPriorityIsInitialized( false ),
	  projectionHelperIsInitialized( false ), 
	  precipitationConfig( 0 ),
	  wciProtocolIsInitialized( false ),
	  expireRand( 120 )
{
	if( ! note.empty() ) {
		int n;

		if( sscanf( note.c_str(), "%d", &n) == 1  )
			subversion = n;
	}
	//NOOP
}

LocationForecastHandler::
~LocationForecastHandler()
{
	// NOOP
} 

void 
LocationForecastHandler::
configureWdbProjection( const wdb2ts::config::ActionParam &params, Wdb2TsApp *app )
{
	projectionHelperIsInitialized = wdb2ts::configureWdbProjection( projectionHelper,
			                                                          params, 
			                                                          wciProtocol, 
			                                                          wdbDB, 
			                                                          app );
}

NoteProviderList*
LocationForecastHandler::
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
LocationForecastHandler::
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
      paramDefListRresolveProviderGroups( *app, *paramDefsPtr_, wdbDB );
      symbolConf_ = symbolConfProviderWithPlacename( params, wdbDB, app);
   }

   return noteProviderList;
}


void
LocationForecastHandler::
extraConfigure( const wdb2ts::config::ActionParam &params, Wdb2TsApp *app )
{
   NoteProviderList *noteProviderList = doExtraConfigure( params, app );

   if( noteProviderList )
      app->notes.setNote( updateid + ".LocationProviderList",
                          noteProviderList );
}

	

bool 
LocationForecastHandler::
configure( const wdb2ts::config::ActionParam &params,
		     const wdb2ts::config::Config::Query &query,
			  const std::string &wdbDB_ )
{
	const string MODEL_TOPO_PROVIDER_KEY("model_topo_provider-");
		
	Wdb2TsApp *app=Wdb2TsApp::app();
	actionParams = params;

	//Create a logger file for the wetbulb logger.
	WEBFW_CREATE_LOGGER_FILE("wetbulb");

	WEBFW_USE_LOGGER( "handler" );
	
	wdb2ts::config::ActionParam::const_iterator it=params.find("expire_rand");
	
	if( it != params.end() )  {
		try {
			expireRand = it->second.asInt();
			expireRand = abs( expireRand );

			WEBFW_LOG_DEBUG("Config: expire_rand: " << expireRand);
		}
		catch( const std::logic_error &ex ) {
			WEBFW_LOG_ERROR("expire_rand: not convertible to an int. Value given '" << ex.what() <<"'.");
		}
	}
	
	noDataResponse = NoDataResponse::decode( params );

	it=params.find("updateid");
	
	if( it != params.end() )  
		updateid = it->second.asString();
	
	symbolGenerator.readConf( app->getConfDir()+"/qbSymbols.def" );
	
	if( ! updateid.empty() ) {
		string noteName = updateid+".LocationProviderReftimeList";
		app->notes.registerPersistentNote( noteName, new NoteProviderReftimes() );
		app->notes.registerNoteListener( updateid+".LocationProviderReftimeList", this );

		wdb2ts::config::ActionParam::const_iterator it = params.find("provider_priority");

		if( it != params.end() )
		   app->notes.setNote( updateid + ".LocationProviderPriority",
		                       new NoteString( it->second.asString() ) );

	}
	
	wdbDB = wdbDB_;
	urlQuerys = query;

	if( app && ! updateid.empty() ) 
		app->notes.setNote( updateid+".LocationForecastWdbId", new NoteString( wdbDB ) );
	
	modelTopoProviders = configureModelTopographyProvider( params );
	topographyProviders = configureTopographyProvider( params );
	nearestHeights = NearestHeight::configureNearestHeight( params );
	nearestLands = NearestLand::configureNearestLand( params );

	configureSymbolconf( params, symbolConf_ );
	metaModelConf = wdb2ts::configureMetaModelConf( params );

	precipitationConfig = ProviderPrecipitationConfig::configure( params, app );
	//paramDefsPtr_.reset( new ParamDefList( app->getParamDefs() ) );
	paramDefsPtr_.reset( new ParamDefList( getParamdef() ) );
	paramDefsPtr_->setProviderList( providerListFromConfig( params ).providerWithoutPlacename() );
	doNotOutputParams = OutputParams::decodeOutputParams( params );

	return true;
}


void 
LocationForecastHandler::
noteUpdated( const std::string &noteName, 
             boost::shared_ptr<NoteTag> note )
{
	if( updateid.empty() )
		return;

	string testId( updateid+".LocationProviderReftimeList" );
	boost::mutex::scoped_lock lock( mutex );
	
	if( noteName == testId ) {
		NoteProviderReftimes *refTimes = dynamic_cast<NoteProviderReftimes*>( note.get() );
	
		if( ! refTimes )
			return;
	
		providerReftimes.reset( new  ProviderRefTimeList( *refTimes ) );

		//Read in the projection data again. It may have come new models.
		projectionHelperIsInitialized = false;
		
		//Resolve the priority list again.
		providerPriorityIsInitialized = false;

		paramDefsPtr_.reset( new ParamDefList( *paramDefsPtr_ ) );

		WEBFW_USE_LOGGER( "handler" );
		std::ostringstream logMsg;
		logMsg << "noteUpdated: ProviderReftimes:\n";
		for ( ProviderRefTimeList::const_iterator it = providerReftimes->begin();	it != providerReftimes->end(); ++it )
			logMsg << "        " <<  it->first << ": " << it->second.refTime << '\n';
		WEBFW_LOG_DEBUG(logMsg.str());
	}
}

void 
LocationForecastHandler::
getProtectedData( SymbolConfProvider &symbolConf, ProviderList &providerList, ParamDefListPtr &paramDefsPtr )
{
	boost::mutex::scoped_lock lock( mutex );
	paramDefsPtr = paramDefsPtr_;
	providerList = providerPriority_;
	symbolConf = symbolConf_;
}


PtrProviderRefTimes 
LocationForecastHandler::
getProviderReftimes() 
{
	NoteProviderReftimes *tmp=0;
	
	Wdb2TsApp *app=Wdb2TsApp::app();
	
	{ //Create lock scope for mutex.
		boost::mutex::scoped_lock lock( mutex );

		WEBFW_USE_LOGGER( "handler" );
	
		if( ! providerReftimes ) {
			try {
				app=Wdb2TsApp::app();
			
				providerReftimes.reset( new ProviderRefTimeList() ); 
				
				WciConnectionPtr wciConnection = app->newWciConnection( wdbDB );
				
				//miutil::pgpool::DbConnectionPtr con=app->newConnection( wdbDB );
				
				if( ! updateProviderRefTimes( wciConnection, *providerReftimes, providerPriority_, wciProtocol ) ) { 
					WEBFW_LOG_ERROR("LocationForecastHandler::getProviderReftime: Failed to set providerReftimes.");
				} else {
					tmp = new NoteProviderReftimes( *providerReftimes );
				}
			}
			catch( exception &ex ) {
				WEBFW_LOG_ERROR( "LocationForecastHandler::getProviderReftime: " << ex.what() );
			}
			catch( ... ) {
				WEBFW_LOG_ERROR( "LocationForecastHandler::getProviderReftime: unknown exception " );
			}
		
			std::ostringstream logMsg;
			logMsg << "ProviderReftimes:\n";
			for ( ProviderRefTimeList::const_iterator it = providerReftimes->begin(); it != providerReftimes->end(); ++it )
				logMsg << "        " <<  it->first << ": " << it->second.refTime << '\n';
			WEBFW_LOG_DEBUG(logMsg.str());
		} 
	}
	
	if( tmp ) 
		app->notes.setNote( updateid+".LocationProviderReftimeList", tmp );
	
		
	return providerReftimes;	
}

void
LocationForecastHandler::
doStatus( Wdb2TsApp *app,
		  webfw::Response &response, ConfigDataPtr config,
		  PtrProviderRefTimes refTimes, const ProviderList &providerList,
		  ParamDefListPtr paramdef )
{
	ostringstream out;
	response.contentType("text/xml");
	response.directOutput( false );

	out << "<status>\n";
	out << "   <updateid>" << updateid << "</updateid>\n";

	list<string> defProviders=paramdef->getProviderList();

	out << "   <defined_providers>\n";
	for( list<string>::const_iterator it=defProviders.begin(); it != defProviders.end(); ++it ) {
		if( !it->empty())
			out << "      <provider>" << *it << "</provider>\n";
	}
	out << "   </defined_providers>\n";


	out << "   <resolved_providers>\n";
	for( ProviderList::const_iterator it=providerList.begin(); it != providerList.end(); ++it ) {
		out << "      <provider>" << it->providerWithPlacename() << "</provider>\n";
	}
	out << "   </resolved_providers>\n";

	out << "   <reftimes>\n";
	for( ProviderRefTimeList::iterator rit=refTimes->begin(); rit != refTimes->end(); ++rit ) {
	  out << "      <provider>\n";
	  out << "         <name>" << rit->first << "</name>\n";
	  out << "         <reftime>"<< rit->second.refTime << "</reftime>\n";
	  out << "         <updated>"<< rit->second.updatedTime << "</updated>\n";
	  out << "         <disabled>" << (rit->second.disabled?"true":"false") << "</disabled>\n";
	  out << "         <version>" << rit->second.dataversion << "</version>\n";
	  out << "      </provider>\n";
	}
	out << "   </reftimes>\n";

	out << "</status>\n";

	response.out() << out.str();
}


void 
LocationForecastHandler::
get( webfw::Request  &req, 
     webfw::Response &response, 
     webfw::Logger   & )
{
	using namespace boost::posix_time;
	WEBFW_USE_LOGGER( "handler" );
	ostringstream ost;
	int   altitude;
	PtrProviderRefTimes refTimes;
	ProviderList        providerPriority;
	SymbolConfProvider  symbolConf;
	WebQuery            webQuery;
	ParamDefListPtr     paramDefsPtr

	// Initialize Profile
	INIT_MI_PROFILE(100);
	USE_MI_PROFILE;
	MARK_ID_MI_PROFILE("LocationForecastHandler");

	ost << endl << "URL:   " << req.urlPath() << endl 
	    << "Query: " << req.urlQuery() << " subversion: " << subversion << endl;
	WEBFW_LOG_DEBUG( ost.str() );

	try { 
		MARK_ID_MI_PROFILE("decodeQuery");
		webQuery = WebQuery::decodeQuery( req.urlQuery(), req.urlPath() );
		altitude = webQuery.altitude();
		MARK_ID_MI_PROFILE("decodeQuery");
	}
	catch( const std::exception &ex ) {
		WEBFW_LOG_ERROR("get: decodeQuery: " <<  ex.what() );
		response.errorDoc( ost.str() );
		response.status( webfw::Response::INVALID_QUERY );
		return;
	}

	ConfigDataPtr configData( new ConfigData() );
	configData->url = webQuery.urlQuery();
	configData->parameterMap = doNotOutputParams;
	configData->throwNoData = noDataResponse.doThrow();

	Wdb2TsApp *app=Wdb2TsApp::app();

	extraConfigure( actionParams, app );
	
	app->notes.checkForUpdatedPersistentNotes();
	refTimes = getProviderReftimes();
	getProtectedData( symbolConf, providerPriority, paramDefsPtr  );
	removeDisabledProviders( providerPriority, *refTimes );

	if( webQuery.isStatusRequest() ) {
		doStatus( app, response, configData, refTimes, providerPriority, paramDefsPtr );
		return;
	}
	
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
    		WEBFW_LOG_INFO( "get: INIT: Loading map file." );
    		MARK_ID_MI_PROFILE("getHight");
    		return;
    	}
    	catch( const logic_error &ex ) {
    		response.status( webfw::Response::INTERNAL_ERROR );
    		response.errorDoc( ex.what() );
    		WEBFW_LOG_ERROR( ex.what() );
    		MARK_ID_MI_PROFILE("getHight");
    		return;
    	}
	}
    
	try{
		if( altitude == INT_MAX || altitude==INT_MIN ) {
			WEBFW_LOG_DEBUG( "Altitude: INT_MAX/INT_MIN" );
		} else {
 			WEBFW_LOG_DEBUG( "Altitude: " << altitude );
		}

		LocationPointDataPtr locationPointData = requestWdb( webQuery.locationPoints(), webQuery.to(),
															 webQuery.isPolygon(),
				                                             altitude, refTimes, paramDefsPtr, providerPriority );

	   if( ! webQuery.isPolygon() ) {
	      if( !locationPointData->empty() && (altitude==INT_MIN || altitude == INT_MAX)) {
	            LocationData ld( locationPointData->begin()->second,
	                             webQuery.longitude(), webQuery.latitude(), altitude,
	                             providerPriority, modelTopoProviders, topographyProviders );
	            altitude = ld.computeAndSetHeight( altitude );
	      }
	   }

		if( ! webQuery.isPolygon() )
			nearestHeightPoint( webQuery.locationPoints(), webQuery.to(),locationPointData,
						        altitude, refTimes, paramDefsPtr, providerPriority );

		if( !webQuery.isPolygon() && webQuery.nearestLand() )
		   nearestLandPoint( webQuery.locationPoints(), webQuery.to(), locationPointData,
		                     altitude, refTimes, paramDefsPtr, providerPriority );

		if( subversion == 0 ) {
		   WEBFW_LOG_DEBUG("Using  encoder 'EncodeLocationForecast'.");
			EncodeLocationForecast encode( locationPointData,
						                   &projectionHelper,
									       webQuery.longitude(), webQuery.latitude(), altitude,
									       webQuery.from(),
									       refTimes,
									       metaModelConf,
									       precipitationConfig,
									       providerPriority,
									       modelTopoProviders,
									       topographyProviders,
									       symbolConf,
									       expireRand );
			encode.config( configData );
         encode.schema( schema );
			MARK_ID_MI_PROFILE("encodeXML");
			encode.encode( response );
			MARK_ID_MI_PROFILE("encodeXML");
		} else if( subversion == 2 ) {
		   WEBFW_LOG_DEBUG("Using  encoder 'EncodeLocationForecast2'.");
			EncodeLocationForecast2 encode( locationPointData,
						                    &projectionHelper,
									        webQuery.longitude(), webQuery.latitude(), altitude,
									        webQuery.from(),
									        refTimes,
									        metaModelConf,
									        precipitationConfig,
									        providerPriority,
									        modelTopoProviders,
									        topographyProviders,
									        symbolConf,
									        expireRand );
			encode.schema( schema );
			encode.config( configData );
         MARK_ID_MI_PROFILE("encodeXML");
         encode.encode( response );
         MARK_ID_MI_PROFILE("encodeXML");
		} else if( subversion == 3 ) {
		   WEBFW_LOG_DEBUG("Using  encoder 'EncodeLocationForecast3'.");
		   EncodeLocationForecast3 encode( locationPointData,
                                         &projectionHelper,
                                         webQuery.longitude(), webQuery.latitude(), altitude,
                                         webQuery.from(),
                                         refTimes,
                                         metaModelConf,
                                         precipitationConfig,
                                         providerPriority,
                                         modelTopoProviders,
                                         topographyProviders,
                                         symbolConf,
                                         expireRand );
		   encode.schema( schema );
		   encode.config( configData );

			MARK_ID_MI_PROFILE("encodeXML");
			encode.encode( response );
			MARK_ID_MI_PROFILE("encodeXML");
		} else {
			WEBFW_LOG_ERROR( "Unknown subversion: " << subversion );
			response.errorDoc( "Unknown subversion." );
			response.status( webfw::Response::CONFIG_ERROR );
		}

	}
	catch( const NoData &ex ) {
	   using namespace boost::posix_time;

	   if( noDataResponse.response == NoDataResponse::ServiceUnavailable ) {
	       ptime retryAfter( second_clock::universal_time() );
	       retryAfter += seconds( 10 );
	       response.serviceUnavailable( retryAfter );
	       response.status( webfw::Response::SERVICE_UNAVAILABLE );
	       WEBFW_LOG_INFO( "ServiceUnavailable: Url: " << webQuery.urlQuery() );
	   } else if( noDataResponse.response == NoDataResponse::NotFound ) {
	       response.status( webfw::Response::NOT_FOUND );
	       WEBFW_LOG_INFO( "NotFound: Url: " <<  webQuery.urlQuery()  );
	   } else {
	       response.status( webfw::Response::NO_ERROR );
	       WEBFW_LOG_INFO( "Unexpected NoData exception: Url: " <<  webQuery.urlQuery()  );
	       response.status( webfw::Response::NOT_FOUND );
	   }
	   return;
	}
	catch( const miutil::pgpool::DbConnectionPoolMaxUseEx &ex ) {
	   using namespace boost::posix_time;

	   ptime retryAfter( second_clock::universal_time() );
	   retryAfter += seconds( 10 );
	   response.serviceUnavailable( retryAfter );
	   response.status( webfw::Response::SERVICE_UNAVAILABLE );
	   WEBFW_LOG_INFO( "get: Database not available or heavy loaded." );
	   return;
	}
	catch( const WdbDataRequestManager::ResourceLimit &ex )
	{
		using namespace boost::posix_time;

		ptime retryAfter( second_clock::universal_time() );
		retryAfter += seconds( 10 );
		response.serviceUnavailable( retryAfter );
		response.status( webfw::Response::SERVICE_UNAVAILABLE );
		WEBFW_LOG_INFO( "get: RequestManager: Database not available or heavy loaded." );
		return;
	}
	catch( const webfw::IOError &ex ) {
		response.errorDoc( ex.what() );
        
		if( ex.isConnected() ) {
			response.status( webfw::Response::INTERNAL_ERROR );
			WEBFW_LOG_ERROR( "get: webfw::IOError: " << ex.what() );;
		}
	}
	catch( const std::ios_base::failure &ex ) {
		response.errorDoc( ex.what() );
		response.status( webfw::Response::INTERNAL_ERROR );
		WEBFW_LOG_ERROR( "get: std::ios_base::failure: " << ex.what() );;
	}
	catch( const logic_error &ex ){
		response.errorDoc( ex.what() );
		response.status( webfw::Response::INTERNAL_ERROR );
		WEBFW_LOG_ERROR( "get: std::logic_error: " << ex.what() );;
	}
	catch( const std::exception &ex ) {
		response.errorDoc( ex.what() );
		response.status( webfw::Response::INTERNAL_ERROR );
		WEBFW_LOG_ERROR( "get: std::exception: " << ex.what() );;
	}
	catch( ... ) {
	   WEBFW_LOG_ERROR( "get: .... Unexpected exception." );;
		response.errorDoc("Unexpected exception!");
    	response.status( webfw::Response::INTERNAL_ERROR );
	}
	MARK_ID_MI_PROFILE("LocationForecastHandler");

	// TODO: to logging with whis
	PRINT_MI_PROFILE_SUMMARY( cerr );
}

LocationPointDataPtr
LocationForecastHandler::
requestWdb( const LocationPointList &locationPoints,
		    const boost::posix_time::ptime &to,
	        bool isPolygon, int altitude,
		    PtrProviderRefTimes refTimes,
		    ParamDefListPtr  paramDefs,
		    const ProviderList &providerPriority
          ) const
{	
	WdbDataRequestManager requestManager;
	Wdb2TsApp *app=Wdb2TsApp::app();

	return requestManager.requestData( app, wdbDB, locationPoints, to, isPolygon, altitude,
									   refTimes, paramDefs, providerPriority, urlQuerys, wciProtocol );
}


void
LocationForecastHandler::
nearestHeightPoint( const LocationPointList &locationPoints,
			       const boost::posix_time::ptime &to,
		            LocationPointDataPtr data,
		            int altitude,
		            PtrProviderRefTimes refTimes,
		            ParamDefListPtr params,
		            const ProviderList &providerPriority
				  ) const
{

	if( nearestHeights.empty() || locationPoints.empty() )
		return;

	Wdb2TsApp *app=Wdb2TsApp::app();
	//ParamDefList params = app->paramDefs();
	WciConnectionPtr wciConnection = app->newWciConnection( wdbDB );

	NearestHeight::processNearestHeightPoint( locationPoints,to, data, altitude, refTimes,
			                                  providerPriority, *params, nearestHeights,
			                                  wciProtocol, wciConnection );

}

void
LocationForecastHandler::
nearestLandPoint( const LocationPointList &locationPoints,
                  const boost::posix_time::ptime &to,
                  LocationPointDataPtr data,
                  int altitude,
                  PtrProviderRefTimes refTimes,
                  ParamDefListPtr params,
                  const ProviderList &providerPriority
) const
{
   if( nearestLands.empty() || locationPoints.empty() ) {
      WEBFW_USE_LOGGER( "nearest_land" );
      WEBFW_LOG_DEBUG( "NearestLand: Not configured.");
      return;
   }

   Wdb2TsApp *app=Wdb2TsApp::app();
   WciConnectionPtr wciConnection = app->newWciConnection( wdbDB );

   NearestLand nearestLandPoint( locationPoints,to, data, altitude, refTimes,
                                 providerPriority, *params, nearestLands,
                                 wciProtocol, wciConnection );

   nearestLandPoint.processNearestLandPoint();


}


}
