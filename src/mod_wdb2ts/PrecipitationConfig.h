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


#ifndef __PRECIPITATION_CONFIG_H__
#define __PRECIPITATION_CONFIG_H__

#include <map>
#include <vector>
#include <RequestConf.h>

namespace wdb2ts {

class Wdb2TsApp;
typedef enum { PrecipMulti, PrecipSequence } PrecipConfigType;

struct PrecipConfigElement {
	PrecipConfigType precipType;
	std::vector<int> precipHours;

	PrecipConfigElement():
		precipType( PrecipSequence ) {}

	PrecipConfigElement( PrecipConfigType precipType_):
			precipType( precipType_ ) {}

	PrecipConfigElement( const PrecipConfigElement &elem )
		: precipType( elem.precipType ), precipHours( elem.precipHours ) {}

	PrecipConfigElement& operator=( const PrecipConfigElement &rhs )
		{
			if( this != &rhs ) {
				precipType = rhs.precipType;
				precipHours = rhs.precipHours;
			}

			return *this;
		}
};

class ProviderPrecipitationConfig {
	PrecipConfigElement defaultConfig;
	std::map<std::string, PrecipConfigElement > providerPrecipitationList;

	static bool decodeValue( const std::string &value, PrecipConfigElement &precip );

public:
	ProviderPrecipitationConfig();
	ProviderPrecipitationConfig( const ProviderPrecipitationConfig &precip );

	/**
	 * Lookup the precipitation configuration for the provider. If no config
	 * for the provider exist return the default precipitation configuration.
	 *
	 * @param provider Find the configuration for this provider.
	 * @return The configuration for the provider on success or 0 on error.
	 */
	const PrecipConfigElement *get( const std::string &provider )const;

	const PrecipConfigElement getDefault()const { return defaultConfig; };


	/**
	 * Configure the aggregation of precipitation.
	 * The form of the config element is:
	 *
	 *   -# key="precipitation" value="precipitationtype:hour1,hour2, .. ,hourN"
	 *   -# key="precipitation-provider" value="precipitationtype:hour1,hour2, .. ,hourN"
	 *
	 * The first form set the default method to generate the precipitation. In the second form
	 * can we specify a method that only is used for this provider.
	 *
	 * The precipitationtype can be: multi or sequence.
	 *
	 * In the multi method is precipitation generated for all times for all aggragation hours.
	 * For the sequence type is aggregation generated for each hour as long as the data time
	 * resolution allow, then we try the next hour.
	 *
	 * If now precipitation parameter is given the default is set to: sequence:1,3,6,12,24
	 *
	 * @param params A list of actionparams from the configuration file.
	 * @param app A pointer to the App class.
	 * @return A list of
	 */
	static
	ProviderPrecipitationConfig*
	configure( const wdb2ts::config::ActionParam &params,
			   Wdb2TsApp *app );

};


}

#endif /* PRECIPITATIONCONFIG_H_ */
