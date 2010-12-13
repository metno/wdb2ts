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
#include <UrlParamFloat.h>
// PROJECT INCLUDES
//
// SYSTEM INCLUDES
#include <stdio.h>
#include <sstream>

using namespace std;

namespace wdb2ts {

UrlParamFloat::   
UrlParamFloat( float defVal)
    : value_( FLT_MIN)
{
	if( defVal  != FLT_MIN ) 
		valid_ = true;
}

UrlParamFloat::   
UrlParamFloat( int protocol, float defVal )
	: UrlParam( protocol )
{
	if( defVal  != FLT_MIN ) 
			valid_ = true;
}
   
void
UrlParamFloat::
clean()
{
	value_ = FLT_MIN;
	valid_ = false;
}

void 
UrlParamFloat::
decode( const std::string &to )
{
    float f;
    
    if( sscanf(to.c_str(),"%f", &f) != 1 ) {
    	ostringstream ost;
    	ost << "Not a number: " << to;
    	throw logic_error( ost.str() );
    }
    
    value_ = f;
    valid_ = true;
}

} // namespace

/**
 * @}
 *
 * @}
 */
