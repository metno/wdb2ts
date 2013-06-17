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

#include <Precipitation.h>

namespace wdb2ts {

PrecipitationAggregations::
PrecipitationAggregations()
{
}

void
PrecipitationAggregations::
findAllFromtimes( const boost::posix_time::ptime &toTime,
		          const std::string &provider,
		          std::set<boost::posix_time::ptime> &fromtimes )const
{
	boost::posix_time::ptime fromTime;
	std::map<boost::posix_time::ptime, Precipitation>::const_iterator fit;

	for(std::map< int, std::map<boost::posix_time::ptime, Precipitation> >::const_iterator it = begin();
	    it != end(); ++it )
	{
		fromTime = toTime - boost::posix_time::hours( it->first );
		fit = it->second.find( fromTime );

		if( fit != it->second.end() ) {
			if( provider.empty() || provider == fit->second.provider )
				fromtimes.insert( fromTime );
		}
	}
}

bool
PrecipitationAggregations::
findPrecip( const std::string &provider,
            const boost::posix_time::ptime &fromtime,
            const boost::posix_time::ptime &totime,
            Precipitation &precip ) const
{
	std::map< int, std::map<boost::posix_time::ptime, Precipitation> >::const_iterator it;
	boost::posix_time::time_duration duration= totime - fromtime;
	int timespanInHours = duration.hours();

	precip = Precipitation(); //Set all value to FLT_MAX.

	timespanInHours = abs( timespanInHours );

	it= find( timespanInHours );

	if( it == end() )
		return false;

	std::map<boost::posix_time::ptime, Precipitation>::const_iterator fit;
	fit = it->second.find( fromtime );

	if( fit == it->second.end() )
		return false;

	if( fit->second.provider != provider )
		return false;

	precip = fit->second;

	return true;
}

}

