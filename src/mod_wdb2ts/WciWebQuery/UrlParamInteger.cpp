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
 * Implementation of the UrlParamFormat class.
  */
#ifdef HAVE_CONFIG_H
#include <wdb2ts_config.h>
#endif
// CLASS
#include <UrlParamInteger.h>
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
    
UrlParamInteger::
UrlParamInteger( bool array, int defVal )
    : array( array )
{
    ostringstream ost;
    
    valid_ = true;
    
    if( defVal != INT_MIN ) {
    	value_.push_back( defVal );
	
    	if( array ) 
    		ost << "array[" << defVal << "]";
    	else
    		ost << defVal;
    	
    	selectPart_= ost.str();
    }
    else {
    	selectPart_ = "NULL";
    }
    
    defValue_ = value_;
    defSelectPart_ = selectPart_;
}

UrlParamInteger::
UrlParamInteger( int protocol, bool array, int defVal )
	: UrlParam( protocol ), array( array )
{
   ostringstream ost;
   
   valid_ = true;
   
   if( defVal != INT_MIN ) {
   	value_.push_back( defVal );
	
   	if( array ) 
   		ost << "array[" << defVal << "]";
   	else
   		ost << defVal;
   	
   	selectPart_= ost.str();
   }
   else {
   	selectPart_ = "NULL";
   }
   
   defValue_ = value_;
   defSelectPart_ = selectPart_;
}

void
UrlParamInteger::
clean()
{
	value_ = defValue_;
	selectPart_ = defSelectPart_;
	valid_ = true;
}


void 
UrlParamInteger::
decode( const std::string &to )
{
    ostringstream ost;
    int i;
    int n;
    vector<string> vals=miutil::splitstr(to, ',');
    
    value_.clear();
    
    for( n=0; n<vals.size(); ++n ) {
		if( sscanf(vals[n].c_str(),"%d", &i) != 1 ) {
		    ost << "Not a number: " << to;
		    throw logic_error( ost.str() );
		}
		
		value_.push_back( i );
    }
    
    if( array ) {
		if( ! value_.empty() ) {
		    list<int>::const_iterator it=value_.begin();
		    
		    ost << "array[" << *it;
		    
		    for( ++it ; it!=value_.end() ; ++it )
			ost << "," << *it;
		    
		    ost << "]";
		}
		else
		    ost << "NULL";
    } 
    else if( value_.size() == 0 )
    	ost << "NULL";
    else if( value_.size() == 1 )
    	ost << *value_.begin();
    else {
    	ost << "Expecting only one value, NOT a list of values.";
		throw logic_error( ost.str() );
    }
    valid_ = true;
    selectPart_ = ost.str();
}

} // namespace

/**
 * @}
 *
 * @}
 */
