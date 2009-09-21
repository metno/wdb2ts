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

#ifndef __WCI_READ_LOCATIONFORECAST_H__
#define __WCI_READ_LOCATIONFORECAST_H__


#include <pqxx/transactor>
#include <string>
#include <PointDataHelper.h>
#include <UpdateProviderReftimes.h>
#include <ParamDef.h>
#include <Config.h>

namespace wdb2ts {


/**
 * Transactor class to fetch data from an wdb database.
 * The data is returned in an TimeSeriePtr.
 */
class WciReadLocationForecast 
	: public pqxx::transactor<>
{
public:
	WciReadLocationForecast(float latitude, float longitude, int altitude,
	      					const ParamDefList &paramDefs,
	      					PtrProviderRefTimes refTimes,
	      					const ProviderList  &providerPriority,
	      					const wdb2ts::config::Config::Query &urlQuerys,
	      					int wciProtocol );
	
	~WciReadLocationForecast();

	void operator () ( argument_type &t );

	wdb2ts::LocationPointDataPtr result() const { return locationPointData; }

	void on_abort( const char msg_[] )throw (); 

	
private:
	const float latitude;
	const float longitude;
	const int   altitude;
	bool  polygonRequest;
	const ParamDefList &paramDefs;
	PtrProviderRefTimes refTimes;
	const ProviderList  &providerPriority;
	const wdb2ts::config::Config::Query &urlQuerys;
	const int wciProtocol;

	wdb2ts::LocationPointDataPtr locationPointData;
};

}

#endif 
