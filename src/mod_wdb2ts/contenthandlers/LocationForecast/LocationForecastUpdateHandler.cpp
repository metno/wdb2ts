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
#include <vector>
#include <stlContainerUtil.h>
#include <algorithm>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/assign/list_inserter.hpp>
#include <boost/regex.hpp>
#include <contenthandlers/LocationForecast/LocationForecastUpdateHandler.h>
#include <replace.h>
#include <splitstr.h>
#include <ptimeutil.h>
#include <trimstr.h>
#include <exception.h>
#include <NoteStringList.h>
#include <NoteProviderList.h>
#include <NoteString.h>
#include <NoteProviderReftime.h>
#include <Logger4cpp.h>
#include <UrlQuery.h>
#include <RequestConf.h>
#include <ProviderListConfigure.h>

using namespace std;
using namespace boost::posix_time; //ptime, second_clock

namespace wdb2ts {

LocationForecastUpdateHandler::LocationForecastUpdateHandler()
	: wciProtocol( -1 )
{
	// NOOP
}

LocationForecastUpdateHandler::
LocationForecastUpdateHandler( int major, int minor )
   : HandlerBase( major, minor), wciProtocol( -1 )
{
	
	// NOOP
}

LocationForecastUpdateHandler::
~LocationForecastUpdateHandler()
{
	// NOOP
} 



bool
LocationForecastUpdateHandler::
decodeQueryValue( const std::string &provider, const std::string &val, ProviderRefTimeList &newRefTime)
{
	const string DisableEnable = "(en|di)\\w*";
	const string Timespec = "(today|now|tomorrow|infinity|-infinity|yesterday|epoch|\\d{4}-\\d{2}-\\d{2}[ T]\\d{2}(:\\d{2}){0,2}z?)";
	const string OnlyVersion="(0)? *, *([\\-+]?\\d+)";
	const string TimeAndVersion(Timespec+"( *, *([\\-+]?\\d+))?");

	boost::regex reDisableEnable( DisableEnable, boost::regex::perl|boost::regex::icase);
	boost::regex reOnlyVersion( OnlyVersion );
	boost::regex reTimeAndVersion( TimeAndVersion, boost::regex::perl|boost::regex::icase );
	boost::smatch match;

	WEBFW_USE_LOGGER( "handler" );

	if( provider.empty() ) {
		WEBFW_LOG_ERROR( "WebQuery: Empty provider." );
		return false;
	}

	ProviderTimes &providerTimes = newRefTime[provider];
	providerTimes = ProviderTimes();

	if( val.empty() ) {
		providerTimes.reftimeUpdateRequest = true;
		return true;
	}

	if(  boost::regex_match( val, match, reDisableEnable) ) {
		providerTimes.disableEnableRequest = true;
		providerTimes.disabled = tolower(string(match[0])[0])=='d'?true:false;
	}else if( boost::regex_match( val, match, reOnlyVersion) ) {
		providerTimes.dataversion = boost::lexical_cast<int>( match[2] );
		providerTimes.dataversionRequest = true;

		if( ! string( match[1] ).empty() )  //reftime=0, Request update to latest reference time
			providerTimes.reftimeUpdateRequest = true;
	}else if( boost::regex_match( val, match, reTimeAndVersion ) ) {
		string timeSpec=match[1];
		if( ! string(match[4]).empty() ) {
			providerTimes.dataversion = boost::lexical_cast<int>( match[4]);
			providerTimes.dataversionRequest = true;
		}
		try {
			providerTimes.refTime = miutil::ptimeFromIsoString( val );
			providerTimes.reftimeUpdateRequest = true;
		}
		catch( logic_error &e ) {
			WEBFW_LOG_ERROR( "WebQuery: Invalid value. Provider <" << provider << "> value <" << val << ">. Not a valid timespec." );
			return false;
		}
	} else {
		WEBFW_LOG_ERROR( "WebQuery: Invalid value for provider '" << provider  << ". Value: '" << val << "'." );
		return false;
	}
	return true;
}

bool
LocationForecastUpdateHandler::
decodeQuery( const std::string &query, ProviderRefTimeList &newRefTime, bool &debug )
{
	webfw::UrlQuery     urlQuery;
	std::list<std::string> keys;

	WEBFW_USE_LOGGER( "handler" );

	debug=false;
	newRefTime.clear();

	if( query.empty() )
		return true;

	try {
		urlQuery.decode( query );
	}
	catch( const std::exception &ex ) {
		WEBFW_LOG_ERROR( "WebQuery: Invalid query '" + query +". Reason: " + ex.what() );
		return false;
	}

	keys = urlQuery.keys();

	if( std::count( keys.begin(), keys.end(), "debug" ) > 0 ) {
		debug = true;
		return true;
	}

	for( std::list<std::string>::const_iterator iKey=keys.begin(); iKey != keys.end(); ++iKey ) {
		string key = boost::trim_copy( *iKey );
		string val = boost::trim_copy( urlQuery.asString( *iKey, "") );

		if( key.empty() ) {
			WEBFW_LOG_ERROR( "WebQuery: Invalid key. Empty key." );;
			return false;
		}

		WEBFW_LOG_ERROR( "LocationUpdateHandler:Query: Key: '" << key << "' value: '" << val );
		ProviderItem pi=ProviderList::decodeItem( key );
		if( ! decodeQueryValue( pi.providerWithPlacename(), val, newRefTime ) ) {
			WEBFW_LOG_ERROR( "WebQuery: Key: '" << key << "' value: '" << val );
			return false;
		}
	}
	
	std::ostringstream logMsg;
	logMsg << "WebQuery: Requested providers reftime!\n";
	for( ProviderRefTimeList::const_iterator it = newRefTime.begin(); it != newRefTime.end(); ++it )
		logMsg <<"     " << it->first << ": " << it->second.refTime
		       << " dataversion (" << (it->second.dataversionRequest?"t":"f") << "): " << it->second.dataversion
		       << " disabled(" << (it->second.disableEnableRequest?"t":"f") << "): " << (it->second.disabled?"true":"false")
		       <<  '\n';
	WEBFW_LOG_DEBUG(logMsg.str());
	
	return true;
}

bool
LocationForecastUpdateHandler::
getProviderPriorityList( Wdb2TsApp *app,
		                const std::string &wdbID,
		                ProviderList &providerPriorityList )const
{
	WEBFW_USE_LOGGER( "handler" );
	wdb2ts::config::ActionParam params;

	//providerPriorityList.clear();

	boost::shared_ptr<NoteTag> note=app->notes.getNote(updateid + ".LocationProviderPriority");

	if( note ) {
		NoteString *myNote = dynamic_cast<NoteString*>( note.get() );

		if( ! myNote ) {
			WEBFW_LOG_ERROR( "getProviderPriorityList: Dynamic_cast failed <" +  updateid + ".LocationProviderPriority> Note. Check the that the 'updateid' is correct set in the config file." );
			return false;
		}

		params["provider_priority"] = *myNote;
	} else {
		WEBFW_LOG_ERROR( "getProviderPriorityList: No <" +  updateid + ".LocationProviderPriority> Note. Check that the 'updateid' is correct set in the config file." );
		return false;
	}

	providerPriorityList = configureProviderList( params, wdbID, app );

	std::ostringstream logMsg;
	logMsg << "LocationForecastUpdateHandler: providerPriorityList: " ;
	for( ProviderList::iterator it=providerPriorityList.begin();
	     it != providerPriorityList.end();
	     ++it )
		logMsg << "'" << it->providerWithPlacename() << "'";

	logMsg << endl;

	WEBFW_LOG_DEBUG( logMsg.str() );

	return true;
}


bool 
LocationForecastUpdateHandler::
getProviderPriorityList( Wdb2TsApp *app, ProviderList &providerPriorityList )const
{
	WEBFW_USE_LOGGER( "handler" );

	providerPriorityList.clear();
	
	boost::shared_ptr<NoteTag> note=app->notes.getNote(updateid + ".LocationProviderList");
		
	if( note ) {
		NoteProviderList *myNote = dynamic_cast<NoteProviderList*>( note.get() );
			
		if( myNote ) 
			providerPriorityList = *myNote;
	}
	
	std::ostringstream logMsg;
	logMsg << "LocationForecastUpdateHandler: providerPriorityList: " ;
	for( ProviderList::iterator it=providerPriorityList.begin();
	     it != providerPriorityList.end();
	     ++it )
		logMsg << "'" << it->providerWithPlacename() << "'";
			
	logMsg << endl;

	WEBFW_LOG_DEBUG( logMsg.str() );
	
	return true;
}

bool
LocationForecastUpdateHandler::
checkProviders( const ProviderList &providerList,
		        const ProviderRefTimeList &oldRefTime,
		        ProviderRefTimeList &requestedUpdate ) const
{
	WEBFW_USE_LOGGER( "handler" );
	list<string> providers=providerList.providerWithoutPlacename();
	list<string>::const_iterator pit;
	ProviderRefTimeList::const_iterator itOldRefTime;
	bool disabled;
	int dataversion;
	bool notDefinedProviders=false;
	

	//Check that the requested providers i defined in the provider_prioority.
	for( ProviderRefTimeList::iterator it = requestedUpdate.begin();
		 it != requestedUpdate.end(); ++it ) {
		ProviderList::const_iterator hasProvider = providerList.findProviderWithoutPlacename( it->first );

		if( hasProvider == providerList.end() ) {
			notDefinedProviders = true;
			WEBFW_LOG_WARN( "WARNING: LocationUpdateHandler: It is requested an update for an provider '"
					<< it->first << "' that is not in the provider_priority list.");
			it->second.dataversion = INT_MAX;
		}
	}

	if( notDefinedProviders )
		return false;

	{
		ostringstream ost;
		ost << "checkProviders:" << endl;
		ost << "providerList: " << endl;
		for( ProviderList::const_iterator plit = providerList.begin();
				plit != providerList.end(); ++plit )
			ost << "    " << plit->providerWithPlacename() << endl;


		ost << "oldRefTime: " << endl;
		for( ProviderRefTimeList::const_iterator oldRefTimeit = oldRefTime.begin();
						oldRefTimeit != oldRefTime.end(); ++oldRefTimeit )
			ost << "    " << oldRefTimeit->first << " -> " << oldRefTimeit->second.refTime
			    << " " <<  oldRefTimeit->second.updatedTime
			    << " " <<  (oldRefTimeit->second.disabled?"true":"false") << endl;

		ost << "reuquestedUpdate: " << endl;
		for( ProviderRefTimeList::const_iterator it = requestedUpdate.begin();
								it != requestedUpdate.end(); ++it )
			ost << "    " << it->first << " -> " << it->second.refTime
			    << " " <<  it->second.updatedTime
			    << " disabled(" << (it->second.disableEnableRequest?"t":"f") << "): " <<  (it->second.disabled?"true":"false")
			    << " dataversion(" << (it->second.dataversionRequest?"t":"f") << "): " << it->second.dataversion << endl;

		WEBFW_LOG_DEBUG( ost.str() );
	}


	if( requestedUpdate.empty() ) {
		for ( pit = providers.begin(); 
		      pit != providers.end();
		      ++ pit ) {
			requestedUpdate[*pit] = ProviderTimes();

			if( oldRefTime.disabled( *pit, disabled ) )
				requestedUpdate[*pit].disabled = disabled;

			if( oldRefTime.dataversion( *pit, dataversion ) )
			   requestedUpdate[*pit].dataversion = dataversion;

			 requestedUpdate[*pit].reftimeUpdateRequest = true;
		}

		ostringstream ost;
		ost << "checkProviders: return requestedUpdate: " << endl;
		for( ProviderRefTimeList::const_iterator it = requestedUpdate.begin();
				it != requestedUpdate.end(); ++it )
			ost << "    reft.:" << it->first << " -> " << it->second.refTime
			<< " updatet.:" <<  it->second.updatedTime
			<< " disabled: " <<  (it->second.disabled?"true":"false")
			<< " dataversion: " << it->second.dataversion << endl;

		WEBFW_LOG_DEBUG( ost.str() );

		return true;
	}

	{
		ostringstream ost;
		ost << "checkProviders: return requestedUpdate: " << endl;
		for( ProviderRefTimeList::const_iterator it = requestedUpdate.begin();
			 it != requestedUpdate.end(); ++it )
		   ost << "    reft.:" << it->first << " -> " << it->second.refTime
		       << " updatet.:" <<  it->second.updatedTime
		       << " disabled: " <<  (it->second.disabled?"true":"false")
		       << " dataversion: " << it->second.dataversion << endl;

		WEBFW_LOG_DEBUG( ost.str() );
	}

	return true;

}



bool 
LocationForecastUpdateHandler::
configure( const wdb2ts::config::ActionParam &params,
		     const wdb2ts::config::Config::Query &query,
			  const std::string &wdbDB_ )
{
	wdb2ts::config::ActionParam::const_iterator it=params.find("updateid");
		
	if( it != params.end() )  
		updateid = it->second.asString();
	
	wdbDB = wdbDB_;
	return true;
}



std::string 
LocationForecastUpdateHandler::
updateStatus( ProviderRefTimeList &oldRefTime, 
			     ProviderRefTimeList &newRefTime )const
{
	WEBFW_USE_LOGGER( "handler" );
	ProviderRefTimeList::const_iterator itOld;
	ProviderRefTimeList::const_iterator itNew;
	
	std::ostringstream logMsg;
	logMsg << "LocationForecastUpdateHandler: refTimes. " << endl;
	for( ProviderRefTimeList::iterator it = newRefTime.begin();
	     it != newRefTime.end();
		  ++it )
		logMsg << "  " << it->first << ": " << it->second.refTime
		       << ": " << it->second.dataversion << endl;;

	WEBFW_LOG_INFO( logMsg.str() );
	
	
	for( itOld = oldRefTime.begin(); itOld != oldRefTime.end(); ++itOld ) {
		itNew = newRefTime.find( itOld->first );
		
		if( itNew == newRefTime.end() || 
			 itOld->second.refTime != itNew->second.refTime ||
			 itOld->second.dataversion != itNew->second.dataversion ||
			 itOld->second.disabled != itNew->second.disabled )
			return "Updated";
	}
	
	//Has any new providers been added.
	for( itNew = newRefTime.begin(); itNew != newRefTime.end(); ++itNew ) {
		itOld = oldRefTime.find( itNew->first );

		if( itOld == oldRefTime.end() )
			return "Updated";
	}

	return "NoNewDataRefTime";
}

ProviderRefTimeList
LocationForecastUpdateHandler::
getProviderReftimes( Wdb2TsApp *app )
{
   WEBFW_USE_LOGGER( "handler" );
	boost::shared_ptr<NoteTag> note;
	
	note = app->notes.getNote( updateid + ".LocationProviderReftimeList" );
	
	if( ! note ) {
	   WEBFW_LOG_DEBUG("getProviderReftimes: No note for '" << updateid << ".LocationProviderReftimeList'");
		return ProviderRefTimeList();
	}
	
	NoteProviderReftimes *refTimes = dynamic_cast<NoteProviderReftimes*>( note.get() );
	
	if( ! refTimes ) {
	   WEBFW_LOG_DEBUG("getProviderReftimes: dynamic_cast failed for '" << updateid << ".LocationProviderReftimeList");
		return ProviderRefTimeList();
	}
	
	return *static_cast<ProviderRefTimeList*>(refTimes);
}

std::string
LocationForecastUpdateHandler::
getWdbId( Wdb2TsApp *app ) 
{
	if( ! wdbDB.empty() )
		return wdbDB;

	if( updateid.empty() )
		return "";
	
	WEBFW_USE_LOGGER( "handler" );

	boost::shared_ptr<NoteTag> note;
	
	//Has the LocationForecastHandler set a note
	//with the wdbID it use.   
	note = app->notes.getNote( updateid+".LocationForecastWdbId" );
		
	if( ! note )  {
		WEBFW_LOG_WARN( "getWdbId: no note!" );;
		return string();
	}
		
	NoteString *noteWdbId = dynamic_cast<NoteString*>( note.get() );
		
	if( ! noteWdbId ) {
		WEBFW_LOG_ERROR( "getWdbId <NoteString> : failed dynamic_cast <" <<note->noteType()<< ">." );;
			return string();
	}
		
	return *static_cast<string*>( noteWdbId );
}

void 
LocationForecastUpdateHandler::
checkAgainstExistingProviders( const ProviderRefTimeList &exitingProviders, 
		                         ProviderRefTimeList &newRefTime ) const
{
	ProviderRefTimeList::iterator newIt = newRefTime.begin();
	ProviderRefTimeList::const_iterator eit;
	
	while(  newIt != newRefTime.end() ) {
		eit = exitingProviders.find( newIt->first );
		
		//If the provider do not exist, delete it.
		newIt = miutil::eraseElementIf( newRefTime, newIt, eit == exitingProviders.end() );
	}
}

std::string
LocationForecastUpdateHandler::
statusDoc( const std::string &status, const std::string &comment )
{
	std::ostringstream xml;
	xml << "<?xml version=\"1.0\" ?>\n"
		<< "<update>\n"
		<< "   <comment>" << comment << "</comment>\n"
		<< "   <status>" << status << "</status>\n"
		<< "</update>";

	return xml.str();
}


void 
LocationForecastUpdateHandler::
get( webfw::Request  &req, 
     webfw::Response &response, 
     webfw::Logger   & )
{
	WEBFW_USE_LOGGER( "handler" );
	ostringstream ost;
	string status;
	ProviderRefTimeList requestedProviders;
	ProviderList providerPriorityList;
	int wciProtocol_;
	bool debug;
	WciConnectionPtr wciConnection;
	

	//Only allow one update at a time.
	boost::mutex::scoped_lock lock( mutex );

	response.contentType("text/xml");
	response.directOutput( true );
   
	
	ost << endl << "URL:   " << req.urlPath() << endl 
	<< "Query: " << req.urlQuery() << endl << "Updateid: " << updateid << endl;
   
	WEBFW_LOG_DEBUG( ost.str() );
   
	if( ! decodeQuery( req.urlQuery(), requestedProviders, debug ) ) {
		WEBFW_LOG_ERROR( ost.str() );
		response.errorDoc( ost.str() );
		response.status( webfw::Response::INVALID_QUERY );
		return;
	}

	if( updateid.empty() ) {
		std::string xml;         
		xml="<?xml version=\"1.0\" ?>\n<status>Error: missing updateid!</status>\n";
		response.errorDoc( xml ); 	
		response.status( webfw::Response::INTERNAL_ERROR );
		WEBFW_LOG_ERROR( "LocationForecatUpdate: " << ost.str() << ". Missing: updateid!" );;
		return;
	}
	

	Wdb2TsApp *app=Wdb2TsApp::app();

	//We must call checkForUpdatedPersistentNotes to ensure that
	//all notes saved to disk is read in from disk at start up.
	app->notes.checkForUpdatedNotes( 0 );
	
	try {
		ProviderRefTimeList exitingProviders;
		ProviderRefTimeList oldRefTime = getProviderReftimes( app );
		ProviderRefTimeList newRefTime( oldRefTime );
		string myWdbID = getWdbId( app );

		if( debug ) {
			app->notes.setNote( updateid + ".LocationProviderReftimeList", new NoteProviderReftimes( oldRefTime ) );
			WEBFW_LOG_DEBUG("LocationForecastUpdateHandler: (debug) note updated: '" << updateid + ".LocationProviderReftimeList." );
			std::string xml;
			xml="<?xml version=\"1.0\" ?>\n<status>Updated</status>\n";
			response.out() << xml;
			return;
		}


		if( ! getProviderPriorityList( app, myWdbID, providerPriorityList ) ) {
			std::string xml;
			xml="<?xml version=\"1.0\" ?>\n<status>Error: Missing provider priority list!</status>\n";
			response.errorDoc( xml );
			response.status( webfw::Response::INTERNAL_ERROR );
			WEBFW_LOG_ERROR( "LocationForecatUpdate: " << ost.str() << ". Missing: provider priority list!" );
			return;
		}
	
		if( ! checkProviders( providerPriorityList, oldRefTime, requestedProviders ) ) {
			response.status( webfw::Response::INVALID_QUERY );
			return;
		}
		
		
		WEBFW_LOG_DEBUG( "LocationForecastUpdateHandler: myWdbID: " << myWdbID );
		
		if( wciProtocol < 0 ) {
			wciProtocol_ = app->wciProtocol( wdbDB );
			
			if( wciProtocol_ <= 0 )
				wciProtocol_ = 1;
			else
				wciProtocol = wciProtocol_;
		} else {
			wciProtocol_ = wciProtocol;
		}
			
		try {
			wciConnection = app->newWciConnection( myWdbID );
		}
		catch( const std::exception &ex ) {
			std::string xml;
			xml="<?xml version=\"1.0\" ?>\n<status>Error: Db error!</status>\n";
			response.errorDoc( xml );
			response.status( webfw::Response::INTERNAL_ERROR );
			WEBFW_LOG_ERROR( "LocationForecatUpdate: " << ost.str() << ". Db error: '"  << ex.what() << "'" );
			return;
		}
	
		ost.str("");
		ost << "*** oldRefTime: "<< oldRefTime.size() << endl;
		for( ProviderRefTimeList::const_iterator it=oldRefTime.begin();
					     it!=oldRefTime.end();
					     ++ it )
			ost <<	" --- : " << it->first << ": refTime: " << it->second.refTime
				  << " updated: " << it->second.updatedTime
				  << " disabled: " << (it->second.disabled?"true":"false")
				  << " version: " << it->second.dataversion  << endl;

		WEBFW_LOG_DEBUG( ost.str() );
		
		//Get a list of the providers with placename that is exiting in the database.
		//It my happend that we dont have data for some models, ie they are deleted.
		if( updateProviderRefTimes( wciConnection, exitingProviders, providerPriorityList, wciProtocol_ ) ) {
			
			ost.str("");
			ost << "*** exitingProviders: " << exitingProviders.size() << endl;
			for( ProviderRefTimeList::const_iterator it=exitingProviders.begin();
   		         it!=exitingProviders.end();
			     ++ it )
			{
				ost << " --- : " << it->first << " reftime: " << it->second.refTime
				     << " updated: " << it->second.updatedTime
				     << " version: " << it->second.dataversion << endl;
			}
			WEBFW_LOG_DEBUG( ost.str() );
			
			ost.str("");
			ost << "*** requestedProviders: " << requestedProviders.size() << endl;
			for( ProviderRefTimeList::const_iterator it=requestedProviders.begin();
					it!=requestedProviders.end();
					++ it )
			{
				ost << " --- : " << it->first << " reftime: " << it->second.refTime
						<< " updated: " << it->second.updatedTime
						<< " disabled(" << (it->second.disableEnableRequest?"t":"f") <<"): " << (it->second.disabled?"true":"false")
						<< " version(" << (it->second.dataversionRequest?"t":"f") <<"): " << it->second.dataversion << endl;
			}
			WEBFW_LOG_DEBUG( ost.str() );

			try {
				updateProviderRefTimes( wciConnection, requestedProviders, newRefTime, wciProtocol_ );
			}
			catch( const std::logic_error &ex ) {
				WEBFW_LOG_ERROR("Update failed: " << ex.what() );
				response.errorDoc( ex.what() );
				response.status( webfw::Response::NO_DATA );
				return;
			}

			ost.str("");
			ost << "*** newRefTime: " << newRefTime.size() << endl;
			for( ProviderRefTimeList::const_iterator it=newRefTime.begin();
				 it!=newRefTime.end();
				++ it )
				ost << " --- : " << it->first << " reftime: " << it->second.refTime
				    << " updated: " << it->second.updatedTime
				    << " disabled: " << (it->second.disabled?"true":"false")
				    << " version: " << it->second.dataversion << endl;

			WEBFW_LOG_DEBUG( ost.str() );
				
			checkAgainstExistingProviders( exitingProviders, newRefTime );
			app->notes.setNote( updateid + ".LocationProviderReftimeList", new NoteProviderReftimes( newRefTime ) );
			WEBFW_LOG_DEBUG("LocationForecastUpdateHandler: note updated: '" << updateid + ".LocationProviderReftimeList." );
		}
   		
		status = updateStatus( oldRefTime, newRefTime );
		ost.str("");
		ost << "UpdateHandler: Provider reftimes at " << second_clock::universal_time() << "Z" << endl;
		for( ProviderRefTimeList::const_iterator it=newRefTime.begin(); it != newRefTime.end(); ++it )
			ost << it->first << "Reftime:  " << it->second.refTime << " Disabled: " << (it->second.disabled?"true":"false") << " Version:  " << it->second.dataversion << endl;
		ost << "UpdateStatus: " << status << endl;
   		
		ost << endl;
		WEBFW_LOG_INFO( ost.str() );
		 
		std::string xml;         
		xml="<?xml version=\"1.0\" ?>\n<status>" + status + "</status>\n";
				
		response.out() << xml; 
	}
	catch( const std::logic_error &ex ) {
		WEBFW_LOG_WARN( ex.what() );
      response.errorDoc( ex.what() );
      response.status( webfw::Response::INTERNAL_ERROR );
	}
	catch( ... ) {
		WEBFW_LOG_WARN("UpdateHandler: Unknown exception.)");
      response.errorDoc("Unexpected exception!");
      response.status( webfw::Response::INTERNAL_ERROR );
	}
}

}
