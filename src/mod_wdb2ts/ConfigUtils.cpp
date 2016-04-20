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

#include <Logger4cpp.h>
#include <trimstr.h>
#include <splitstr.h>
#include <string>
#include <compresspace.h>
#include "ConfigUtils.h"

using namespace std;


namespace wdb2ts {

miutil::EnableTimePeriod
configEnableThunderInSymbols( const wdb2ts::config::ActionParam &conf )
{
	WEBFW_USE_LOGGER( "handler" );
	miutil::EnableTimePeriod thunder;
	wdb2ts::config::ActionParam::const_iterator it=conf.find("symbols_enable_thunder");

	if( it == conf.end() ) {
		WEBFW_LOG_INFO("Config: No specification for 'symbols_enable_thunder' is given. Thunder is enabled throughout the year.");
		return thunder;
	}

	string val = it->second.asString();

	thunder = miutil::EnableTimePeriod::parse( val );

	if( ! thunder.valid() ) {
		WEBFW_LOG_ERROR("Config: Invalid specification for 'symbols_enable_thunder', '" << val << "'. The format of the time period from/to is: MMM-DD/MMM-DD, where MMM is Jan, Feb, Mar, Apr, May, Jun, Jul, Aug, Sep, Oct, Nov and Dec. DD is the day in the month, 1 - 31. Thunder is NOT enabled.");
	} else {
		WEBFW_LOG_INFO("Config: Thunder, 'symbols_enable_thunder', enabled for period: " << thunder << ".");
	}

	return thunder;
}

int
configModelResolution(const wdb2ts::config::ActionParam &conf, int defaultResolutionInSeconds){
	WEBFW_USE_LOGGER( "handler" );
	wdb2ts::config::ActionParam::const_iterator it=conf.find("model_resolution");

	int res = defaultResolutionInSeconds;

	if( it == conf.end() ) {
		WEBFW_LOG_INFO("Config: No specification for 'model_resolution'. Using default:" << defaultResolutionInSeconds << " seconds.");
		return defaultResolutionInSeconds;
	}
	try {
		res = it->second.asInt();
	}
	catch( const std::exception &ex ) {
		WEBFW_LOG_ERROR("Config: Failed to read 'model_resolution'. " << ex.what() << ". Using default:" << defaultResolutionInSeconds << " seconds.");
	}

	WEBFW_LOG_INFO("Config: 'model_resolution': " << res);
	return res;
}


NoDataResponse
NoDataResponse::
decode( const wdb2ts::config::ActionParam &conf )
{
    wdb2ts::config::ActionParam::const_iterator it=conf.find("no_data");

    if( it != conf.end() ) {
        string val = it->second.asString();
        if( strcasecmp(val.c_str(), "NOT_FOUND") == 0 ) {
            return NoDataResponse( NotFound );
        } else if( strcasecmp(val.c_str(), "SERVICE_UNAVAILABLE") == 0 ) {
            return NoDataResponse( ServiceUnavailable );
        }
    }

    return NoDataResponse( NotDefined );
}

bool
NoDataResponse::
doThrow()const
{
    if( response != NotDefined )
        return true;
    else
        return false;
}

std::ostream&
operator<<( std::ostream &o, const NoDataResponse &re )
{
    switch( re.response ) {
    case NoDataResponse::NotDefined: o << "NotDefined"; break;
    case NoDataResponse::NotFound: o << "NotFound"; break;
    case NoDataResponse::ServiceUnavailable: o << "ServiceUnavailable"; break;
    default:
        break;
    }
    return o;
}



bool
OutputParams::
paramFlag( std::string &param )const
{
   bool flag=true;

   miutil::trimstr(param);

   while( param.length() > 0 &&
         (param[0]=='-' || param[0]=='+') ) {
      if( param[0]=='-') flag=false;
      else flag=true;
      param.erase( 0, 1 );
      miutil::trimstr(param);
   }
   return flag;
}

OutputParams::
OutputParams()
{
}

bool
OutputParams::
setParam( const std::string &param, bool flag )
{
   imap::iterator it = defParams.find( param );

   if( it == defParams.end() ) {
      return false;
   } else {
      it->second = flag;
      return true;;
   }
}

bool
OutputParams::
setParam( const std::string &param_ )
{
   string param(param_);
   bool flag = paramFlag( param );
   return setParam( param, flag );
}

void
OutputParams::
addParam( const std::string &param, bool f )
{
   defParams[param]=f;
}

bool
OutputParams::
useParam( const std::string &param ) const
{
   imap::const_iterator it = defParams.find( param );

   if( it == defParams.end() )
      return true;
   else
      return it->second;
}

OutputParams
OutputParams::
decodeOutputParams( const wdb2ts::config::ActionParam &conf )
{
   WEBFW_USE_LOGGER( "handler" );

   OutputParams params;
   string param;
   wdb2ts::config::ActionParam::const_iterator it;

   params.addParam("dewpointTemperature", false );
   params.addParam("symbol:T.WB", true );
   params.addParam("seaIceingIndex",  false );
   params.addParam("significantSwellWaveHeight",  false );
   params.addParam("meanSwellWavePeriode",  false );
   params.addParam("meanSwellWaveDirection",  false );
   params.addParam("peakSwellWavePeriode",  false );
   params.addParam("peakSwellWaveDirection",  false );
   params.addParam("meanTotalWavePeriode",  false );
   params.addParam("maximumTotalWaveHeight",  false );


   it = conf.find("output_params");

   if( it == conf.end()) {
      WEBFW_LOG_DEBUG("No output_params defined. This is OK, using default!");
      return params;
   }

   vector<string> vals = miutil::splitstr( it->second.asString(), ';' );

   for( vector<string>::const_iterator it=vals.begin();
        it != vals.end(); ++it ) {

      param = *it;
      miutil::compres( param, ":");

      if( ! params.setParam( param ) ) {
         WEBFW_LOG_ERROR( "output_params: Unknown parameter name '"
               << param << "'");
      }
   }

   return params;
}

std::ostream&
operator<<(std::ostream &s, const OutputParams &op )
{
   for( OutputParams::imap::const_iterator it=op.defParams.begin();
         it != op.defParams.end(); ++it ) {
      if( it != op.defParams.begin() )
         s << ", ";
      s << it->first << "=" << (it->second?"T":"F");
   }

   return s;
}


}
