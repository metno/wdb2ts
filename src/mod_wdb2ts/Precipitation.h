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

#ifndef __PRECIPITATION_H__
#define __PRECIPITATION_H__

#include <float.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <string>
#include <map>
#include <set>

namespace wdb2ts {

struct Precipitation {
	std::string provider;
	float  precip;
	float  min;
	float  max;
	float  prob;

	Precipitation():precip(FLT_MAX), min( FLT_MAX ), max( FLT_MAX ), prob( FLT_MAX ) {}
	Precipitation( const Precipitation &p )
      : provider( p.provider ), precip( p.precip ), min( p.min ), max( p.max ), prob( p.prob )
        {}
	Precipitation( const std::string &provider_, float precip_, float min=FLT_MAX, float max = FLT_MAX, float prob = FLT_MAX )
	    : provider( provider_ ), precip( precip_ ), min( min ), max( max ), prob( prob ) {}

	Precipitation& operator=( const Precipitation &rhs ) {
		if( this != &rhs ) {
			provider = rhs.provider;
			precip = rhs.precip;
			min = rhs.min;
			max = rhs.max;
			prob = rhs.prob;
		}

		return *this;
	}

};

///Hold the precip agragations. The first argument is the number of hours the aggregation is for.

class PrecipitationAggregations :
	  public std::map< int, std::map<boost::posix_time::ptime, Precipitation>  >
{
public:

   PrecipitationAggregations();

   void findAllFromtimes( const boost::posix_time::ptime &toTime,
		                  const std::string &provider,
		                  std::set<boost::posix_time::ptime> &fromtimes )const;

   bool findPrecip( const std::string &provider,
	                const boost::posix_time::ptime &fromtime,
	                const boost::posix_time::ptime &totime,
	                Precipitation &precip ) const;



};
}


#endif /* PRECIPAGGREGATION_H_ */
