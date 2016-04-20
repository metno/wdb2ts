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

#ifndef __RFC1123DATE_H__
#define __RFC1123DATE_H__

#include <string>
#include <stdexcept>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace miutil {
   
   /**
    * Takes an ptime and convert it to a time string coded in
    * the RFC 1123 format. This is the date format used by 
    * the HTTP protocol.
    * 
    * \b Ex. on RFC 1123 coded string
    *     
    *   Fri, 16 Mar 2007 08:13:37 GMT  
    * 
    * @param t the time to convert to a RFC 1123 coded string representation.
    * @return On success a string coded in the RFC 1123 format. On error an 
    * empty string is returned.
    */
   
   std::string rfc1123date(const boost::posix_time::ptime &t);
   
   
   /**
    * Takes an RFC 1123 coded date string and return an boost::posix_time.
    * 
    * \b Ex. on RFC 1123 coded string
    *     
    *   Fri, 16 Mar 2007 08:13:37 GMT  
    * 
    * @param rfc1123 coded data string.
    * @return An valid ptime on success and not_a_date_time on failure.
    */
   boost::posix_time::ptime rfc1123date(const std::string &rfc1123);
   
   
   /**
    * Takes an RFC 1123 coded date string and return an boost::posix_time.
    * 
    * \b Ex. on RFC 1123 coded string
    *     
    *   Fri, 16 Mar 2007 08:13:37 GMT  
    * 
    * @param rfc1123 coded data string.
    * @return An valid ptime on success and not_a_date_time on failure.
    */
   boost::posix_time::ptime rfc1123date(const char *rfc1123);
   
   /**
    * isotimeString generates a timeformatted string in one of two
    * iso compatible formats.
    * 
    *  - YYYY-MM-DD hh:mm:ss
    *  - YYYY-MM-DDThh:mm:ss
    * 
    * The second form separates the date part from the timepart with a T.
    * 
    * If markAsZulu is true an Z is added to the end of the timestring.
    * 
    * ex. 2008-02-15T18:00:00Z
    * 
    * An empty string is returned if the \em time is not a valid
    * ptime, ie. the call to is_special return true.
    * 
    * @param t the time to return as a string.
    * @param useTseparator is true if we want the datepart and timepart separated with a T.
    * @param markAsZulu Add an Z to the end of the timestring 
    * @return An iso compatible string on success or an empty string if \em t is invalid.
    */
   std::string  isotimeString(const boost::posix_time::ptime &t, 
   		                     bool useTseparator=false, 
   		                     bool markAsZulu=false);
   
   
   /**
    * ptimeFromIsoString decodes a time string on the followwing formates to a ptime.
    *
    * - YYYY-MM-DDThh:mm:ssZ
    * - YYYY-MM-DDThh:mm:ssSHHMM 
    * - YYYYMMDDThhmmssSHHMM
    * - Sun, 01 Apr 2007 09:51:04 GMT
    *  
    * Z is optional and means ZULU time (GMT).
    * S in front of HHMM stands for - or +. HHMM is an offset to UTC.
    * T is an optional separator it my bee an space.
    * 
    * If the timestring is invallid an logic_error exception is thrown.
    * 
    * @param isoTime A timestring.
    * @return An ptime.
    * @exception logic_error
    */
   boost::posix_time::ptime ptimeFromIsoString( const std::string &isoTime );
   
   /**
    * Construct a ptime with value as the geographical localtime. This is
    * NOT the same as the local time in effect on a given location.
    * But this i good enough to compute meteorological variation over
    * a day.
    *
    * @param longitude The longitude to return the localtime from.
    * @return A ptime that represent a local time.
    */
   boost::posix_time::ptime geologicalNowLocalTime( float longitude  );

   /**
    * Construct a ptime with value as the geographical localtime. This is
    * NOT the same as the local time in effect on a given location.
    * But this i good enough to compute meteorological variation over
    * a day.
    *
    * @param utcTime Convert this time, in utc, to local time at
    *                the longitude.
    * @param longitude The longitude to return the localtime from.
    * @return A ptime that represent a local time.
    */
   boost::posix_time::ptime geologicalLocalTime( const boost::posix_time::ptime &utcTime,
                                              float longitude  );

   /**
    * Creat a time_t from a posix time.
    *
    * Precondition: The time to convert must be greater
    * than the epoch, 1 Jan 1970 00:00:00;
    *
    * @param t The time to convert to time_t.
    * @return a time_t.
    * @exception std::range_error if t is less than epoch or
    *    t is not a time, ie t.is_special is true.
    */

   time_t to_time_t( const boost::posix_time::ptime &t );

   /**
    * nearestTimeInTheFuture compute the nearest time in the future
    * that is divisible by resolutionInSeconds. At the moment it only
    * computes the nearest time at max one day into the future, ie
    * the resolutionInSeconds must be in the interval [0, 86400].
    * @param resolutionInSeconds Valid values [0, 86400]
    * @param refTime Use this as the reference time to compute the nearest time. If
    * refTime.is_special it use the wall clock as reference time.
    * @param offsetInSeconds If the computation is not to reference at full hour.
    */
   boost::posix_time::ptime nearesTimeInTheFuture( int resolutionInSeconds, const boost::posix_time::ptime &refTime=boost::posix_time::ptime(), int offsetInSeconds=0);
}
#endif 
