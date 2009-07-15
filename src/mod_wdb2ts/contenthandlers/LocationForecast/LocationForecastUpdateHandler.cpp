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
	
	vector<string> keys;
	vector<string> keyvals;
	string              buf;
	int                      dataversion;
	
	newRefTime.clear();
	
	if( query.empty() )
		return true;
	
	string::size_type i=query.find_first_of("&;");
	
	if( i == string::npos )
		keys.push_back( query );
	else
		keys=splitstr( query, query[i] );
	
	for( vector<string>::size_type iKey=0; iKey < keys.size(); ++iKey ) {
		string key;
		string val;
		
		keyvals = splitstr( keys[iKey], '=' );
		
		if( keyvals.size() != 2 ) {
			cerr << "LocationUpdateHandler:Query: Invalid key. Expecting the form key=value." << endl;
			return false;
		}
	
		key = keyvals[0];
		val = keyvals[1];
		
		trimstr( key );
		trimstr( val );
		
		if( key.empty() ) {
			cerr << "LocationUpdateHandler:Query: Invalid key. Empty key." << endl;
			return false;
		}
		
		ProviderItem pi=ProviderList::decodeItem( key );
		

		//The val can be on the form YYYY-MM-DDThh:mm:ss,dataversion
		//Where dataverion is optional.
		//A dataversion is used only if the reftime is given.
		keyvals = splitstr( val, ',' );
		dataversion = -1;
			
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
				cerr << "LocationUpdateHandler:Query: Invalid Value. key <" << key << "> expecting a number." << endl;
				return false;
			}
			
			if( n != 0 ) {
				cerr << "LocationUpdateHandler:Query: Invalid Value. key <" << key << "> Not a valid value. Valid values [0]." << endl;
				return false;
			}
				
			newRefTime[pi.providerWithPlacename()] = ProviderTimes();
			continue;
		}
		
		try{ 
			newRefTime[pi.providerWithPlacename()].refTime = ptimeFromIsoString( val );
			newRefTime[pi.providerWithPlacename()].dataversion = dataversion;
		}
		catch( logic_error &e ) {
			cerr << "LocationUpdateHandler:Query: Invalid value. key <" << key << "> value <" << val << ">. Not a valid timespec." << endl;
			return false;
		}
	}
	
	
	cerr << "LocationUpdateHandler:Query: Requested providers reftime!" << endl;
	for( ProviderRefTimeList::iterator it = newRefTime.begin();
	     it != newRefTime.end() ;
		  ++it )
	{
		cerr <<"     " << it->first << ": " << it->second.refTime << " dataversion: " << it->second.dataversion << endl;
	}
	
	cerr << endl;
	
	return true;
}


bool 
LocationForecastUpdateHandler::
getProviderPriorityList( Wdb2TsApp *app, ProviderList &providerPriorityList )const
{
	providerPriorityList.clear();
	
	boost::shared_ptr<NoteTag> note=app->notes.getNote(updateid + ".LocationProviderList");
		
	if( note ) {
		NoteProviderList *myNote = dynamic_cast<NoteProviderList*>( note.get() );
			
		if( myNote ) 
			providerPriorityList = *myNote;
	}
	
	cerr << "LocationForecastUpdateHandler: providerPriorityList: " ;
	for( ProviderList::iterator it=providerPriorityList.begin();
	     it != providerPriorityList.end();
	     ++it )
		cerr << "'" << it->providerWithPlacename() << "'";
			
	cerr << endl;
	
	return true;
}

void
LocationForecastUpdateHandler::
checkProviders( const ProviderList &providerList,  ProviderRefTimeList &requestedUpdate ) const
{
	list<string> providers=providerList.providerWithoutPlacename();
	list<string>::const_iterator pit;
	
	if( requestedUpdate.empty() ) {
		for ( pit = providers.begin(); 
		      pit != providers.end();
		      ++ pit )
			requestedUpdate[*pit] = ProviderTimes();
		
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
			cerr << "WARNING: LocationUpdateHandler: It is requested an update for an provider '" 
			     << it->first << "' that is not in the provider_priority list. The provider is ignored." << endl;
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
	ProviderRefTimeList::const_iterator itOld;
	ProviderRefTimeList::const_iterator itNew;
	
	cerr << "LocationForecastUpdateHandler: refTimes. " << endl;
	for( ProviderRefTimeList::iterator it = newRefTime.begin();
	     it != newRefTime.end();
		  ++it )
		cerr << "  " << it->first << ": " << it->second.refTime
		     << ": " << it->second.dataversion << endl;;
		
	
	
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
	
	boost::shared_ptr<NoteTag> note;
	
	//Has the LocationForecastHandler set a note
	//with the wdbID it use.   
	note = app->notes.getNote( updateid+".LocationForecastWdbId" );
		
	if( ! note )  {
		cerr << "getWdbId: no note!" << endl;
		return string();
	}
		
	NoteString *noteWdbId = dynamic_cast<NoteString*>( note.get() );
		
	if( ! noteWdbId ) {
		cerr << "getWdbId <NoteString> : failed dynamic_cast <" <<note->noteType()<< ">." << endl;
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
     webfw::Logger   &logger )   
{
	ostringstream ost;
	string status;
	ProviderRefTimeList requestedProviders;
	ProviderList providerPriorityList;
	int wciProtocol_;
	
	response.contentType("text/xml");
	response.directOutput( true );
   
	
	ost << endl << "URL:   " << req.urlPath() << endl 
	<< "Query: " << req.urlQuery() << endl << "Updateid: " << updateid << endl;
   
	logger.debug( ost.str() );
   
	if( ! decodeQuery( req.urlQuery(), requestedProviders ) ) {
		logger.error( ost.str() );
		response.errorDoc( ost.str() );
		response.status( webfw::Response::INVALID_QUERY );
		return;
	}

	if( updateid.empty() ) {
		std::string xml;         
		xml="<?xml version=\"1.0\" ?>\n<status>Error: missing updateid!</status>\n";
		response.errorDoc( xml ); 	
		response.status( webfw::Response::INTERNAL_ERROR );
		cerr << "LocationForecatUpdate: " << ost.str() << ". Missing: updateid!" << endl;
		return;
	}
	
	Wdb2TsApp *app=Wdb2TsApp::app();

	
	getProviderPriorityList( app, providerPriorityList );
	
	checkProviders( providerPriorityList, requestedProviders );
	
	if( requestedProviders.empty() ) {
		ost.str("");
		ost << "Only providers referenced in the provider_priority can be updated.";
		logger.error( ost.str() );
		response.errorDoc( ost.str() );
		response.status( webfw::Response::INVALID_QUERY );
		return;
	}
	
	try {
	
		app->initHightMap();
		
		ProviderRefTimeList exitingProviders;
		ProviderRefTimeList oldRefTime = getProviderReftimes( app );
		ProviderRefTimeList newRefTime( oldRefTime );
		string myWdbID = getWdbId( app );
		
		cerr << "LocationForecastUpdateHandler: myWdbID: " << myWdbID << endl;
		
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
	
		for( ProviderRefTimeList::const_iterator it=oldRefTime.begin();
					     it!=oldRefTime.end();
					     ++ it )
			cerr << "*** oldRefTime: " << it->first << ": " << it->second.refTime 
				  << ": " << it->second.updatedTime 
				  << ": " << it->second.dataversion << endl;

		
		//Get a list of the providers with placename that is exiting in the database. It my happend that
		//we dont have data for some models, ie they are deleted. 
		if( updateProviderRefTimes( wciConnection, exitingProviders, providerPriorityList, wciProtocol_ ) ) {
			
			for( ProviderRefTimeList::const_iterator it=exitingProviders.begin();
   		     it!=exitingProviders.end();
			     ++ it )
				cerr << "*** exitingProviders: " << it->first << ": " << it->second.refTime 
				     << ": " << it->second.updatedTime 
				     << ": " << it->second.dataversion << endl;
			
			if( updateProviderRefTimes( wciConnection, requestedProviders, newRefTime, wciProtocol_ ) ) {
				for( ProviderRefTimeList::const_iterator it=newRefTime.begin();
				     it!=newRefTime.end();
				     ++ it )
					cerr << "*** newRefTime: " << it->first << ": " << it->second.refTime << ": " 
					     << it->second.updatedTime 
					     << ": " << it->second.dataversion << endl;;
				
				checkAgainstExistingProviders( exitingProviders, newRefTime );
				app->notes.setNote( updateid + ".LocationProviderReftimeList", new NoteProviderReftimes( newRefTime ) );
				cerr << "LocationForecastUpdateHandler: note updated: '" << updateid + ".LocationProviderReftimeList" 
				  	  << "'." <<  endl;
			   app->notes.checkForUpdatedPersistentNotes();
			}
		}
   		
		status = updateStatus( oldRefTime, newRefTime );
		ost.str("");
		ost << " ---- UpdateHandler: Provider reftimes at " << second_clock::universal_time() << "Z" << endl;
		for( ProviderRefTimeList::const_iterator it=newRefTime.begin(); it != newRefTime.end(); ++it )
			ost << it->first << ":  " << it->second.refTime << ":  " << it->second.dataversion << endl;
		ost << "UpdateStatus: " << status << endl;
   		
		ost << endl;
		logger.info( ost.str() );
		 
		std::string xml;         
		xml="<?xml version=\"1.0\" ?>\n<status>" + status + "</status>\n";
				
		response.out() << xml; 
	}
	catch( const std::logic_error &ex ) {
		logger.warning( ex.what() );
      response.errorDoc( ex.what() );
      response.status( webfw::Response::INTERNAL_ERROR );
	}
	catch( ... ) {
		logger.warning("UpdateHandler: Unknown exception.)");
      response.errorDoc("Unexpected exception!");
      response.status( webfw::Response::INTERNAL_ERROR );
	}
}

}
