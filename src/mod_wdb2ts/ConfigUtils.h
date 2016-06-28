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

#ifndef __CONFIGUTILS_H__
#define __CONFIGUTILS_H__

#include <cstring>
#include <string>
#include <map>
#include <configparser/RequestConf.h>
#include <EnableTimePeriod.h>

namespace wdb2ts {

miutil::EnableTimePeriod
configEnableThunderInSymbols( const wdb2ts::config::ActionParam &conf );

int
configModelResolution( const wdb2ts::config::ActionParam &conf, int defaultResolutionInSeconds=3600 );

bool configForcast( const wdb2ts::config::ActionParam &conf);

struct NoDataResponse {
    typedef enum{ NotDefined,ServiceUnavailable, NotFound } ENoDataResponse;
    ENoDataResponse response;

    NoDataResponse():response(NotDefined) {};
    NoDataResponse( ENoDataResponse response ):response(response) {};

    bool doThrow()const;
    friend std::ostream& operator<<( std::ostream o, const NoDataResponse &re );
    static NoDataResponse decode( const wdb2ts::config::ActionParam &conf );
};


class OutputParams {
   struct ILess {
      bool operator()( const std::string &s1, const std::string &s2 )const
      {
         return strcasecmp( s1.c_str(), s2.c_str() ) < 0;
      }
   };
   typedef std::map<std::string, bool, ILess> imap;

   bool paramFlag( std::string &param )const;
   imap defParams;

public:
   OutputParams();

   /**
    * Only set the param  to flag if it alleady exist in the
    * defined set of parameters.
    *
    * @param param The param to set to true if it exist.
    * @return true if the parameter exist and false otherwise.
    */
   bool setParam( const std::string &param, bool flag );

   /**
    * Only set the param if it alleady exist in the
    * defined set of parameters. A flag in front
    * of the param decide if it is set to true
    * or false. Valid flag in front is '-' and '+'.
    * No flag is the same as '+'.
    *
    * @param param The param to set to true if it exist.
    * @return true if the parameter exist and false otherwise.
    */
    bool setParam( const std::string &param );

   void addParam( const std::string &param, bool f );
   bool useParam( const std::string &param ) const;

   friend std::ostream& operator<<(std::ostream &s, const OutputParams &op );
   static OutputParams
   decodeOutputParams( const wdb2ts::config::ActionParam &conf );
};

std::ostream&
operator<<(std::ostream &s, const OutputParams &op );

}


#endif
