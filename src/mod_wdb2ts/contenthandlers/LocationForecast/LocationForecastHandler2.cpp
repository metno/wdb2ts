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
#include <contenthandlers/LocationForecast/LocationForecastHandler2.h>
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
#include <contenthandlers/LocationForecast/EncodeLocationForecast4.h>
#include <wdb2TsApp.h>
#include <PointDataHelper.h>
#include <preprocessdata.h>
#include <wdb2tsProfiling.h>
#include <NoteStringList.h>
#include <NoteProviderList.h>
#include <NoteString.h>
#include <NoteProviderReftimeByDbId.h>
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
#include <WeatherSymbol.h>
#include "statd/statsd_client.h"

DECLARE_MI_PROFILE;

using namespace std;

namespace wdb2ts {

LocationForecastHandler2::
LocationForecastHandler2()
	: subversion( 0 ),
	  noteIsUpdated( false ),
      providerPriorityIsInitialized( false ),
	  projectionHelperIsInitialized( false), 
	  precipitationConfig( 0 )
{
	// NOOP
}

LocationForecastHandler2::
LocationForecastHandler2( int major, int minor, const std::string &note_ )
	: HandlerBase( major, minor),
	  note( note_ ),
	  subversion( 0 ),
	  noteIsUpdated( false ),
	  providerPriorityIsInitialized( false ),
	  projectionHelperIsInitialized( false ), 
	  precipitationConfig( 0 ),
	  wciProtocolIsInitialized( false )
{
	if( ! note.empty() ) {
		int n;

		if( sscanf( note.c_str(), "%d", &n) == 1  )
			subversion = n;
	}
}

LocationForecastHandler2::
~LocationForecastHandler2()
{
	// NOOP
} 

void 
LocationForecastHandler2::
configureWdbProjection( const wdb2ts::config::ActionParam &params, Wdb2TsApp *app )
{
	projectionHelperIsInitialized = wdb2ts::configureAllWdbProjection( projectionHelper,
			                                                          params, 
			                                                          wciProtocol, 
			                                                          definedDbIds,
			                                                          app );
}

NoteProviderList*
LocationForecastHandler2::
configureProviderPriority( const wdb2ts::config::ActionParam &params, Wdb2TsApp *app )
{
   providerPriority_ = wdb2ts::configureProviderList( params, definedDbIds, app );

   providerPriorityIsInitialized = true;

   if( app && ! updateid.empty() )
      return new NoteProviderList( providerPriority_, true );
   else
      return 0;
}

void
LocationForecastHandler2::
clearConfig(Wdb2TsApp *app)
{
	boost::mutex::scoped_lock lock( mutex );
	wciProtocolIsInitialized=false;
	projectionHelperIsInitialized=false;
	providerPriorityIsInitialized = false;
	queryMaker.reset();
	noteIsUpdated=false;

	if( !providerRefTimesByDbIdIsPersisten )
		providerReftimesByDbId.reset(new ProviderRefTimesByDbId());

	app->releaseAllConnections();
}


NoteProviderList*
LocationForecastHandler2::
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
      //symbolConf_ = symbolConfProviderWithPlacename( params, wdbDB, app);
      symbolConf_ = symbolConfProviderWithPlacename( params, definedDbIds, app);
   }

   if( ! queryMaker ) {
	   queryMaker.reset( qmaker::QueryMaker::create( *paramDefsPtr_->idDefsParams, wciProtocol ) );
   }

   if( noteIsUpdated ) {
	   WEBFW_USE_LOGGER( "update" );
	   noteIsUpdated = false;
   }

   return noteProviderList;
}


void
LocationForecastHandler2::
extraConfigure( const wdb2ts::config::ActionParam &params, Wdb2TsApp *app )
{
   NoteProviderList *noteProviderList = doExtraConfigure( params, app );
   getOrLoadProviderReftimesByDbId();

   if( noteProviderList )
      app->notes.setNote( updateid + ".LocationProviderList",
                          noteProviderList );
}


bool 
LocationForecastHandler2::
configure( const wdb2ts::config::ActionParam &params,
		     const wdb2ts::config::Config::Query &query,
			  const std::string &wdbDB_ )
{
	const string MODEL_TOPO_PROVIDER_KEY("model_topo_provider-");
		
	//Initialize the symbolgenerator.
	WeatherSymbolGenerator::init();
	Wdb2TsApp *app=Wdb2TsApp::app();
	actionParams = params;

	//Create a logger file for the wetbulb logger.
	WEBFW_CREATE_LOGGER_FILE("wetbulb");

	//create a logfile for update.
	WEBFW_CREATE_LOGGER_FILE("+update");
	{
		WEBFW_USE_LOGGER( "update" );
		WEBFW_SET_LOGLEVEL( log4cpp::Priority::INFO );
	}

	WEBFW_USE_LOGGER( "handler" );
	
	expireConfig = ExpireConfig::readConf(params);
	if( expireConfig.expireRand_ == INT_MAX) {
		WEBFW_LOG_DEBUG("Config: expire_rand: " << expireConfig.expireRand_);
	} else {
		WEBFW_LOG_DEBUG("Config: expire_rand: undefined");
	}
	
	noDataResponse = NoDataResponse::decode( params );
	updateid=getUpdateId(params, &providerRefTimesByDbIdIsPersisten);

	symbolGenerator.readConf( app->getConfDir()+"/qbSymbols.def" );
	
	if( ! updateid.empty() ) {
		string noteName = updateid+".LocationProviderReftimeListByDbId";
		if( providerRefTimesByDbIdIsPersisten)
			app->notes.registerPersistentNote( noteName, new NoteProviderReftimesByDbId() );
		else
			app->notes.setNote( noteName, new NoteProviderReftimesByDbId() );

		noteHelper.addNoteListener( noteName, this );

		if( params.hasKey("provider_priority") )
		   app->notes.setNote( updateid + ".LocationProviderPriority",
		                       new NoteString( params.getStr("provider_priority")));
	}
	
	wdbDB = wdbDB_;
	urlQuerys = query;
	definedDbIds=getListOfQueriesDbIds(urlQuerys, wdbDB);

	if( app && ! updateid.empty() ) 
		app->notes.setNote( updateid+".LocationForecastWdbId", new NoteString( wdbDB ) );
	
	modelTopoProviders = configureModelTopographyProvider( params );
	topographyProviders = configureTopographyProvider( params );
	nearestHeights = NearestHeight::configureNearestHeight( params, wdbDB );
	nearestLands = NearestLand::configureNearestLand( params );
	isForecast = configForcast( params );
	configureSymbolconf( params, symbolConf_ );
	metaModelConf = wdb2ts::configureMetaModelConf( params );

	precipitationConfig = ProviderPrecipitationConfig::configure( params, app );
	paramDefsPtr_.reset( new ParamDefList( getParamdef() ) );
	paramDefsPtr_->setProviderList( providerListFromConfig( params ).providerWithoutPlacename() );
	doNotOutputParams = OutputParams::decodeOutputParams( params );
	thunderInSymbols = wdb2ts::configEnableThunderInSymbols( params );

	return true;
}


void 
LocationForecastHandler2::
noteUpdated( const std::string &noteName, 
             boost::shared_ptr<NoteTag> note )
{
	WEBFW_USE_LOGGER( "handler" );

	if( updateid.empty() )
		return;

	string testId( updateid+".LocationProviderReftimeListByDbId" );
	boost::mutex::scoped_lock lock( mutex );

	if( noteName == testId ) {
		NoteProviderReftimesByDbId *refTimes = dynamic_cast<NoteProviderReftimesByDbId*>( note.get() );

		if( ! refTimes ) {
			WEBFW_LOG_ERROR("FAILED: dynamic cast.");
			return;
		}
	
		noteIsUpdated = true;
		providerReftimesByDbId.reset( new  ProviderRefTimesByDbId( *refTimes ) );

		//Read in the projection data again. It may have come new models.
		projectionHelperIsInitialized = false;
		
		//Resolve the priority list again.
		providerPriorityIsInitialized = false;

		if( paramDefsPtr_ ) {
			paramDefsPtr_.reset( new ParamDefList( *paramDefsPtr_ ) );
			queryMaker.reset(  qmaker::QueryMaker::create( *(paramDefsPtr_->idDefsParams), wciProtocol ) );
		}

		WEBFW_LOG_INFO( "NoteUpdated ( " << noteListenerId() << "): '" << noteName << "'." );
//		cerr << "NoteUpdated ( " << noteListenerId() << "): '" << noteName << "' version: "
//			 << note->version()  << "." << endl;
	}
}

std::string
LocationForecastHandler2::
noteListenerId()
{
	return getLogprefix();
}


void 
LocationForecastHandler2::
getProtectedData( SymbolConfProvider &symbolConf,
		          ProviderList &providerList,
		          ParamDefListPtr &paramDefsPtr )
{
	boost::mutex::scoped_lock lock( mutex );
	paramDefsPtr = paramDefsPtr_;
	providerList = providerPriority_;
	symbolConf = symbolConf_;
}




PtrProviderRefTimesByDbId
LocationForecastHandler2::
getOrLoadProviderReftimesByDbId()
{
	NoteProviderReftimesByDbId *tmp=0;

	Wdb2TsApp *app=Wdb2TsApp::app();

	{ //Create lock scope for mutex.
		boost::mutex::scoped_lock lock( mutex );

		WEBFW_USE_LOGGER( "handler" );

		if( ! providerReftimesByDbId || providerReftimesByDbId->empty()) {
			try {
				app=Wdb2TsApp::app();
				ProviderRefTimeList requestedProviders;
				PtrProviderRefTimesByDbId oldRefTime(new ProviderRefTimesByDbId());
				std::map<std::string,std::string> dummyStatus;
				providerReftimesByDbId = locationForecastUpdateAllDbIds(app,
						definedDbIds, requestedProviders, providerPriority_,
						oldRefTime, wciProtocol, dummyStatus);
				if( !providerReftimesByDbId ) {
					WEBFW_LOG_ERROR("LocationForecastHandler2::getOrloadProviderReftimesByDbId: Failed to set providerReftimesByDbId.");
				} else {
					tmp = new NoteProviderReftimesByDbId( *providerReftimesByDbId);
				}
			}
			catch( const miutil::pgpool::DbNoConnectionException &ex ) {
				throw;
			}
			catch( exception &ex ) {
				WEBFW_LOG_ERROR( "LocationForecastHandler2::getOrLoadProviderReftimesByDbId: " << ex.what() );
			}
			catch( ... ) {
				WEBFW_LOG_ERROR( "LocationForecastHandler2::getOrLoadProviderReftimesByDbId: unknown exception " );
			}
		}
	}

	if( tmp ) {
		app->notes.setNote( updateid+".LocationProviderReftimeListByDbId", tmp );
	}

	return providerReftimesByDbId;
}




void
LocationForecastHandler2::
doStatus( Wdb2TsApp *app,
		  webfw::Response &response,
		  ConfigDataPtr config,
		  const ProviderList &providerList,
		  ParamDefListPtr paramdef )
{
	ostringstream out;
	PtrProviderRefTimesByDbId refTimes=config->getRefererenceTimes();
	response.contentType("text/xml");
	response.directOutput( false );

	out << "<status>\n";
	out << "   <updateid>" << updateid << "</updateid>\n";

	list<string> defProviders=paramdef->getProviderList();

	out << "   <defined_dataproviders>\n";
	for( list<string>::const_iterator it=defProviders.begin(); it != defProviders.end(); ++it ) {
		if( !it->empty()) {
			ProviderItem item = ProviderItem::decode( *it );
			out << "      <dataprovider>\n";
			out << "         <name>" << item.provider << "</name>\n";
			if( ! item.placename.empty() )
				out << "         <placename>" << item.placename << "</placename>\n";
			out << "      </dataprovider>\n";
		}
	}
	out << "   </defined_dataproviders>\n";


	out << "   <resolved_dataproviders>\n";
	for( ProviderList::const_iterator it=providerList.begin(); it != providerList.end(); ++it ) {
		out << "      <dataprovider>\n";
		out << "         <name>" <<		   it->provider << "</name>\n";
		out << "         <placename>" <<		   it->placename << "</placename>\n";
		out << "      </dataprovider>\n";
		//out << "      <dataprovider>" << it->providerWithPlacename() << "</dataprovider>\n";
	}
	out << "   </resolved_dataproviders>\n";

	out << "   <referencetimes>\n";
	for (ProviderRefTimesByDbId::const_iterator itDbId = refTimes->begin();
			itDbId != refTimes->end(); ++itDbId) {
		out << "      <wdbid id=\"" << itDbId->first << "\">\n";
		for (ProviderRefTimeList::iterator rit = itDbId->second->begin();
				rit != itDbId->second->end(); ++rit) {
			ProviderItem item = ProviderItem::decode(rit->first);
			out << "      <dataprovider>\n";
			out << "         <name>" << item.provider << "</name>\n";
			out << "         <placename>" << item.placename << "</placename>\n";
			out << "         <referencetime>"
					<< miutil::isotimeString(rit->second.refTime, true, true)
					<< "</referencetime>\n";
			out << "         <updated>"
					<< miutil::isotimeString(rit->second.updatedTime, true,
							true) << "</updated>\n";
			out << "         <disabled>"
					<< (rit->second.disabled ? "true" : "false")
					<< "</disabled>\n";
			out << "         <version>" << rit->second.dataversion
					<< "</version>\n";
			out << "      </dataprovider>\n";
		}
		out << "      </wdbid>\n";
	}
	out << "   </referencetimes>\n";

	out << "</status>\n";

	response.out() << out.str();
}

void
LocationForecastHandler2::
get( webfw::Request  &req,
     webfw::Response &response,
     webfw::Logger   &log )
{
	WEBFW_USE_LOGGER( "handler" );
	bool noConnection=false;

	for(int i=0; i<2; ++i) {
		try {
			if( noConnection) {
				WEBFW_LOG_DEBUG("LocationForecastHandler2::get: Retry\n");
			}
			noConnection=false;
			return get_(req, response, log);
		}
		catch( const miutil::pgpool::DbNoConnectionException &ex) {
			WEBFW_LOG_DEBUG("LocationForecastHandler2::get: EXCEPTION: " << ex.what() <<".");
			noConnection=true;
			Wdb2TsApp *app=Wdb2TsApp::app();
			clearConfig(app);
		}
	}

	if( noConnection ){
		using namespace boost::posix_time;
		WEBFW_LOG_ERROR("No database connection, request failed.");
		ptime retryAfter( second_clock::universal_time() );
		retryAfter += seconds( 10 );
		response.serviceUnavailable( retryAfter );
		response.status( webfw::Response::SERVICE_UNAVAILABLE );
	}
}

ConfigDataPtr
LocationForecastHandler2::
getRequestConfig(const WebQuery &webQuery, wdb2ts::Wdb2TsApp *app)
{
	ConfigDataPtr configData;
	try {
		configData.reset(new ConfigData(webQuery, app));
		configData->requestMetric.startTimer();
		configData->url = webQuery.urlQuery();
		configData->parameterMap = doNotOutputParams;
		configData->throwNoData = noDataResponse.doThrow();
		configData->requestedProvider = webQuery.dataprovider();
		configData->thunder = thunderInSymbols;
		configData->expireConf=expireConfig;
		configData->isForecast = isForecast;
		configData->setReferenceTimes(getReferenceTimesByDbId());
		configData->defaultDbId=wdbDB;
		return configData;
	}
	catch( ... ){
		throw NoData();
	}

}

void 
LocationForecastHandler2::
get_( webfw::Request  &req,
     webfw::Response &response, 
     webfw::Logger   &log )
{
	using namespace boost::posix_time;
	WEBFW_USE_LOGGER( "handler" );
	ConfigDataPtr configData;
	ostringstream ost;
	int   altitude;
	ProviderList        providerPriority;
	SymbolConfProvider  symbolConf;
	WebQuery            webQuery;
	ParamDefListPtr     paramDefsPtr;

	// Initialize Profile
	INIT_MI_PROFILE(100);
	USE_MI_PROFILE;
	MARK_ID_MI_PROFILE("LocationForecastHandler2");

	ost << endl << "URL:   " << req.urlPath() << endl 
	    << "Query: " << req.urlQuery() << " subversion: " << subversion << endl;

	WEBFW_LOG_DEBUG( ost.str() );

	try { 
		MARK_ID_MI_PROFILE("decodeQuery");
		webQuery = WebQuery::decodeQuery( req.urlQuery(), req.urlPath() );
		altitude = webQuery.altitude();
		webQuery.setFromTimeIfNotSet(expireConfig.modelTimeResolution_);
		MARK_ID_MI_PROFILE("decodeQuery");
	}
	catch( const std::exception &ex ) {
		WEBFW_LOG_ERROR("get: decodeQuery: " <<  ex.what() );
		response.errorDoc( ost.str() );
		response.status( webfw::Response::INVALID_QUERY );
		return;
	}

	Wdb2TsApp *app=Wdb2TsApp::app();


	app->notes.checkForUpdatedNotes( &noteHelper );
	extraConfigure( actionParams, app );
	configData=getRequestConfig(webQuery, app);


	//PtrProviderRefTimes refTimes = getProviderReftimes();
	if( ! configData->getRefererenceTimes() ) {
		using namespace boost::posix_time;

		ptime retryAfter( second_clock::universal_time() );
		retryAfter += seconds( 30 );
		response.serviceUnavailable( retryAfter );
		response.status( webfw::Response::SERVICE_UNAVAILABLE );
		return;
	}
	removeDisabledProviders( providerPriority, *configData->getRefererenceTimes() );

	getProtectedData( symbolConf, providerPriority, paramDefsPtr  );
	
	if( webQuery.isStatusRequest() ) {
		doStatus( app, response, configData, providerPriority, paramDefsPtr );
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
    
	configData->altitude = altitude;

	try{
		if( altitude == INT_MAX || altitude==INT_MIN ) {
			WEBFW_LOG_DEBUG( "Altitude: INT_MAX/INT_MIN" );
		} else {
 			WEBFW_LOG_DEBUG( "Altitude: " << altitude );
		}


		LocationPointDataPtr locationPointData;

//		cerr << "Level: '" << (webQuery.getLevel().isDefined()? webQuery.getLevel().toWdbLevelspec():string("none"))
//			 << "' Provider: '" << webQuery.dataprovider() << "'." << endl;
		if( webQuery.getLevel().isDefined() || ! webQuery.dataprovider().empty() )
			locationPointData = requestWdbSpecific( configData.get(), paramDefsPtr, providerPriority );
		else
			locationPointData = requestWdbDefault( configData.get(), paramDefsPtr, providerPriority );




		miutil::MetricTimer encodeTimer(configData->encodeMetric);
		if( subversion == 3 ) {
		   WEBFW_LOG_DEBUG("Using  encoder 'EncodeLocationForecast3'.");
		   EncodeLocationForecast3 encode( locationPointData,
                                         &projectionHelper,
                                         webQuery.longitude(), webQuery.latitude(), altitude,
                                         webQuery.from(),
										 configData->getRefererenceTimes(),
                                         metaModelConf,
                                         precipitationConfig,
                                         providerPriority,
                                         modelTopoProviders,
                                         topographyProviders,
                                         symbolConf,
                                         expireConfig.expireRand_ );

		   encode.schema( schema );
		   encode.config( configData );

		   MARK_ID_MI_PROFILE("encodeXML");
		   encode.encode( response );
		   MARK_ID_MI_PROFILE("encodeXML");
		} else if( subversion == 4 ) {
			   WEBFW_LOG_DEBUG("Using  encoder 'EncodeLocationForecast4'.");
			   EncodeLocationForecast4 encode( locationPointData,
	                                         &projectionHelper,
	                                         webQuery.longitude(), webQuery.latitude(), altitude,
	                                         webQuery.from(),
											 configData->getRefererenceTimes(),
	                                         metaModelConf,
	                                         precipitationConfig,
	                                         providerPriority,
	                                         modelTopoProviders,
	                                         topographyProviders,
	                                         symbolConf);
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
		encodeTimer.stop();
		configData->requestMetric.stopTimer();
		app->sendMetric(configData.get());
	}
	catch( const miutil::pgpool::DbNoConnectionException &ex) {
		throw;
	}
	catch( const NoData &ex ) {
	   using namespace boost::posix_time;

	   if( !configData ||
		   noDataResponse.response == NoDataResponse::ServiceUnavailable ) {
	       ptime retryAfter( second_clock::universal_time() );
	       retryAfter += seconds( 10 );
	       response.serviceUnavailable( retryAfter );
	       response.status( webfw::Response::SERVICE_UNAVAILABLE );

	       if( !configData ){
	    	   WEBFW_LOG_ERROR("ServiceUnavailable: Url: '" <<webQuery.urlQuery()<< "' NOMEM\n");
	       }
	       WEBFW_LOG_INFO( "ServiceUnavailable: Url: " << webQuery.urlQuery() );
	   } else if( noDataResponse.response == NoDataResponse::NotFound ) {
	       response.status( webfw::Response::NOT_FOUND );
	       WEBFW_LOG_INFO( "NotFound: Url: " <<  webQuery.urlQuery()  );
	   } else {
	       //response.status( webfw::Response::NO_ERROR );
	       WEBFW_LOG_INFO( "Unexpected NoData exception: Url: " <<  webQuery.urlQuery()  );
	       response.status( webfw::Response::NOT_FOUND );
	   }
	}
	catch( const miutil::pgpool::DbConnectionPoolMaxUseEx &ex ) {
	   using namespace boost::posix_time;

	   ptime retryAfter( second_clock::universal_time() );
	   retryAfter += seconds( 10 );
	   response.serviceUnavailable( retryAfter );
	   response.status( webfw::Response::SERVICE_UNAVAILABLE );
	   WEBFW_LOG_INFO( "get: Database not available or heavy loaded." );
	}
	catch( const WdbDataRequestManager::ResourceLimit &ex )
	{
		using namespace boost::posix_time;

		ptime retryAfter( second_clock::universal_time() );
		retryAfter += seconds( 10 );
		response.serviceUnavailable( retryAfter );
		response.status( webfw::Response::SERVICE_UNAVAILABLE );
		WEBFW_LOG_INFO( "get: RequestManager: Database not available or heavy loaded." );
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

	configData->requestMetric.stopTimer();
	MARK_ID_MI_PROFILE("LocationForecastHandler2");


	PRINT_MI_PROFILE_SUMMARY( cerr );
}

LocationPointDataPtr
LocationForecastHandler2::
requestWdbDefault(ConfigData *config,
				  ParamDefListPtr     paramDefsPtr,
		          const ProviderList  &providerPriority
          ) const
{
	LocationPointDataPtr locationPointData = requestWdb( config, paramDefsPtr, providerPriority );

   if( ! config->webQuery.isPolygon() ) {
      if( !locationPointData->empty() && (config->altitude==INT_MIN || config->altitude == INT_MAX)) {
            LocationData ld( locationPointData->begin()->second,
                             config->webQuery.longitude(), config->webQuery.latitude(), config->altitude,
                             providerPriority, modelTopoProviders, topographyProviders );
            config->altitude = ld.computeAndSetHeight( config->altitude );
      }
   }

	if( ! config->webQuery.isPolygon() )
		nearestHeightPoint( config, config->webQuery.locationPoints(), config->webQuery.to(),
				locationPointData,
				config->altitude, config->getReferenceTimeByDbId(config->defaultDbId)/*refTimes*/,
				paramDefsPtr, providerPriority );

	if( !config->webQuery.isPolygon() && config->webQuery.nearestLand() )
	   nearestLandPoint( config, config->webQuery.locationPoints(), config->webQuery.to(), locationPointData,
			   config->altitude, config->getReferenceTimeByDbId(config->defaultDbId)/*refTimes*/, paramDefsPtr, providerPriority );


	return locationPointData;
}

LocationPointDataPtr
LocationForecastHandler2::
requestWdbSpecific( ConfigData *config,
				    ParamDefListPtr paramDefsPtr,
		            const ProviderList &providerPriority
          ) const
{
	ProviderList providerList;
	string id("default");
	Level level = config->webQuery.getLevel();
	qmaker::QuerysAndParamDefsPtr querys;
	WdbDataRequestManager requestManager;
	Wdb2TsApp *app=Wdb2TsApp::app();


	if( config->webQuery.dataprovider().empty() ) {
		//cerr << "USING: providerPriority.\n";
		providerList = providerPriority;
	} else {
		//cerr << "USING: '" << webQuery.dataprovider()<<"'.\n";
		providerList.push_back( ProviderItem( config->webQuery.dataprovider() ) );
	}

	if( level.isDefined() ) {
		id = level.levelparametername();

		if( ! paramDefsPtr->idDefsParams->hasParamDefId( id ) )
			id = "";
	}

	//cerr << "USING PARAMDEFID: '" << id << "' (" <<level.unit() << ").\n";

	querys = queryMaker->getWdbReadQuerys( id, config, providerList);

	//cerr << *querys << endl << endl;

	return requestManager.requestData( app, querys, wdbDB,
									   config->webQuery.locationPoints(),
									   config->webQuery.from(), config->webQuery.to(),
									   config->webQuery.isPolygon(), config->altitude );
}



LocationPointDataPtr
LocationForecastHandler2::
requestWdb(ConfigData *config,
		   ParamDefListPtr  paramDefs,
		     const ProviderList &providerPriority
          ) const
{	
	WdbDataRequestManager requestManager;
	LocationPointDataPtr ret= requestManager.requestData( config,
			paramDefs, providerPriority, urlQuerys, wciProtocol );

	config->dbMetric.addToTimer(requestManager.dbDecodeMetric.getTimerSum());
	config->decodeMetric.addToTimer(requestManager.dbDecodeMetric.getTimerSum());
	config->validateMetric.addToTimer(requestManager.validateMetric.getTimerSum());
	return ret;
}


void
LocationForecastHandler2::
nearestHeightPoint( ConfigData *config,
		              const LocationPointList &locationPoints,
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
	miutil::MetricTimer time(config->dbMetric);
	NearestHeight::processNearestHeightPoint( config, locationPoints,to, data, altitude,
			                                  providerPriority, *params, nearestHeights,
			                                  wciProtocol);

}

void
LocationForecastHandler2::
nearestLandPoint( ConfigData *config,
		            const LocationPointList &locationPoints,
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

   miutil::MetricTimer time(config->dbMetric);
   NearestLand nearestLandPoint( locationPoints,to, data, altitude, refTimes,
                                 providerPriority, *params, nearestLands,
                                 wciProtocol, wciConnection );

   nearestLandPoint.processNearestLandPoint();
}

PtrProviderRefTimesByDbId
LocationForecastHandler2::
getReferenceTimesByDbId()
{
	boost::mutex::scoped_lock lock( mutex );
	if( ! providerReftimesByDbId )
		providerReftimesByDbId.reset( new ProviderRefTimesByDbId());

	return providerReftimesByDbId;
}


}
