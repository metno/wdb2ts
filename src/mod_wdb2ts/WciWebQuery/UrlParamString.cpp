/*
    wdb - weather and water data storage

    Copyright (C) 2008 met.no

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

/**
 * @addtogroup wdb2ts 
 * @{
 * @addtogroup mod_wdb2ts
 * @{
 */
/**
 * @file
 * Implementation of the UrlParamString class.
  */
#ifdef HAVE_CONFIG_H
#include <wdb2ts_config.h>
#endif
// CLASS
#include <UrlParamString.h>
// PROJECT INCLUDES
//
// SYSTEM INCLUDES
#include <iostream>
#include <sstream>
#include <vector>
#include <splitstr.h>
#include <trimstr.h>

using namespace std;

namespace wdb2ts {

UrlParamString::
UrlParamString(): defValue("")
{
    array = true;
    decode( defValue );
}

UrlParamString::
UrlParamString( int protocol )
	: UrlParam( protocol ), defValue("")
{
    array = true;
    decode( defValue );
}

UrlParamString::
UrlParamString( const std::string &defaultValue,
		          bool decodeToArray )
	: defValue( defaultValue )
{
	array = decodeToArray;
	decode( defValue );
}

UrlParamString::
UrlParamString( int protocol, 
		          const std::string &defaultValue,
		          bool decodeToArray )
	: UrlParam( protocol ), defValue( defaultValue )
{
	array = decodeToArray;
	decode( defValue );
}


UrlParamString::
~UrlParamString()
{
	// NOOP
}

void 
UrlParamString::
clean()
{
	decode( defValue );
}


void 
UrlParamString::
updateSelectPart()
{
	ostringstream ost;
	bool first=true;
	
	if( ! array ) {
		if( valueList.empty() )
			ost << "NULL";
		else
			ost << "'" << valueList.front() << "'";
		   	 
		selectPart_ = ost.str();
		valid_ = true;
		return;
	}
		    
	for( std::list<std::string>::iterator it=valueList.begin();
	     it != valueList.end();
	     ++it )
	{
		if( first ) {
			ost << "ARRAY[ '" << *it << "'";
			first = false;
		} else {
			ost << ", '" << *it << "'"; 
		}
	}

	// Tidy up
	if( !first ) 
		ost << " ]";
	else
		ost << "NULL";
		    
	selectPart_ = ost.str();
	valid_ = true;
}

std::string 
UrlParamString::
selectPart() const
{
	return selectPart_; 
}

bool 
UrlParamString::
hasValue( const std::string &value ) const
{
	for( std::list<std::string>::const_iterator it=valueList.begin();
	     it!=valueList.end();
	     ++it ) 
		if( *it == value )
	     return true;
	
	return false;
}


void 
UrlParamString::
decode( const std::string &toDecode )
{
    ostringstream ost;
    bool first=true;
    string buf;
    valueList.clear();
    
    if( !array ) {
   	 buf=toDecode;
   	 miutil::trimstr( buf, miutil::TRIMBOTH, " \r\n\t\"" );
   	 
   	 if( ! buf.empty() ){
   		 valueList.push_back( buf );
   		 ost << "'" << buf << "'";
   	 }else {
   		 ost << "NULL";
   	 }
   	 
   	 selectPart_ = ost.str();
   	 valid_ = true;
   	 return;
    }
    
    vector<std::string> vals=miutil::splitstr(toDecode, ',');
    
    for( vector<std::string>::size_type i=0; i<vals.size(); ++i ) {
    	buf = vals[i];
    	miutil::trimstr( buf, miutil::TRIMBOTH, " \r\n\t\"" );
	
    	if( ! buf.empty() ) {     
    		valueList.push_back( buf );
    		
    		if( first ) {
    			ost << "ARRAY[ '" << buf << "'";
    			first =false;
    		} 
    		else {
    			ost << ", '" << buf << "'";
    		}
    	}
    }
    // Tidy up
    if( !first ) 
    	ost << " ]";
    else
    	ost << "NULL";
    
    selectPart_ = ost.str();
    valid_ = true;
}

} // namespace

/**
 * @}
 *
 * @}
 */
