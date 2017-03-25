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

#include <sys/time.h>
#include <unistd.h>
#include <boost/date_time/gregorian_calendar.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "gettimeofday.h"

namespace pt=boost::posix_time;
namespace gd=boost::gregorian;

#if 0
double
miutil::
gettimeofday()
{
    pt::ptime epoch( gd::date(1970, gd::Jan, 1), pt::time_duration(0,0,0) );
    pt::ptime now( pt::microsec_clock::universal_time() );
    pt::time_duration dif = now - epoch;

    return dif.total_microseconds()/1000000.0;
}

#endif

double
miutil::
gettimeofday()
{
	struct timeval tv;
	
	if(gettimeofday(&tv, 0)!=0)
		return -1.0l;
	
	return (double)tv.tv_sec+((double)tv.tv_usec)/1000000.0;
}
