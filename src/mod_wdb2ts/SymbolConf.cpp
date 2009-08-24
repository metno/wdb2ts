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

#include <ctype.h>
#include <stdio.h>
#include <SymbolConf.h>
#include <trimstr.h>
#include <splitstr.h>
#include <ProviderList.h>
#include <UpdateProviderReftimes.h>
#include <transactor/ProviderRefTime.h>
#include <wdb2TsApp.h>
#include <Logger4cpp.h>

namespace wdb2ts {

using namespace std;
	
SymbolConf::
SymbolConf()
{
}

SymbolConf::
SymbolConf( int min, int max, int precipHours )
	: min_( min ), max_( max ), precipHours_( precipHours )
{
}

SymbolConf::
SymbolConf( const SymbolConf &sf )
	: min_( sf.min_), max_( sf.max_ ), precipHours_( sf.precipHours_ )
{
}
	

SymbolConf& 
SymbolConf::
operator=( const SymbolConf &rhs )
{
	if( &rhs != this ) {
		min_ = rhs.min_;
		max_ = rhs.max_;
		precipHours_ = rhs.precipHours_;
	}
	
	return *this;
}

	
bool
SymbolConf::
operator==( const SymbolConf &rhs )
{
	return rhs.min_ == min_ && rhs.max_ == max_ && rhs.precipHours_ == precipHours_;
}
bool 
SymbolConf::
operator!=( const SymbolConf &rhs )
{
	return rhs.min_ != min_ || rhs.max_ != max_ || rhs.precipHours_ != precipHours_;
}	
	
bool
SymbolConf::
parse( const std::string &buf_, SymbolConfList &conf )
{
   string strbuf(buf_);
   string buf;
   string iBuf;
   string::size_type i, i2;
   int min, max, precipHours;

   conf.clear();
   
   i = strbuf.find_first_of("(");
   
   while ( i!=string::npos ) {
      i2 = strbuf.find_first_of(")", i);
   
      if ( i2 == string::npos )
         return false;
   
      i++;
      buf = strbuf.substr( i, i2-i );
      
      vector<string> vals=miutil::splitstr(buf, ',');
   
      if ( vals.size() != 3 ) 
         return false;
      
      miutil::trimstr(vals[0]);
      miutil::trimstr(vals[1]);
      miutil::trimstr(vals[2]);
      
      if ( vals[0].empty() || vals[1].empty() || vals[2].empty() )
         return false;
      
      for ( vector<string>::size_type ii=0; ii < vals.size(); ++ii )
         for ( vector<string>::size_type k=0; k < vals[ii].size(); ++k )
            if ( ! isdigit( vals[ii][k] ) )
               return false;   
   
      if ( sscanf( vals[0].c_str(), "%d", &min ) == 0 )
         return false;
   
      if ( sscanf( vals[1].c_str(), "%d", &max ) == 0 )
         return false;
         
      if ( sscanf( vals[2].c_str(), "%d", &precipHours ) == 0 )
         return false;            
      
      conf.push_back( SymbolConf( min, max, precipHours ) );

      i = strbuf.find_first_of("(", i2);
   }

   return true;
}

void
configureSymbolconf( const wdb2ts::config::ActionParam &params, 
		               SymbolConfProvider &symbolConfProvider )
{
	const string SYMBOL_PROVIDER_KEY("symbol_provider-");
	
	symbolConfProvider.clear();
	
	string::size_type i;

	for( wdb2ts::config::ActionParam::const_iterator it = params.begin(); 
     	  it!=params.end(); 
        ++it ) 
	{
		i=it->first.find( SYMBOL_PROVIDER_KEY );
		SymbolConfList symList;
		
		if( i != string::npos && i==0 ) {
			string provider=it->first;
			provider.erase(0, SYMBOL_PROVIDER_KEY.size() );
			//miutil::replace( provider, "_", " ");
			ProviderItem pi = ProviderList::decodeItem( provider );
			
			if( SymbolConf::parse(it->second.asString(), symList ) )
				symbolConfProvider[ pi.providerWithPlacename() ] = symList;
		}
	}
}

SymbolConfProvider
symbolConfProviderSetPlacename( const SymbolConfProvider &symbolConfProvider, 
		                          WciConnectionPtr wciConnection )
{
	WEBFW_USE_LOGGER( "handler" );

	SymbolConfProvider resList;
		
	for( SymbolConfProvider::const_iterator it=symbolConfProvider.begin(); 
	     it != symbolConfProvider.end(); 
	     ++it ) 
	{
		ProviderItem pi = ProviderList::decodeItem( it->first );
		
		if( ! pi.placename.empty() ) {
			resList[pi.providerWithPlacename()] = it->second;
			continue;
		}
		
		try {
			ProviderRefTimeList dummyRefTimeList;
			ProviderRefTime providerReftimeTransactor( dummyRefTimeList, 
					                                     pi.provider, 
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
					resList[tmp[i].providerWithPlacename()] = it->second;
			}
		}
		catch( const std::ios_base::failure &ex ) {
			WEBFW_LOG_ERROR( "std::ios_base::failure: LocationForecastHandler::providerPrioritySetPlacename: " << ex.what() );
		}
		catch( const std::runtime_error &ex ) {
			WEBFW_LOG_ERROR( "std::runtime_error: LocationForecastHandler::providerPrioritySetPlacename: " << ex.what() );
		}
		catch( const std::logic_error &ex ) {
			WEBFW_LOG_ERROR( "std::logic_error: LocationForecastHandler::providerPrioritySetPlacename: " << ex.what() );
		}
		catch( ... ) {
			WEBFW_LOG_ERROR( "unknown: LocationForecastHandler::providerPrioritySetPlacename" );
		}
	}
		
	return resList;			
}

SymbolConfProvider
symbolConfProviderWithPlacename( const wdb2ts::config::ActionParam &params, 
											const std::string &wdbDB,                           
											Wdb2TsApp *app
                               )
{
	WciConnectionPtr wciConnection;
	SymbolConfProvider tmpList;

	WEBFW_USE_LOGGER( "handler" );
		
	try {
		wciConnection = app->newWciConnection( wdbDB );
	}
	catch( exception &ex ) {
		WEBFW_LOG_ERROR( "symbolConfProviderWithPlacename: NO DB CONNECTION. " << ex.what() );
		return tmpList;
	}
	catch( ... ) {
		WEBFW_LOG_ERROR( "symbolConfProviderWithPlacename: NO DB CONNECTION. unknown exception " );
		return tmpList;
	}

	configureSymbolconf( params, tmpList );

	return symbolConfProviderSetPlacename( tmpList, wciConnection );
}

}

