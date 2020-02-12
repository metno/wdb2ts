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
   
      precipHours = INT_MAX;
      if ( vals.size() == 3 ) {
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
      } else if ( vals.size() == 1 ) {
         for ( vector<string>::size_type k=0; k < vals[0].size(); ++k )
            if ( ! isdigit( vals[0][k] ) )
               return false;

         if ( sscanf( vals[0].c_str(), "%d", &precipHours ) == 0 )
            return false;
         min = INT_MIN;
         max = INT_MAX;
      } else {
         return false;
      }
      
      conf.push_back( SymbolConf( min, max, precipHours ) );

      i = strbuf.find_first_of("(", i2);
   }

   return true;
}


SymbolConfProvider::
SymbolConfProvider()
	: maxHours_( 0 )
{
}

SymbolConfList
SymbolConfProvider::
get( const std::string &provider )const
{
	const_iterator it = find( provider );

	if( it != end() )
		return it->second;

	ProviderItem item( ProviderItem::decode( provider ) );
	it = find( item.provider );

	if( it != end() ) {
		const_cast<SymbolConfProvider&>(*this)[provider] = it->second;
		return it->second;
	}

	const_cast<SymbolConfProvider&>(*this)[provider] = defaultConf;

	return const_cast<SymbolConfProvider&>(*this)[provider];
}

void SymbolConfProvider::merge(const SymbolConfProvider &other){
	for( auto &sc : other ) {
		add( sc.first, sc.second);
	}
}

void
SymbolConfProvider::
add( const std::string &provider, const SymbolConfList &conf )
{
	int h;
	for( SymbolConfList::const_iterator it = conf.begin();
		 it != conf.end(); ++it ) {
		h=0;
		if( it->min() != INT_MAX && it->max() != INT_MAX ) {
			h = it->min() + it->max();
			if( h == 0 ) h=1;
		} else if( it->precipHours() != INT_MAX ) {
			h = it->precipHours();
		}

		if( h > maxHours_ )
			maxHours_ = h;
	}

	(*this)[provider] = conf;
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
				symbolConfProvider.add( pi.providerWithPlacename(), symList );
		}
	}
}
std::ostream&
operator<<( std::ostream &ost, const SymbolConf &conf )
{
	ost << "min: " << conf.min_ << " max: " << conf.max_ << " precipHours: " << conf.precipHours_;
	return ost;
}
}

