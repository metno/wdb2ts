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

#ifndef URLPARAMTIMESPEC_H_
#define URLPARAMTIMESPEC_H_

/**
 * @addtogroup wdb2ts
 * @{
 * @addtogroup mod_wdb2ts
 * @{
 */
/**
 * @file
 * This file contains the UrlParamTimeSpec class definition.
 */

// PROJECT INCLUDES
#include <UrlParam.h>

// SYSTEM INCLUDES
#include <boost/date_time/posix_time/posix_time.hpp>

namespace wdb2ts
{

/**
 * Decoding Class for Timespec Parameters from URI
 * 
 * @see UrlParam 
 */
class UrlParamTimeSpec : public UrlParam
{
public:
	boost::posix_time::ptime fromTime;
	boost::posix_time::ptime toTime;
	bool isDecoded;
	std::string indCode;
	std::string sameTimespecAsProvider;
	bool useProviderList_;
	
	/// Default Constructor
    UrlParamTimeSpec(  );
    UrlParamTimeSpec( int protocol );
    
    virtual void clean();
    
    /**
     * Decode the URI Time Spec string
     * @param toDecode	String to decode
     * @exception logic_error
     */
    virtual void decode( const std::string &toDecode );

private:
	/**
	 * Verify that a date string is a valid date
	 * @param dateString	date string to verify
	 * @return	true if date is valid, otherwise false
	 */
    bool validDate( const std::string &dateString );
    /**
     * Verify that an indeterminate code is valid
     * @param indCode	String to decode
     * @return	true if code is valid, otherwise false
     */ 
    bool validIndCode( const std::string &indCode );
    
    /**
     * If the timespec is set to a value with ${provider} use
     * the smae timespec as the provoder 'provider'.
     * 
     * @param[out] provider set the timespec of provider 'provider'
     *     as timespec for this. 
     * @return true if there is such a specification and false otherwise.
     */
protected:
	void decodeProtocol1( const std::string &toDecode );
	void decodeProtocol2( const std::string &toDecode );

public:
    bool sameTimespecAs( std::string &provider )const {
   	 				provider = sameTimespecAsProvider;
   	 				return ! sameTimespecAsProvider.empty();
    				}
    bool useProviderList() const { return useProviderList_; }
    
};


} // namespace

/**
 * @}
 *
 * @}
 */

#endif /*URLPARAMTIMESPEC_H_*/
