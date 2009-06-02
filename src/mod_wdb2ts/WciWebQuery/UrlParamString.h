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

#ifndef URLPARAMSTRING_H_
#define URLPARAMSTRING_H_
/**
 * @addtogroup wdb2ts
 * @{
 * @addtogroup mod_wdb2ts
 * @{
 */
/**
 * @file
 * This file contains the UrlParamString class definition.
 */

// PROJECT INCLUDES
#include <UrlParam.h>
// SYSTEM INCLUDES
#include <string>
#include <list>
#include <algorithm>

namespace wdb2ts
{

/**
 * Decoding Class for String Parameters from URI
 * 
 * @see UrlParam 
 */
class UrlParamString : public UrlParam
{
	///Shall the string decodes to an SQL array.
	bool array;
	std::string defValue;
	
public:

	typedef std::list<std::string> ValueList;
	///The decoded values.
	ValueList valueList;
	
	bool hasValue( const std::string &value )const;
	
	/**
	 * Update the valueList from a container value.
	 * The select part is updated.
	 */
	template< class T>
	void setValueList( const T &value ) {
		valueList.clear();

		for( typename T::const_iterator it = value.begin(); it != value.end(); ++it )
			valueList.push_back( *it );
		
		updateSelectPart();
	}
	
	void updateSelectPart();
	
	/**
	 * Default Constructor
	 * The default value of the string is set to \em NULL.
	 * array is set to true.
	 */
	UrlParamString();
	
	UrlParamString( int protocol );
	
	///Set the default value of the string to the \em defaultValue. 
	UrlParamString( const std::string &defaultValue, bool decodeToArray=true );
	UrlParamString( int protocol, const std::string &defaultValue, bool decodeToArray=true );
	
	/// Default Destructor
	virtual ~UrlParamString();
	
	virtual void clean();

	/**
	 * Return the list of values decoded by UrlParamString
	 * @return	List of values
	 */ 
    std::string selectPart() const;
    
    /**
     * Decoding function for UrlParamString
     * @param toDecode	String to decode
     * @exception logic_error
     */
    virtual void decode( const std::string &toDecode );
    
};

} // namespace

/**
 * @}
 *
 * @}
 */

#endif /*URLPARAMSTRING_H_*/
