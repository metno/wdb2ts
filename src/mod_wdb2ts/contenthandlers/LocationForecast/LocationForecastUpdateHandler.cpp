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
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/assign/list_inserter.hpp>
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
decodeQuery( const std::string &query, ProviderRefTimeList &newRefTime )const
{
	using namespace miutil;
	using namespace boost::posix_time;
	
	WEBFW_USE_LOGGER( "handler" );

	std::list<std::string> keys;
	//vector<string> keys;
	vector<string> keyvals;
	string              buf;
	int                 dataversion;
	bool                disable;
	bool 	            doDisableEnable=false;
	webfw::UrlQuery     urlQuery;
	
	newRefTime.clear();
	
	if( query.empty() )
		return true;
	
	try {
		urlQuery.decode( query );
	}
	catch( const std::exception &ex ) {
		WEBFW_LOG_ERROR( "LocationUpdateHandler:Query: Invalid query '" + query +". Reason: " + ex.what() );;
	}

	keys = urlQuery.keys();
	
	for( std::list<std::string>::const_iterator iKey=keys.begin(); iKey != keys.end(); ++iKey ) {
		string key = *iKey ;
		string val = urlQuery.asString( *iKey, "");
		string::size_type i;
		
		trimstr( key );
		trimstr( val );
		
		if( key.empty() ) {
			WEBFW_LOG_ERROR( "LocationUpdateHandler:Query: Invalid key. Empty key." );;
			return false;
		}

		if( val.empty() ) {
			WEBFW_LOG_ERROR( "LocationUpdateHandler:Query: Key: '" << key << "'. Empty value." );;
			return false;
		}

		WEBFW_LOG_ERROR( "LocationUpdateHandler:Query: Key: '" << key << "' vcalue: '" << val );
		ProviderItem pi=ProviderList::decodeItem( key );
		

		//The val can be on the form YYYY-MM-DDThh:mm:ss,dataversion
		//Where dataversion is optional.
		//A dataversion is used only if the reftime is given.
		keyvals = splitstr( val, ',' );
		dataversion = -1;
		disable = false;
			
		if( keyvals.size() > 1 ) {
			val = keyvals[0];
			buf = keyvals[1];
				
			if( sscanf( buf.c_str(), "%d", &dataversion ) != 1 )
				dataversion = -1;
		}
		
		if( val.empty() ) {
			newRefTime[pi.providerWithPlacename()] = ProviderTimes();
			continue;
		}
		
				
		i = val.find_first_not_of( "0123456789" );
		

		if( i == string::npos ) {
			int n;
			
			if( sscanf( val.c_str(), "%d", &n ) != 1 ) {
				WEBFW_LOG_ERROR( "LocationUpdateHandler:Query: Invalid Value. key <" << key << "> expecting a number." );
				return false;
			}
			
			if( n != 0 ) {
				WEBFW_LOG_ERROR( "LocationUpdateHandler:Query: Invalid Value. key <" << key << "> Not a valid value. Valid values [0]." );;
				return false;
			}
				
			newRefTime[pi.providerWithPlacename()] = ProviderTimes();
			continue;
		} if( val[0]=='d' || val[0]=='D' ) {
			doDisableEnable = true;
			disable = true;
		} else if( val[0]=='e' || val[0]=='E' ) {
			doDisableEnable = true;
		}
		
		try{ 
			if( doDisableEnable )
				newRefTime[pi.providerWithPlacename()].disabled = disable;
			else
				newRefTime[pi.providerWithPlacename()].refTime = ptimeFromIsoString( val );

			newRefTime[pi.providerWithPlacename()].dataversion = dataversion;
		}
		catch( logic_error &e ) {
			WEBFW_LOG_ERROR( "LocationUpdateHandler:Query: Invalid value. key <" << key << "> value <" << val << ">. Not a valid timespec." );;
			return false;
		}
	}
	
	std::ostringstream logMsg;
	logMsg << "LocationUpdateHandler:Query: Requested providers reftime!\n";
	for( ProviderRefTimeList::const_iterator it = newRefTime.begin(); it != newRefTime.end(); ++it )
		logMsg <<"     " << it->first << ": " << it->second.refTime << " dataversion: " << it->second.dataversion << '\n';
	WEBFW_LOG_DEBUG(logMsg.str());
	
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

void
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
	
	if( requestedUpdate.empty() ) {
		for ( pit = providers.begin(); 
		      pit != providers.end();
		      ++ pit ) {
			requestedUpdate[*pit] = ProviderTimes();

			if( oldRefTime.disabled( *pit, disabled ) )
				requestedUpdate[*pit].disabled = disabled;
		}
		return;
	}
	
	ProviderRefTimeList::iterator it = requestedUpdate.begin();
	while( it != requestedUpdate.end() )
	{
		for( pit = providers.begin();
		     pit != providers.end();
		     ++pit )
		{
			ProviderItem pi=ProviderList::decodeItem( it->first );

			if( *pit == pi.provider )
				break;
		}	
		
		if( pit == providers.end() ) {
			//It is requested update for a provider that is not defined 
			//in the priority_provider. It is removed from the list.
			WEBFW_LOG_WARN( "WARNING: LocationUpdateHandler: It is requested an update for an provider '"
			     << it->first << "' that is not in the provider_priority list. The provider is ignored.");
			it = miutil::eraseElement( requestedUpdate, it );
		} else {
			++it;
		}
	}
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
			 itOld->second.dataversion != itNew->second.dataversion )
			return "Updated";
	}
	
	return "NoNewDatRefTime";
}

ProviderRefTimeList
LocationForecastUpdateHandler::
getProviderReftimes( Wdb2TsApp *app )
{
	boost::shared_ptr<NoteTag> note;
	
	note = app->notes.getNote( updateid + ".LocationProviderReftimeList" );
	
	if( ! note ) 
		return ProviderRefTimeList();
	
	
	NoteProviderReftimes *refTimes = dynamic_cast<NoteProviderReftimes*>( note.get() );
	
	if( ! refTimes )
		return ProviderRefTimeList();
	
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
	
	response.contentType("text/xml");
	response.directOutput( true );
   
	
	ost << endl << "URL:   " << req.urlPath() << endl 
	<< "Query: " << req.urlQuery() << endl << "Updateid: " << updateid << endl;
   
	WEBFW_LOG_DEBUG( ost.str() );
   
	if( ! decodeQuery( req.urlQuery(), requestedProviders ) ) {
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
	


	
	try {
		Wdb2TsApp *app=Wdb2TsApp::app();
		ProviderRefTimeList exitingProviders;
		ProviderRefTimeList oldRefTime = getProviderReftimes( app );
		ProviderRefTimeList newRefTime( oldRefTime );
		string myWdbID = getWdbId( app );

		getProviderPriorityList( app, providerPriorityList );
	
		checkProviders( providerPriorityList, oldRefTime, requestedProviders );
	
		if( requestedProviders.empty() ) {
			ost.str("");
			ost << "Only providers referenced in the provider_priority can be updated.";
			WEBFW_LOG_ERROR( ost.str() );
			response.errorDoc( ost.str() );
			response.status( webfw::Response::INVALID_QUERY );
			return;
		}
	
		app->initHightMap();
		
		
		WEBFW_LOG_DEBUG( "LocationForecastUpdateHandler: myWdbID: " << myWdbID );;
		
		if( wciProtocol < 0 ) {
			wciProtocol_ = app->wciProtocol( wdbDB );
			
			if( wciProtocol_ <= 0 )
				wciProtocol_ = 1;
			else
				wciProtocol = wciProtocol_;
		} else {
			wciProtocol_ = wciProtocol;
		}
			
		
		WciConnectionPtr wciConnection = app->newWciConnection( myWdbID );
	
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
		
		//Get a list of the providers with placename that is exiting in the database. It my happend that
		//we dont have data for some models, ie they are deleted. 
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
						<< " disabled: " << (it->second.disabled?"true":"false")
						<< " version: " << it->second.dataversion << endl;
			}
			WEBFW_LOG_DEBUG( ost.str() );

			if( updateProviderRefTimes( wciConnection, requestedProviders, newRefTime, wciProtocol_ ) ) {

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
				app->notes.checkForUpdatedPersistentNotes();
			}
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
