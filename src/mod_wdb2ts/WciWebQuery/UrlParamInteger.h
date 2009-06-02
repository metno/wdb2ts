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

#ifndef URLPARAMINTEGER_H_
#define URLPARAMINTEGER_H_

/**
 * @addtogroup wdb2ts
 * @{
 * @addtogroup mod_wdb2ts
 * @{
 */
/**
 * @file
 * This file contains the UrlParamInteger class definition.
 */

// PROJECT INCLUDES
#include <UrlParam.h>
// SYSTEM INCLUDES
//

namespace wdb2ts
{

/**
 * Decoding Class for Integers from URI
 * 
 * @see UrlParam 
 */
class UrlParamInteger : public UrlParam
{
public:
	/**
	 *  Default Constructor
	 * @param array		Indicates whether the parameter is an array
	 * @param defVal	The default value of the parameter
	 */
    UrlParamInteger( bool array=false , int defVal=INT_MIN );
    UrlParamInteger( int protocol, bool array=false , int defVal=INT_MIN );
    
    /**
     * reset the value to the default value
     */
    virtual void clean();
    
    /**
     * Return the Values given
     * @return	The list of integer values
     */
    std::list<int> value()const { return value_; }
    /**
     * Decode the string
     * @param toDecode	Input string to decode
     * @exception logic_error
     */
    virtual void decode( const std::string &toDecode );
    
private:
	/// Boolean indicator as to whether the parameter is an array
    bool array;
    /// The list of values
    std::list<int> value_;
    std::list<int> defValue_;
    std::string defSelectPart_;
    
};   

} // namespace

/**
 * @}
 *
 * @}
 */

#endif /*URLPARAMINTEGER_H_*/
