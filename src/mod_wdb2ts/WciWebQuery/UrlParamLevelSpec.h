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

#ifndef URLPARAMLEVELSPEC_H_
#define URLPARAMLEVELSPEC_H_

/**
 * @addtogroup wdb2ts
 * @{
 * @addtogroup mod_wdb2ts
 * @{
 */
/**
 * @file
 * This file contains the UrlParamLevelSpec class definition.
 */

// PROJECT INCLUDES
#include <UrlParam.h>
// SYSTEM INCLUDES
//

namespace wdb2ts
{

/**
 * Decoding Class for Level Parameter Specification from URI
 * 
 * @see UrlParam 
 */
class UrlParamLevelSpec : public UrlParam
{
public:
	/// Default Constructor
	UrlParamLevelSpec(  );
	
	UrlParamLevelSpec( int protocol );
    /**
     * Decode the string
     * @param toDecode	Input string to decode
     * @exception logic_error
     */
    virtual void decode( const std::string &toDecode );
    
    virtual void clean(); 

protected:
	
	void decodeProtocol1( const std::string &toDecode );
	void decodeProtocol2( const std::string &toDecode );
	
	/**
	 * Verify that the Level given is valid
	 * @param level	The level string
	 * @return	True if the level is valid, false otherwise
	 */	
    bool validLevel( const std::string &level );
	/**
	 * Verify that the Level indeterminate code given is valid
	 * @param level	The level string
	 * @return	True if the level IC is valid, false otherwise
	 */	
    bool validIndCode( const std::string &indCode );
	/**
	 * Verify that the Level parameter given is valid
	 * @param level	The level string
	 * @return	True if the level parameter is valid, false otherwise
	 */	
    bool validLeveldomain( const std::string &leveldomain );
};

} // namespace

/**
 * @}
 *
 * @}
 */

#endif /*URLPARAMLEVELSPEC_H_*/
