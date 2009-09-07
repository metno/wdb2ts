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
#include <algorithm>
#include <iostream>
#include <sstream>
#include <replace.h>
#include <splitstr.h>
#include <trimstr.h>
#include <Logger4cpp.h>
#include <wdb2TsApp.h>
#include <PrecipitationConfig.h>

using namespace std;

namespace wdb2ts {

/*
PrecipConfigElement defaultConfig;
std::map<std::string, PrecipConfigElement > ProviderPrecipitationList;
*/

ProviderPrecipitationConfig::
ProviderPrecipitationConfig()
{

}

ProviderPrecipitationConfig::
ProviderPrecipitationConfig( const ProviderPrecipitationConfig &precip )
{

}

bool
ProviderPrecipitationConfig::
decodeValue( const std::string &value, PrecipConfigElement &precip )
{
	WEBFW_USE_LOGGER( "handler" );

	vector<string> pp = miutil::splitstr( value, ':' );

	if( pp.size() > 2 ) {
		WEBFW_LOG_ERROR("precipitation spec: invalid format. Valid format: (multi|sequence):h1,h2,..,hN. Ex. multi:3,6");
		return false;
	}

	precip.precipHours.clear();

	if( pp[0]=="multi")
		precip.precipType = PrecipMulti;
	else if( pp[0]=="sequence")
		precip.precipType = PrecipSequence;
	else {
		WEBFW_LOG_ERROR("precipitation spec: invalid format. Valid format: (multi|sequence):h1,h2,..,hN. Ex. multi:3,6");
		return false;
	}

	if( pp.size() == 1 ) {

		if( precip.precipType == PrecipMulti ) {
			WEBFW_LOG_WARN("precipitation spec (PrecipMulti): No aggregation hours given. Setting hours to 1, 2, 3 and 6 as default.");
			precip.precipHours.push_back( 1 );
			precip.precipHours.push_back( 2 );
			precip.precipHours.push_back( 3 );
			precip.precipHours.push_back( 6 );
		} else {
			WEBFW_LOG_WARN("precipitation spec (PrecipSequence): No aggregation hours given. Setting hours to 1, 3, 6 and 12 as default.");
			precip.precipHours.push_back( 1 );
			precip.precipHours.push_back( 3 );
			precip.precipHours.push_back( 6 );
			precip.precipHours.push_back( 12 );
			precip.precipHours.push_back( 24 );
		}

		return true;
	}

	pp = miutil::splitstr( pp[1], ',' );

	for( vector<string>::size_type i=0; i<pp.size(); ++i ) {
		int n;
		if( sscanf( pp[i].c_str(), "%d", &n) != 1 ) {
			WEBFW_LOG_ERROR("precipitation spec: invalid format. Hour spec not a number. Valid format: (multi|sequence):h1,h2,..,hN. Ex. multi:3,6");
			return false;
		}

		precip.precipHours.push_back( n );
	}

	return true;
}

const PrecipConfigElement *
ProviderPrecipitationConfig::
get( const std::string &provider )const
{
	std::map<std::string, PrecipConfigElement >::const_iterator it = providerPrecipitationList.find( provider );

	if( it != providerPrecipitationList.end() )
		return &it->second;

	return &defaultConfig;
}


ProviderPrecipitationConfig*
ProviderPrecipitationConfig::
configure( const wdb2ts::config::ActionParam &params,
		   Wdb2TsApp *app )
{
	const string PRECIPITATION_KEY("precipitation-");
	ProviderPrecipitationConfig *conf;
	PrecipConfigElement configElement;
	wdb2ts::config::ActionParam::const_iterator it;

	WEBFW_USE_LOGGER( "handler" );

	it=params.find("precipitation");

	if( it != params.end() ) {
		if( ! decodeValue( it->second.asString(), configElement ) )
			return 0;
	} else {
		configElement.precipType = PrecipSequence;
		configElement.precipHours.push_back( 1 );
		configElement.precipHours.push_back( 3 );
		configElement.precipHours.push_back( 6 );
		configElement.precipHours.push_back( 12 );
		configElement.precipHours.push_back( 24 );
	}

	try {
		conf = new ProviderPrecipitationConfig();
	}
	catch( ... ) {
		WEBFW_LOG_CRIT( "Precip configuration: NO MEM");
		return 0;
	}

	conf->defaultConfig = configElement;

	for( wdb2ts::config::ActionParam::const_iterator it = params.begin();
     	  it!=params.end();
        ++it )
	{
		string::size_type i = it->first.find( PRECIPITATION_KEY );

		if( i != string::npos && i==0 ) {
			string provider = it->first;
			provider.erase(0, PRECIPITATION_KEY.size() );

			if( ! decodeValue( it->second.asString(), configElement ) )
				continue;

			conf->providerPrecipitationList[provider] = configElement;
		}
	}

	return conf;
}


}
