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
#include <UrlParamFormat.h>
// PROJECT INCLUDES
//
// SYSTEM INCLUDES
//

using namespace std;

namespace wdb2ts {
    
UrlParamFormat::
UrlParamFormat( UrlParamFormat::Format defFormat)
   : format_( defFormat ), defFormat_( defFormat )
{
    valid_ = true;
}

UrlParamFormat::
UrlParamFormat( int protocol, Format defFormat )
: UrlParam( protocol ), format_( defFormat ), defFormat_( defFormat )
{
	valid_ = true;
}

UrlParamFormat::
UrlParamFormat( int protocol )
 	: UrlParam( protocol ), format_( UNDEF_FORMAT ), defFormat_( UNDEF_FORMAT )
{
	valid_ = false;
}


UrlParamFormat::
UrlParamFormat()
 	:  format_( UNDEF_FORMAT ), defFormat_( UNDEF_FORMAT )
{
	valid_ = false;
}
    
void
UrlParamFormat::
clean()
{
	format_ = defFormat_;
	
	if( defFormat_ != UNDEF_FORMAT )
		valid_ = false;
	else
		valid_ = true;
	
}

void 
UrlParamFormat::
decode( const std::string &to )
{
    // NOOP
}

} // namespace

/**
 * @}
 *
 * @}
 */
