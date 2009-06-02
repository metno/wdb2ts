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


#include <algorithm>
#include <iostream>
#include <sstream>
#include <replace.h>
#include <splitstr.h>
#include <trimstr.h>
#include <transactor/ProviderRefTime.h>
#include <ProviderList.h>
#include <wdb2TsApp.h>



using namespace std;


namespace wdb2ts {

ProviderList
ProviderList::
decode( const std::string &toDecode_, std::string &provider )
{
	string toDecode( toDecode_ );
	ProviderList pList;
	string::size_type i = toDecode.find("[");
	
	if( i == string::npos ) {
		pList.push_back( ProviderItem( toDecode ) );
		provider = toDecode;
		return pList;
	}
	
	provider = toDecode.substr( 0, i );
	miutil::trimstr( provider );
	
	if( provider.empty() ) {
		cerr << "ProviderList::decode provider priority failed. Expecting a definition on the form " 
		     << " 'provider [placename, placename1, ... ]', but got '" << toDecode_ << "'" << endl;
		return pList;
	}
	
	toDecode.erase( 0, i+1 );
	
	i = toDecode.find("]");
	
	if( i == string::npos ) {
		cerr << "ProviderList::Decode provider priority failed. Expecting a definition on the form " 
		     << " 'provider [placename, placename1, ... ]', but got '" << toDecode_ << "'" << endl;
		return pList;
	}
	
	toDecode.erase( i );
	
	vector<string> placenames = miutil::splitstr( toDecode, ',' );
	
	if( placenames.empty() ) {
		pList.push_back( ProviderItem( provider ) );
		return pList;
	}

	
	for( int index=0; index < placenames.size(); ++index ) {
		string buf( placenames[index] );
		miutil::trimstr( buf );
		
		if( buf.empty() )
			continue;
		
		pList.push_back( ProviderItem( provider, buf ) );
	}
	
	if( pList.empty() ) 
		pList.push_back( ProviderItem( provider ) );
	
	return pList;
}

ProviderList 
ProviderList::
decode( const std::string &toDecode_ ) 
{
	string dummy;
	
	return decode( toDecode_, dummy );
}


ProviderItem 
ProviderList::
decodeItem( const std::string &toDecode )
{
	ProviderList pvList = decode( toDecode );
	
	if( pvList.empty() )
		return ProviderItem();
	
	return pvList[0];
}

ProviderList::const_iterator
ProviderList::
findProvider( const std::string &providerWithPlacename )const
{
	for( const_iterator it = begin(); it != end(); ++it ) {
		if( it->providerWithPlacename() == providerWithPlacename ) {
			return it;
		} 
	}
	
	return end();
}


ProviderList::const_iterator 
ProviderList::
findProvider( const std::string &provider, 
			     const std::string &pointPlacename_,
			     std::string &providerWithplacename)const
{
	string placename;
	string::size_type i = pointPlacename_.find_last_of( ")" );
	
	if( i != string::npos ) {
		++i;
		if( i<pointPlacename_.length() ) {
			i = pointPlacename_.find_first_not_of( " ", i );
		
			if( i != string::npos ) 
				placename = pointPlacename_.substr( i );
		}
	}

	const_iterator itFound=end();
	
   for( const_iterator it = begin(); it != end(); ++it ) {
   	if( it->provider == provider ) {
   		if( placename.empty() ) { 
   			itFound = it;
   			break;
   		} else if( it->placename == placename ) {
   			itFound = it;
   			break;
   		} 
   	}
   }
   	
   if( itFound != end() )
    	providerWithplacename = itFound->providerWithPlacename();
   
   return itFound;
}

std::list<std::string>
ProviderList::
providerWithoutPlacename()const
{
	list<string> res;
	
	for( const_iterator it = begin(); it != end(); ++it )
		res.push_back( it->provider );
	
	/*
	for( list<string>::const_iterator it = res.begin(); it != res.end(); ++it )
		cerr << "**** providerList: " << *it << endl; 
*/		
	
	res.unique();
	/*
	for( list<string>::const_iterator it = res.begin(); it != res.end(); ++it )
		cerr << "--- providerList: " << *it << endl; 
	*/	
	//unique( res.begin(), res.end() );
	
	return res;
}

ProviderList 
ProviderList::
providerListWithoutPlacename() const
{
	ProviderList providerListWithoutPlacenames;
	std::list<std::string> providerListNames = providerWithoutPlacename();

	for( std::list<std::string>::const_iterator it = providerListNames.begin();
	     it != providerListNames.end();
	     ++it )
		providerListWithoutPlacenames.push_back( ProviderItem( *it ) );
	
	return providerListWithoutPlacenames;
}

ProviderList
providerPrioritySetPlacename( const ProviderList &pvList, 
										const std::string &wdbDB,
										Wdb2TsApp *app )
{
	WciConnectionPtr wciConnection;
	ProviderList resList;
	
	try {
		wciConnection = app->newWciConnection( wdbDB );
	}
	catch( exception &ex ) {
		cerr << "EXCEPTION: LocationForecastHandler::providerPrioritySetPlacename: NO DB CONNECTION. "
		<< ex.what() << endl;
		return resList;
	}
	catch( ... ) {
		cerr << "EXCEPTION: LocationForecastHandler::providerPrioritySetPlacename: NO DB CONNECTION. unknown exception "
		<< endl;
		return resList;
	}
	
	for( ProviderList::const_iterator it=pvList.begin(); it != pvList.end(); ++it ) {
		if( ! it->placename.empty() ) {
			resList.push_back( *it );
			continue;
		}
		
		try {
			ProviderRefTimeList dummyRefTimeList;
			ProviderRefTime providerReftimeTransactor( dummyRefTimeList, 
					                                     it->provider, 
					                                     "NULL" );

			wciConnection->perform( providerReftimeTransactor, 3 );
			PtrProviderRefTimes res = providerReftimeTransactor.result();
			
			if( ! res )
				continue;
			
			for( ProviderRefTimeList::iterator pit = res->begin(); 
			     pit != res->end(); 
			     ++pit ) {
				ProviderList tmp = ProviderList::decode( pit->first );
				for( ProviderList::size_type i=0; i < tmp.size(); ++i )
					resList.push_back( tmp[i] );
			}
		}
		catch( const std::ios_base::failure &ex ) {
			cerr << "EXCEPTION:std::ios_base::failure: LocationForecastHandler::providerPrioritySetPlacename: " << ex.what() << endl;
		}
		catch( const std::runtime_error &ex ) {
			cerr << "EXCEPTION:std::runtime_error: LocationForecastHandler::providerPrioritySetPlacename: " << ex.what() << endl;
		}
		catch( const std::logic_error &ex ) {
			cerr << "EXCEPTION:std::logic_error: LocationForecastHandler::providerPrioritySetPlacename: " << ex.what() << endl;
		}
		catch( ... ) {
			cerr << "EXCEPTION:unknown: LocationForecastHandler::providerPrioritySetPlacename" << endl;
		}
	}
		
	return resList;			
}


ProviderList
configureProviderList( const wdb2ts::config::ActionParam &params, 
		                 const std::string &wdbDB,
		                 Wdb2TsApp *app )
{
	string provider;
	ProviderList providerPriority;
	ProviderList retProviderPriority;
	wdb2ts::config::ActionParam::const_iterator it;
	
	it=params.find("provider_priority");
	
	if( it != params.end() ) {
		vector<string> pp = miutil::splitstr( it->second.asString(), ';' );
		
		cerr << "configureProviderPriority: provider_priority: <" << it->second.asString() << "> Size: " << pp.size() << endl;
		
		for( int i=0; i<pp.size(); ++i ) {
			ProviderList pList = ProviderList::decode( pp[i], provider );
			
			for( ProviderList::iterator pit=pList.begin(); pit != pList.end(); ++pit ) 
				providerPriority.push_back( *pit );
		}	
		
		cerr << "configureProviderPriority: ProviderPriority: defined. " << endl;
		for( ProviderList::size_type i=0; i < providerPriority.size(); ++i )
			cerr << "  " << providerPriority[i].providerWithPlacename() << endl;

		retProviderPriority = providerPrioritySetPlacename( providerPriority, wdbDB, app );
		
		cerr << "configureProviderPriority: ProviderPriority: defined (resolved). " << endl;

		for( ProviderList::size_type i=0; i < retProviderPriority.size(); ++i )
			cerr << "  " << retProviderPriority[i].providerWithPlacename() << endl;
	
	}
	
	return retProviderPriority;
}




}


