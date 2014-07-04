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
#include <boost/algorithm/string.hpp>
#include <trimstr.h>
#include <transactor/ProviderRefTime.h>
#include <RequestConf.h>
#include <ProviderList.h>
#include <wdb2TsApp.h>
#include <Logger4cpp.h>


using namespace std;

namespace {

wdb2ts::ProviderList
decode_( const std::string &toDecode_, std::string &provider, bool  toDecodeMayBeList )
{
	using namespace wdb2ts;

	string toDecode( toDecode_ );
	ProviderList pList;
	string::size_type i = toDecode.find("[");
	
	WEBFW_USE_LOGGER( "handler" );

	//WEBFW_LOG_DEBUG( "ProviderList::decode_: toDecode '" << toDecode <<"' mayBeList: "  << (toDecodeMayBeList ? "true":"false") );

	if( i == string::npos ) {
		pList.push_back( ProviderItem( toDecode ) );
		provider = toDecode;
		return pList;
	}
	
	provider = toDecode.substr( 0, i );
	miutil::trimstr( provider );
	
	if( provider.empty() ) {
		WEBFW_LOG_ERROR( "ProviderList::decode_ provider priority failed. Expecting a definition on the form "
		     << " 'provider [placename, placename1, ... ]', but got '" << toDecode_ << "'" );
		return pList;
	}
	
	toDecode.erase( 0, i+1 );
	
	i = toDecode.find("]");
	
	if( i == string::npos ) {
		WEBFW_LOG_ERROR( "ProviderList::Decode_ provider priority failed. Expecting a definition on the form "
		     << " 'provider [placename, placename1, ... ]', but got '" << toDecode_ << "'" );
		return pList;
	}
	
	toDecode.erase( i );
	miutil::trimstr( toDecode );
	
	if( toDecodeMayBeList ) {
		vector<string> placenames = miutil::splitstr( toDecode, ',', '\'' );
	
		if( placenames.empty() ) {
			pList.push_back( ProviderItem( provider ) );
			return pList;
		}

	
		for( vector<string>::size_type index=0; index < placenames.size(); ++index ) {
			string buf( placenames[index] );
			miutil::trimstr( buf );
		
			if( buf.empty() )
				continue;

			if( buf.length() >= 2 && buf[0]=='\'' && buf[buf.length()-1] == '\'' ) {
				buf  = buf.substr(1, buf.length()-2 );
				miutil::trimstr( buf );
			}

		//	WEBFW_LOG_DEBUG( "ProviderList::decode_: '" << provider << "'  [" << buf << "]");
			pList.push_back( ProviderItem( provider, buf ) );
		}
	} else {
		if( toDecode.length() >= 2 && toDecode[0]=='\'' && toDecode[toDecode.length()-1] == '\'' ) {
			toDecode  = toDecode.substr(1, toDecode.length()-2 );
			miutil::trimstr( toDecode );
		}

		if( !toDecode.empty() ) {
			//WEBFW_LOG_DEBUG( "ProviderList::decode_: '" << provider << "'  [" << toDecode << "]");
			pList.push_back( ProviderItem( provider, toDecode ) );
		}
	}
	
	if( pList.empty() ) 
		pList.push_back( ProviderItem( provider ) );
	
	if( toDecodeMayBeList ) {
		for( ProviderList::iterator it = pList.begin(); it!=pList.end(); ++it)
			WEBFW_LOG_DEBUG( "ProvderList::decode: pList: '" << it->providerWithPlacename() << "'" );
	}

	return pList;
}


}


namespace wdb2ts {


ProviderItem
ProviderItem::
decode( const std::string &providerWithPlacename )
{
	ProviderItem pi;
	string::size_type i = providerWithPlacename.find( "[" );

	if( i != string::npos ) {
		string::size_type ii = providerWithPlacename.find( "]", i );
		pi.provider = providerWithPlacename.substr(0, i );
		boost::algorithm::trim( pi.provider );

		if( ii != string::npos ) {
			pi.placename = providerWithPlacename.substr( i+1, ii-i-1 );
			boost::algorithm::trim( pi.placename );
		}
	} else {
		pi.provider = providerWithPlacename;
	}
	return pi;
}

ProviderItem
ProviderItem::
decodeFromWdb( const std::string &provider_, const std::string &pointPlacename )
{
	string provider = provider_;
	string placename;
	string::size_type i = pointPlacename.find_last_of( ")" );

	if( i != string::npos ) {
		++i;
		if( i<pointPlacename.length() ) {
			i = pointPlacename.find_first_not_of( " ", i );

			if( i != string::npos )
				placename = pointPlacename.substr( i );
		}
	}


	boost::algorithm::trim( provider );
	boost::algorithm::trim( placename );

	return ProviderItem( provider, placename );
}



ProviderList
ProviderList::
decode( const std::string &toDecode_, std::string &provider )
{
	return decode_( toDecode_, provider, true );
}

ProviderList 
ProviderList::
decode( const std::string &toDecode_ ) 
{
	string dummy;
	
	return decode_( toDecode_, dummy, true );
}


ProviderItem 
ProviderList::
decodeItem( const std::string &toDecode)
{
	string dummy;
	ProviderList pvList = decode_( toDecode, dummy, false );
	
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
findProviderWithoutPlacename( const std::string &provider )const
{
	ProviderItem pi=ProviderItem::decode( provider );

	for( const_iterator it = begin(); it != end(); ++it ) {
		if( it->provider == pi.provider ) {
			return it;
		}
	}

	return end();
}




void
ProviderList::
addProvider( const ProviderItem &item )
{
	ProviderList::const_iterator it = findProvider( item.providerWithPlacename() );
	if( it == end() )
		push_back( item );
}

ProviderList::const_iterator 
ProviderList::
findProvider( const std::string &provider, 
			  const std::string &pointPlacename_,
			  std::string &providerWithplacename)const
{
	ProviderItem item( ProviderItem::decodeFromWdb( provider, pointPlacename_ ) );
	const_iterator itFound=end();
	
	for( const_iterator it = begin(); it != end(); ++it ) {
		if( it->provider == item.provider ) {
			if( item.placename.empty() ) {
				itFound = it;
				break;
			} else if( it->placename == item.placename ) {
				itFound = it;
				break;
			}
		}
	}
   	
	if( itFound != end() )
		providerWithplacename = itFound->providerWithPlacename();
	else
		providerWithplacename = item.providerWithPlacename();
   
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
providerListFromConfig( const wdb2ts::config::ActionParam &params )
{
   string provider;
   ProviderList providerPriority;
   wdb2ts::config::ActionParam::const_iterator it;

   WEBFW_USE_LOGGER( "handler" );

   it=params.find("provider_priority");

   if( it != params.end() ) {
      vector<string> pp = miutil::splitstr( it->second.asString(), ';' );

      WEBFW_LOG_DEBUG( "configureProviderPriority: provider_priority: <" << it->second.asString() << "> Size: " << pp.size() );

      for( vector<string>::size_type i=0; i<pp.size(); ++i ) {
         ProviderList pList = ProviderList::decode( pp[i], provider );

         for( ProviderList::iterator pit=pList.begin(); pit != pList.end(); ++pit )
            providerPriority.push_back( *pit );
      }

      std::ostringstream logMsg;
      logMsg << "configureProviderPriority: ProviderPriority: defined.\n";
      for( ProviderList::size_type i=0; i < providerPriority.size(); ++i )
         logMsg << "  " << providerPriority[i].providerWithPlacename() << '\n';
      WEBFW_LOG_DEBUG(logMsg.str());
   }

   return providerPriority;
}





}


