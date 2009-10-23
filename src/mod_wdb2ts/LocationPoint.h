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

#ifndef __LOCATIONPOINT_H__
#define __LOCATIONPOINT_H__

#include <stdexcept>
#include <list>
#include <limits.h>

namespace wdb2ts {

class LocationPoint;



/**
 *  LocationPoint hold a point as two integer. The latitude and longitude
 *  is given in decimal grades.
 *
 *  The coding into integer is done as int(dg*10000), ie a resolution on 5 desimals.
 *  The decoding back to desimal grades is: i/10000.
 */
class LocationPoint {
	int latitude_;
	int longitude_;
	int height_;

public:
	LocationPoint();
	LocationPoint( const LocationPoint &lp );
	LocationPoint( float latitude, float longitude, int height=INT_MIN );

	bool operator<( const LocationPoint &rhs ) const;
	bool operator==( const LocationPoint &rhs ) const;
	bool operator!=( const LocationPoint &rhs )const
			{
				return !( *this == rhs );
			}
	LocationPoint& operator=( const LocationPoint &rhs ) ;

	/**
	 * Decode a string on the form: longitude latitude
	 *
	 * @throws std::logic_error on failure
	 */
	static
	void decodePoint( const std::string &toDecode, LocationPoint &point );

	/**
	 * Decode a string on the form: longitude latitude, ...,longitude latitude
	 *
	 * @throws std::logic_error on failure
	 */
	static
	void decodePointList( const std::string &toDecode, std::list<LocationPoint> &points );

	/*
	 * Decode a string on the format point( longitude latitude )
	 */
	static
	bool decodeGisPoint( const std::string &toDecode, LocationPoint &point );

	void   set( float latitude, float longitude, int height=INT_MIN );
	void   get( float &latitude, float &longitude, int &height );
	float  latitude() const;
	float  longitude() const;
	int    iLatitude() const;
	int    iLongitude() const;

	bool   hasHeight()const;
	int    height() const;
	void   height( int height );
};


typedef std::list<LocationPoint> LocationPointList;

LocationPointList::iterator
insertLocationPoint( LocationPointList &locations, LocationPoint  &locationPoint, bool replace=false );

}


#endif /* LOCATIONPOINT_H_ */
