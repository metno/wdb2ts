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
#include <contenthandlers/LocationForecast/LocationForecastHandler.h>
#include <transactor/ProviderRefTime.h>
#include <WciWebQuery.h>
#include <replace.h>
#include <splitstr.h>
#include <trimstr.h>
#include <exception.h>
#include <UrlQuery.h>
#include <contenthandlers/LocationForecast/EncodeLocationForecast.h>
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


DECLARE_MI_PROFILE;

using namespace std;

namespace wdb2ts {

LocationForecastHandler::
LocationForecastHandler()
	: providerPriorityIsInitialized( false ),
	  projectionHelperIsInitialized( false), 
	  expireRand( 120 )
{
	// NOOP
}

LocationForecastHandler::
LocationForecastHandler( int major, int minor, const std::string &note_ )
	: HandlerBase( major, minor), note( note_ ),
	  providerPriorityIsInitialized( false ),
	  projectionHelperIsInitialized( false ), 
	  wciProtocolIsInitialized( false ),
	  expireRand( 120 )
{
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


void
LocationForecastHandler::
configureProviderPriority( const wdb2ts::config::ActionParam &params, Wdb2TsApp *app )
{
	providerPriority_ = wdb2ts::configureProviderList( params, wdbDB, app ); 
	
	providerPriorityIsInitialized = true;
	
	if( app && ! updateid.empty() ) 
		app->notes.setNote( updateid + ".LocationProviderList", 
					           new NoteProviderList( providerPriority_, true ) );
}

void
LocationForecastHandler::
extraConfigure( const wdb2ts::config::ActionParam &params, Wdb2TsApp *app ) 
{
	boost::mutex::scoped_lock lock( mutex );
			
	if( ! wciProtocolIsInitialized ) {
		wciProtocol = app->wciProtocol( wdbDB );
		
		cerr << "WCI protocol: " << wciProtocol << endl;
		
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
LocationForecastHandler::
configure( const wdb2ts::config::ActionParam &params,
		     const wdb2ts::config::Config::Query &query,
			  const std::string &wdbDB_ )
{
	const string MODEL_TOPO_PROVIDER_KEY("model_topo_provider-");
		
	Wdb2TsApp *app=Wdb2TsApp::app();
	actionParams = params;
	
	
	wdb2ts::config::ActionParam::const_iterator it=params.find("expire_rand");
	
	if( it != params.end() )  {
		try {
			expireRand = it->second.asInt();
			expireRand = abs( expireRand );
			
			cerr << "Config: expire_rand: " << expireRand << endl;
		}
		catch( const std::logic_error &ex ) {
			cerr << "expire_rand: not convertible to an int. Value given '" 
			     << ex.what() <<"'." << endl;
		}
	}
	
	it=params.find("updateid");
	
	if( it != params.end() )  
		updateid = it->second.asString();
	
	symbolGenerator.readConf( app->getConfDir()+"/qbSymbols.def" );
	
	if( ! updateid.empty() ) {
		string noteName = updateid+".LocationProviderReftimeList";
		app->notes.registerPersistentNote( noteName, new NoteProviderReftimes() );
		app->notes.registerNoteListener( updateid+".LocationProviderReftimeList", this );
	}
	
	wdbDB = wdbDB_;
	urlQuerys = query;

	if( app && ! updateid.empty() ) 
		app->notes.setNote( updateid+".LocationForecastWdbId", new NoteString( wdbDB ) );
	
	//configureProviderPriority( params, app );
	
	for( it = params.begin(); it!=params.end(); ++it ) {
		string::size_type i=it->first.find( MODEL_TOPO_PROVIDER_KEY );
		
		if( i != string::npos && i==0 ) {
			string provider=it->first;
			provider.erase(0, MODEL_TOPO_PROVIDER_KEY.size() );
			modelTopoProviders[provider]=it->second.asString();
		}
	}
	
	configureSymbolconf( params, symbolConf_ );
	metaModelConf = wdb2ts::configureMetaModelConf( params );
		
	extraConfigure( params, app );
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

		bool first=true;
		for( ProviderRefTimeList::iterator it = providerReftimes->begin();
					it != providerReftimes->end();
					++it ) {
			if( first ) {
				cerr << "noteUpdated: ProviderReftimes: " << endl;
				first = false;
			}
			
			cerr << "        " <<  it->first << ": " << it->second.refTime  << endl;
		}
	}
}

void 
LocationForecastHandler::
getProtectedData( SymbolConfProvider &symbolConf, ProviderList &providerList )
{
	boost::mutex::scoped_lock lock( mutex );
	
	providerList = providerPriority_;
	symbolConf = symbolConf_;
}


PtrProviderRefTimes 
LocationForecastHandler::
getProviderReftimes() 
{
	NoteProviderReftimes *tmp=0;
	
	Wdb2TsApp *app=Wdb2TsApp::app();
	
	{
		boost::mutex::scoped_lock lock( mutex );
	
		if( ! providerReftimes ) {
			try {
				app=Wdb2TsApp::app();
			
				providerReftimes.reset( new ProviderRefTimeList() ); 
				
				WciConnectionPtr wciConnection = app->newWciConnection( wdbDB );
				
				//miutil::pgpool::DbConnectionPtr con=app->newConnection( wdbDB );
				
				if( ! updateProviderRefTimes( wciConnection, *providerReftimes, providerPriority_, wciProtocol ) ) { 
					cerr << "LocationForecastHandler::getProviderReftime: Failed to set providerReftimes."
					     << endl;
				} else {
					tmp = new NoteProviderReftimes( *providerReftimes );
				}
			}
			catch( exception &ex ) {
				cerr << "EXCEPTION: LocationForecastHandler::getProviderReftime: "
			        << ex.what() << endl;
			}
			catch( ... ) {
				cerr << "EXCEPTION: LocationForecastHandler::getProviderReftime: unknown exception "
			        << endl;
			}
		
			bool first=true;
			for( ProviderRefTimeList::iterator it = providerReftimes->begin();
			     it != providerReftimes->end();
			     ++it ) 
			{
				if( first ) {
					cerr << "ProviderReftimes: " << endl;
					first=false;
				}
			
				cerr << "        " <<  it->first << ": " << it->second.refTime << endl;
			}
		} 
	}
	
	if( tmp ) 
		app->notes.setNote( updateid+".LocationProviderReftimeList", tmp );
	
		
	return providerReftimes;	
}

int 
LocationForecastHandler::
getAltitude( LocationData &ld )
{
	int alt;
	ld.init( boost::posix_time::ptime() );
	
	if( ! ld.hasNext() )
		return INT_MIN;
	
	LocationElem &elem= *ld.next();
	
	ProviderList::const_iterator it;
	
	boost::mutex::scoped_lock lock( mutex );
	
	for( it=providerPriority_.begin(); it != providerPriority_.end(); ++it ){
		alt = elem.modeltopography( it->providerWithPlacename() );
		
		if( alt != INT_MIN )
			return alt;
	}
	
	return INT_MIN;
}


void 
LocationForecastHandler::
get( webfw::Request  &req, 
     webfw::Response &response, 
     webfw::Logger   &logger )   
{
	using namespace boost::posix_time;
	ostringstream ost;
	ostream &out = response.out();
	int   altitude;
	//ptime fromtime;
	PtrProviderRefTimes refTimes;
	ProviderList        providerPriority;
	SymbolConfProvider  symbolConf;
	WebQuery            webQuery;
	// Initialize Profile
	INIT_MI_PROFILE(100);
	USE_MI_PROFILE;
	MARK_ID_MI_PROFILE("LocationForecastHandler");
     
	ost << endl << "URL:   " << req.urlPath() << endl 
	    << "Query: " << req.urlQuery() << endl;
     
	logger.debug( ost.str() );
    
//	cerr << "LocationForecastHandler: " << ost.str() << endl;
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
	
	cerr << "RefTimes: " << endl;
	for( ProviderRefTimeList::iterator rit=refTimes->begin(); rit != refTimes->end(); ++rit )
		cerr << "  " << rit->first << " " << rit->second.refTime << endl;
	
    
	try{
		TimeSeriePtr timeSerie = requestWdb( webQuery.latitude(), webQuery.longitude(), 
				                               altitude, refTimes, providerPriority );
   	
		/*
		if( timeSerie->empty() ){
			ost.str("");
			ost << "No data found for the query: latitude="  << latitude 
			    << " longitude=" << longitude << " altitude=" << altitude;
			response.errorDoc( ost.str() );
			response.status( webfw::Response::NOT_FOUND );
			logger.info( ost.str() );
			return;
		}
		*/
    	
		LocationData locationData( timeSerie, 
				                     webQuery.longitude(), webQuery.latitude(), altitude, 
				                     providerPriority,
				                     modelTopoProviders );
    	
		string          error;
		MARK_ID_MI_PROFILE("symboGenerator::computeSymbols");
		ProviderSymbolHolderList sh = symbolGenerator.computeSymbols( locationData, symbolConf, error );
		MARK_ID_MI_PROFILE("symboGenerator::computeSymbols");
    	
    	//MARK_ID_MI_PROFILE("preprocess");
   	  	//preprocessdata( *timeSerie );
   	  	//MARK_ID_MI_PROFILE("preprocess");
        
		if( altitude == INT_MIN )
			altitude = getAltitude( locationData );
    	
		/*DEBUG:
		for( ProviderList::iterator it=providerPriority.begin();
		     it != providerPriority.end(); ++it ) {
			cerr << "ProviderPriority: " << it->providerWithPlacename() << endl;
		}
		*/
    	
		EncodeLocationForecast encode( locationData,
   	                               &projectionHelper,
   			                         webQuery.longitude(), webQuery.latitude(), altitude,
   			                         webQuery.from(),
   			                         sh,
   			                         refTimes,
   			                         metaModelConf,
   			                         expireRand );
		MARK_ID_MI_PROFILE("encodeXML");  
		encode.encode( response );
		MARK_ID_MI_PROFILE("encodeXML");
	}
	catch( const webfw::IOError &ex ) {
		response.errorDoc( ex.what() );
        
		if( ex.isConnected() ) {
			response.status( webfw::Response::INTERNAL_ERROR );
			cerr << "get: Exception: webfw::IOError: " << ex.what() << endl;
		}
	}
	catch( const std::ios_base::failure &ex ) {
		response.errorDoc( ex.what() );
		response.status( webfw::Response::INTERNAL_ERROR );
		cerr << "get: Exception: std::ios_base::failure: " << ex.what() << endl;
	}
	catch( const logic_error &ex ){
		response.errorDoc( ex.what() );
		response.status( webfw::Response::INTERNAL_ERROR );
		cerr << "get: Exception: std::logic_error: " << ex.what() << endl;
	}
	catch( const std::exception &ex ) {
		response.errorDoc( ex.what() );
		response.status( webfw::Response::INTERNAL_ERROR );
		cerr << "get: Exception: std::exception: " << ex.what() << endl;
	}
	catch( ... ) {
		response.errorDoc("Unexpected exception!");
    	response.status( webfw::Response::INTERNAL_ERROR );
	}
	MARK_ID_MI_PROFILE("LocationForecastHandler");
  	
	PRINT_MI_PROFILE_SUMMARY( cerr );
}

TimeSeriePtr 
LocationForecastHandler::
requestWdb( float latitude, float longitude, int altitude,
		      PtrProviderRefTimes refTimes,
		      const ProviderList &providerPriority
          ) const
{	
	Wdb2TsApp *app=Wdb2TsApp::app();
	
	WciConnectionPtr wciConnection = app->newWciConnection( wdbDB );
	WciReadLocationForecast readLocationForecastTransactor(
			                               latitude, longitude, altitude, 
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
			cerr << "requestWdb: Temporary fix for seek bug #149 (" << ex.what() <<")." << endl;
			return TimeSeriePtr( new TimeSerie() );
		}
		
		throw;
	}
	
}

}
