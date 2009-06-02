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

#ifndef URLPARAMFORMAT_H_
#define URLPARAMFORMAT_H_

/**
 * @addtogroup wdb2ts
 * @{
 * @addtogroup mod_wdb2ts
 * @{
 */
/**
 * @file
 * This file contains the UrlParamFormat class definition.
 */

// PROJECT INCLUDES
#include <UrlParam.h>
// SYSTEM INCLUDES
//

namespace wdb2ts
{

/**
 * Decoding Class for Data Format from URI
 * 
 * @see UrlParam 
 */
class UrlParamFormat : public UrlParam
{
public:
	/// The possible data formats supported by WDB2TS
	typedef enum { CSV, HTML, XML, UNDEF_FORMAT } Format;

    /// Default Constructor
    UrlParamFormat();
    
    UrlParamFormat( int protocol );
    /**
     *  Constructor specifying the default format
     *  @param	defFormat	Default format
     */
    UrlParamFormat( Format defFormat );
    
    UrlParamFormat(  int protocol, Format defFormat );
    /**
     * Return the format that the URI wants data in
     * @return The required data format
     * @exception logic_error
     */
    
    virtual void clean();
    Format format() const { return format_; }
    /**
     * Virtual decode function. Not used.
     * @param toDecode	String to decode
     */
    virtual void decode( const std::string &toDecode );

private:
	/// The WDB2TS return format
    Format format_;
    Format defFormat_;
};

} // namespace

/**
 * @}
 *
 * @}
 */

#endif /*URLPARAMFORMAT_H_*/
