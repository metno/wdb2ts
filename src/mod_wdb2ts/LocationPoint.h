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

#include <float.h>
#include <stdexcept>
#include <list>
#include <map>
#include <vector>
#include <limits.h>
#include <boost/shared_ptr.hpp>
#include <boost/multi_array.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>


namespace wdb2ts {

class LocationPoint;



/**
 *  LocationPoint hold a point as two integer. The latitude and longitude
 *  is given in decimal grades.
 *
 *  The coding into integer is done as int((dg+0.00005)*10000), ie a resolution on 5 desimals.
 *  The decoding back to desimal grades is: i/10000.
 */
class LocationPoint {
	int latitude_;
	int longitude_;
	float value_;

public:
	LocationPoint();
	LocationPoint( const LocationPoint &lp );
	LocationPoint( float latitude, float longitude, float value=FLT_MIN );
	~LocationPoint();

	bool operator<( const LocationPoint &rhs ) const;
	bool operator==( const LocationPoint &rhs ) const;
	bool operator!=( const LocationPoint &rhs )const
			{
				return !( *this == rhs );
			}
	//LocationPoint& operator=( const LocationPoint &rhs );
	LocationPoint& operator=( const LocationPoint &rhs );

	bool isSet()const { return latitude_!= INT_MIN && longitude_!= INT_MIN; }

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

	void   set( float latitude, float longitude, float val=FLT_MIN );
	void   get( float &latitude, float &longitude, float &val );
	float  latitude() const;
	float  longitude() const;
	int    iLatitude() const;
	int    iLongitude() const;

	/**
	 * Return the vale rounded to the nearest int.
	 * @return INT_MIN if undefined.
	 */
	int    asInt() const;


   /**
    *
    * @return FLT_MIN if undefined.
    */
	float  value()const;
	void   value( float value );
	bool   hasValue()const;

};


typedef std::list<LocationPoint> LocationPointList;
typedef boost::shared_ptr<LocationPointList> LocationPointListPtr;

typedef boost::multi_array<LocationPoint, 2> LocationPoint2DimMatrix;

class LocationPointMatrix : public LocationPoint2DimMatrix
{
public:
   LocationPointMatrix();
   LocationPointMatrix( const LocationPointMatrix &cc );

   LocationPointMatrix( int xSize, int ySize );

   LocationPointMatrix& operator=( const LocationPointMatrix &rhs );
};

//typedef boost::multi_array<LocationPoint, 2> LocationPointMatrix;
typedef boost::shared_ptr<LocationPointMatrix> LocationPointMatrixPtr;

class LocationPointMatrixTimeserie
{
public:

   //typedef std::pair<boost::posix_time::ptime, LocationPointMatrix> Value;

   struct Value {
      boost::posix_time::ptime first;
      LocationPointMatrix second;

      Value();
      Value( const boost::posix_time::ptime &first_, const LocationPointMatrix &second_ );

      Value( const Value &cc );
      Value& operator=( const Value &rhs );
   };

   typedef std::map<boost::posix_time::ptime, boost::posix_time::ptime > FromIndex;
   typedef std::map<boost::posix_time::ptime, Value > DataType;

   struct XY {
      int x,y;
      XY():x(INT_MIN), y(INT_MIN ){}
      XY( int x_, int y_) : x(x_), y(y_) {}
      XY( const XY &xy ): x(xy.x), y(xy.y){}
      XY& operator=( const XY &rhs )
      {
         if( this != &rhs ) {
            x = rhs.x;
            y = rhs.y;
         }

         return *this;
      }
   };

   typedef std::vector<XY> XYPoints;


protected:
   DataType data;
   FromIndex fromTimeIndex;
   std::string logger;

public:
   LocationPointMatrixTimeserie( const LocationPointMatrixTimeserie &lp );
   LocationPointMatrixTimeserie();
   LocationPointMatrixTimeserie( const std::string &logger );
   ~LocationPointMatrixTimeserie();

   LocationPointMatrixTimeserie& operator=( const LocationPointMatrixTimeserie &rhs );
   void clear();
   int size()const { return data.size();};

   /**
    *
    * @param validTo
    * @param validFrom
    * @param point The point to insert.
    * @param replace Should the point be replaced if a value for this point allready exist in the datalist.
    * @return true if the point was inserted and false other wise.
    * @exception std::logic_error if an inconsistency is found. Ie. replace is true and an element with
    *   toTime equal validTo and fromTime != validFrom.
    */
   bool insert( const boost::posix_time::ptime &validTo, const boost::posix_time::ptime &validFrom,
                const LocationPointMatrix &point, bool replace=false);
#if 0

   /**
    *
    * @param fromTime Start at this from time. If fromTime has the value is_spesial the start at the first
    *    data element.
    * @param exact if true. The time must be equal to fromTime. If false, return the first element equal or
    *   greater than fromTime.
    * @return An iterator pointing to the first element if found. An iteartor eqal to endFromTime if not found.
    */
   FromIndex::const_iterator beginFromTime( const boost::posix_time::ptime &fromTime=boost::posix_time::ptime(),
                                        bool exact=false )const;
   FromIndex::const_iterator endFromTime()const;
#endif

   /**
    *
    * @param fromTime Start at this from time. If fromTime has the value is_spesial the start at the first
    *    data element.
    * @param exact if true. The time must be equal to fromTime. If false, return the first element equal or
    *   greater than fromTime.
    * @return An iterator pointing to the first element if found. An iteartor eqal to endFromTime if not found.
    */
   DataType::const_iterator beginToTime( const boost::posix_time::ptime &fromTime=boost::posix_time::ptime(),
                                      bool exact=false)const;
   DataType::const_iterator endToTime()const;


   /**
    * This function creates an view into values so that it check the values only in the given suroundLevel.
    * With the use of this function we can request more values than we need and loop trough all interested
    * suroundLevels we want to check until we find what we are looking after.
    *
    * We use it so search from the innermost suroundLevel to the outer and count values greater than a limit,
    * checkValue.
    *
    * If the given suroundLevel is greater than what is in the value matrix a range_error exception
    * is thrown.
    *
    * @param values The matrix to check against.
    * @param suroundLevel The suroundLevel to check.
    * @param checkValue Count all values greater than this value.
    * @param points is an vector with the x and y position in the values matrix
    *   with an value greater than checkValue.
    * @return Number of values greater than checkValue.
    * @exception std::range_error if suroundLevel is greater than what is avalable in values.
    */
   static int valuesGreaterThan( const LocationPointMatrix &values,
                                 int suroundLevel, float checkValue, XYPoints &points );
};

typedef boost::shared_ptr<LocationPointMatrixTimeserie> LocationPointMatrixTimeseriePtr;


LocationPointList::iterator
insertLocationPoint( LocationPointList &locations, LocationPoint  &locationPoint, bool replace=false );

}


#endif /* LOCATIONPOINT_H_ */
