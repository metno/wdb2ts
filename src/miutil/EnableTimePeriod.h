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

#ifndef __ENABLETIMEPERIOD_H__
#define __ENABLETIMEPERIOD_H__

#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace miutil {

struct EnableTimePeriod {
	int fromDay; //From day =0, all days is enabled. From day <0 an invalid spec is given, no days is enabled.
	int fromMonth;
	int toDay;
	int toMonth;
	EnableTimePeriod(): fromDay(0), fromMonth( -1 ), toDay( -1 ), toMonth( -1 ) {}
	EnableTimePeriod( int dayFrom, int monthFrom=-1, int dayTo=-1, int monthTo=-1);

	/**
	 * duration is on the form  MM-DD/MM-DD
	 */
	static EnableTimePeriod parse( const std::string &duration );

	boost::gregorian::date_period enabledPeriod(const boost::gregorian::date &refTime=boost::gregorian::day_clock::universal_day() )const;
	bool valid()const { return fromDay>-1; }
	bool isEnabled( const boost::posix_time::ptime &t, const boost::gregorian::date &refTime=boost::gregorian::day_clock::universal_day() )const;
};

}

#endif
