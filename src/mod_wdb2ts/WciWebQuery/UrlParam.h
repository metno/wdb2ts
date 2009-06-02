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

#ifndef __URL_PARAM_H__
#define __URL_PARAM_H__

#include <limits.h>
#include <float.h>
#include <string>
#include <list>
#include <stdexcept>

namespace wdb2ts {

class UrlParam 
{
public:
	/// Default Constructor
    UrlParam(): valid_(false), protocol( 1 ) 
    {
    	// NOOP
    }
    
    /// Default Constructor
    UrlParam( int protocol_ ): valid_(false), protocol( protocol_ ) 
    {
   	 // NOOP
    }
       
    /// Default Destructor
    virtual ~UrlParam()
    {
    	// NOOP
    }
    
    /**
     * Decode the input string with the appropriate 
     *  @param toDecode	String to decode
     * @exception logic_error
     */
    virtual void decode( const std::string &toDecode ) = 0;
    /**
     * Return the selected part of the decoded string
     */
    std::string selectPart() const
    {
    	return selectPart_; 
    } 
    
    virtual void clean() {
   	 selectPart_.erase();
   	 valid_=false;
    }
    
    /**
     * Check whether the decoding is valid.
     */
    bool valid() const 
    {
    	return valid_;
    }

protected:
		
	/// Text Selection
    std::string selectPart_;
    /// Validity Flag
    bool valid_;
    int protocol;
    
};

/*
class UrlParamDataversion : public UrlParamInt
{
public:
    UrlParamDataversion();
};
*/    
    
} // namespace

#endif 
