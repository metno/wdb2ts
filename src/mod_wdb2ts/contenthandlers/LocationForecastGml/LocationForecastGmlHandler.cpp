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
#include <contenthandlers/LocationForecastGml/EncodeLocationForecastGml.h>
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
#include <WebQuery.h>
#include <Logger4cpp.h>

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



void
LocationForecastGmlHandler::
configureProviderPriority( const wdb2ts::config::ActionParam &params, Wdb2TsApp *app )
{
	providerPriority_ = wdb2ts::configureProviderList( params, wdbDB, app ); 
	providerPriorityIsInitialized = true;
	
	//WEBFW_LOG_DEBUG( " ------  CheckPoint --------------------" );
	
	if( app && ! updateid.empty() ) 
		app->notes.setNote( updateid + ".LocationProviderList", 
					           new NoteProviderList( providerPriority_, false ) );
}
	


void
LocationForecastGmlHandler::
extraConfigure( const wdb2ts::config::ActionParam &params, Wdb2TsApp *app ) 
{
	boost::mutex::scoped_lock lock( mutex );

	if( ! wciProtocolIsInitialized ) {
		wciProtocol = app->wciProtocol( wdbDB );
		
		WEBFW_USE_LOGGER( "handler" );
		WEBFW_LOG_DEBUG( "WCI protocol: " << wciProtocol );
		
		if( wciProtocol > 0 )
			wciProtocolIsInitialized = true;
		else
			wciProtocol = 1;
	}

	if( ! projectionHelperIsInitialized ) {
		configureWdbProjection( params, app );
	}
	
	if( ! providerPriorityIsInitialized ) {
		configureProviderPriority( params, app );
		symbolConf_ = symbolConfProviderWithPlacename( params, wdbDB, app);
	}
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
		app->notes.checkForUpdatedPersistentNotes();
	}
	
	wdbDB = wdbDB_;
	urlQuerys = query;

	if( app && ! updateid.empty() ) 
		app->notes.setNote( updateid+".LocationForecastWdbId", new NoteString( wdbDB ) );
	
	
	//configureProviderPriority( params, app );

	/*
	for( it = params.begin(); it!=params.end(); ++it ) {
		string::size_type i=it->first.find( MODEL_TOPO_PROVIDER_KEY );
		
		if( i != string::npos && i==0 ) {
			ProviderItem item = ProviderList::decodeItem( it->first );
			string provider = item.providerWithPlacename();
			provider.erase(0, MODEL_TOPO_PROVIDER_KEY.size() );
			item = ProviderList::decodeItem( it->second.asString() );
			WEBFW_LOG_INFO( "Configure modeltopo provider alias: " << provider << " --> " << item.providerWithPlacename() );
			modelTopoProviders[provider]= item.providerWithPlacename();
		}
	}
	*/

	modelTopoProviders = configureModelTopographyProvider( params );
	topographyProviders = configureTopographyProvider( params );
	
	configureSymbolconf( params, symbolConf_ );	
	metaModelConf = wdb2ts::configureMetaModelConf( params );
	precipitationConfig = ProviderPrecipitationConfig::configure( params, app );
	extraConfigure( params, app );
	
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
		            ProviderList &providerList )
{
	boost::mutex::scoped_lock lock( mutex );
	
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
	ProviderList        providerPriority;
	SymbolConfProvider  symbolConf;
	WebQuery            webQuery;

	WEBFW_USE_LOGGER( "handler" );
	
	// Initialize Profile
	INIT_MI_PROFILE(100);
	USE_MI_PROFILE;
	MARK_ID_MI_PROFILE("LocationForecastGmlHandler");
     
	ost << endl << "URL:   " << req.urlPath() << endl 
	    << "Query: " << req.urlQuery() << endl;
     
	logger.debug( ost.str() );
    
//	WEBFW_LOG_DEBUG( "LocationForecastGmlHandler: " << ost.str() );
	try { 
		MARK_ID_MI_PROFILE("decodeQuery");
		webQuery = WebQuery::decodeQuery( req.urlQuery() );
		altitude = webQuery.altitude();
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
	getProtectedData( symbolConf, providerPriority );
	removeDisabledProviders( providerPriority, *refTimes );
    
	try{
		LocationPointDataPtr locationPointData = requestWdb( webQuery.locationPoints(),
				                                             webQuery.isPolygon(),
				                                             altitude,
											                 refTimes, providerPriority );
   	
		EncodeLocationForecastGml encode( app,
				                          locationPointData,
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
		    bool isPolygon, int altitude,
		    PtrProviderRefTimes refTimes,
		    const ProviderList &providerPriority
          ) const
{	
	Wdb2TsApp *app=Wdb2TsApp::app();
	
	WciConnectionPtr wciConnection = app->newWciConnection( wdbDB );
	WciReadLocationForecast readLocationForecastTransactor(
			                               locationPoints, isPolygon, altitude,
			                               app->paramDefs(), refTimes, providerPriority, urlQuerys,
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

}
