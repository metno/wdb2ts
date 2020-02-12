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

#ifndef __LocationPointRead_H__
#define __LocationPointRead_H__

#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/shared_ptr.hpp>
#include <pqxx/transactor>
#include <ParamDef.h>
#include <ProviderList.h>
#include <UpdateProviderReftimes.h>
#include <PointDataHelper.h>
#include <LocationPoint.h>

namespace wdb2ts {


/**
 * Transactor class to fetch provider data from an wdb database.
 * The data is returned in as an PtrProviderRefTimes.
 * 
 */
class  LocationPointRead
	: public pqxx::transactor<>
{
public:
    LocationPointRead( float latitude, float longitude,
		               const ParamDefList &paramDefs,
		               const ProviderList &providers,
		               const ProviderRefTimeList &refTimeList,
		               const boost::posix_time::ptime &to,
		               LocationPointDataPtr locationPointData,
		               int wciProtocol);
	
	~LocationPointRead();

	void operator () ( argument_type &t );


private:
	float latitude_;
	float longitude_;
	const ParamDefList paramDefs_;
    const ProviderList &providers_;
    const ProviderRefTimeList &refTimeList_;
    const boost::posix_time::ptime &to_;
	int wciProtocol_;
	LocationPointDataPtr locationPointData_;
};

}

#endif 
