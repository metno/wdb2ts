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
 * Implementation of the UrlParamDataProvider class.
  */
#ifdef HAVE_CONFIG_H
#include <wdb2ts_config.h>
#endif
// CLASS
#include <UrlParamParameter.h>
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

UrlParamParameter::
UrlParamParameter( )
{
    valid_ = true;
    selectPart_ = "NULL";
}

UrlParamParameter::
UrlParamParameter( int protocol )
	: UrlParam( protocol )
{
    valid_ = true;
    selectPart_ = "NULL";
}


void
UrlParamParameter::
clean()
{
	valid_ = true;
	selectPart_ = "NULL";
}
    
void 
UrlParamParameter::
decode( const std::string &toDecode )
{  
   ostringstream ost;
   bool first=true;
   string buf;
   vector<string> vals=miutil::splitstr(toDecode, ',');
    
   for( int i=0; i<vals.size(); ++i ) {
   	buf = vals[i];
   	miutil::trimstr( buf, miutil::TRIMBOTH, " \t\r\n\"" );
	
   	if( buf.empty() )
   		continue;
      
   	if( first ) {
   		ost << "array['" << buf << "'";
   		first =false;
   	} 
   	else
   		ost << ", '" << buf << "'";
   }
    
   if( !first ) 
   	ost << "]";
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
