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

#include <iostream>
#include <limits.h>
#include <stdio.h>
#include <LocationPoint.h>
#include <splitstr.h>
#include <compresspace.h>
#include <boost/regex.hpp>
#include <boost/assign/list_of.hpp>
#include <sstream>
#include <Logger4cpp.h>

using namespace boost;
using namespace std;

//If LATLONG_DEG2INT is changed. Then ROUND_BEFORE_DEG2INT must also
//be changed so the rounding shall be correct
#define LATLONG_DEG2INT 10000000
#define ROUND_BEFORE_DEG2INT 0.00000005


namespace {
/// A regular expression for floating points (including exponents)
const string reFloat = "[-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?";

void decodeLocationList( const std::string &toDecode, wdb2ts::LocationPointList &points )
{
   cerr << "decodeLocationList: '" << toDecode << "'" << endl;
   static regex re("^\\s*"+reFloat+"\\s+"+reFloat+"\\s*(,\\s*"+reFloat+"\\s+"+reFloat+"\\s*)*" );

   smatch match;
   if ( !regex_match( toDecode, match, re) ) {
      string msg( "Invalid location list: ");
      msg += toDecode;
      throw logic_error( msg.c_str() );
   }

   float lat, lon;
   string buf;
   string::size_type iPrev=0;
   string::size_type i = toDecode.find( ',', 0 );

   if( i == string::npos )
      buf = toDecode;
   else
      buf = toDecode.substr( 0, i );

   if( sscanf(buf.c_str(), " %f %f", &lon, &lat) != 2 )
      throw logic_error( "Expecting: 'longitude latitude'. latitude and longitude must be valid numbers.");

   points.push_back(wdb2ts::LocationPoint( lat, lon ) );

   while( i != string::npos ) {
      iPrev = i;
      i = toDecode.find( ',', i+1 );

      if( i != string::npos )
         buf = toDecode.substr(iPrev+1, i - iPrev - 1 );
      else
         buf = toDecode.substr( iPrev + 1 );

      if( sscanf(buf.c_str(), " %f %f", &lon, &lat) != 2 )
         throw logic_error( "Expecting: 'longitude latitude'. latitude and longitude must be valid numbers.");

      points.push_back(wdb2ts::LocationPoint( lat, lon ) );
   }
}
}

using namespace std;

namespace wdb2ts {

LocationPoint::
LocationPoint()
: latitude_( INT_MIN ), longitude_( INT_MIN ), value_( FLT_MIN )
{
}

LocationPoint::
LocationPoint( const LocationPoint &lp )
: latitude_( lp.latitude_ ), longitude_( lp.longitude_ ), value_( lp.value_ )
{
}

LocationPoint::
LocationPoint( float latitude, float longitude, float val )
{
   set( latitude, longitude, val );
}

LocationPoint::
~LocationPoint()
{
}

bool
LocationPoint::
operator<( const LocationPoint &rhs ) const
{
   if( (latitude_< rhs.latitude_) ||
         (latitude_ == rhs.latitude_ && longitude_ < rhs.longitude_ ) )
      return true;

   return false;
}

LocationPoint&
LocationPoint::
operator=( const LocationPoint &rhs )
{
   if( this != &rhs ) {
      latitude_ = rhs.latitude_;
      longitude_ = rhs.longitude_;
      value_ = rhs.value_;
   }

   return *this;
}

bool
LocationPoint::
operator==( const LocationPoint &rhs ) const
{
   return latitude_ == rhs.latitude_ && longitude_== rhs.longitude_;
}


void
LocationPoint::
set( float latitude, float longitude, float val )
{
   if( latitude > 90 || latitude < -90 )
      throw range_error( "Latitude out of range. Valid range [-90,90]." );

   if( longitude > 180 || longitude < -180 )
      throw range_error( "Longitude out of range. Valid range [-180,180]." );

   latitude_  = int((latitude+ROUND_BEFORE_DEG2INT)*LATLONG_DEG2INT);
   longitude_ = int((longitude+ROUND_BEFORE_DEG2INT)*LATLONG_DEG2INT);
   value_ = val;
}

void
LocationPoint::
get( float &latitude, float &longitude, float &val )
{
   if( latitude_ == INT_MIN || longitude_ == INT_MIN )
      throw logic_error( "The locationpoint is not initialized.");

   latitude = latitude_/LATLONG_DEG2INT;
   longitude = longitude_/LATLONG_DEG2INT;
   val = value_;
}

float
LocationPoint::
latitude() const
{
   if( latitude_ == INT_MIN )
      throw logic_error( "The locationpoint is not initialized.");

   return float(latitude_)/LATLONG_DEG2INT;
}

float
LocationPoint::
longitude() const
{
   if( longitude_ == INT_MIN )
      throw logic_error( "The locationpoint is not initialized.");

   return float(longitude_)/LATLONG_DEG2INT;
}

int
LocationPoint::
iLatitude() const
{
   if( latitude_ == INT_MIN )
      throw logic_error( "The locationpoint is not initialized.");

   return latitude_;
}

int
LocationPoint::
iLongitude() const
{
   if( longitude_ == INT_MIN )
      throw logic_error( "The locationpoint is not initialized.");

   return longitude_;
}


int
LocationPoint::
asInt() const
{
   if( value_ == FLT_MIN )
      return INT_MIN;

   return static_cast<int>(value_+0.5);
}


bool
LocationPoint::
hasValue()const
{
   return value_ != FLT_MIN;
}

float
LocationPoint::
value() const
{
   return value_;
}

void
LocationPoint::
value( float val )
{
   value_ = val;
}


void
LocationPoint::
decodePoint( const std::string &toDecode_, LocationPoint &point )
{
   LocationPointList points;

   decodeLocationList( toDecode_, points );

   if( points.empty() )
      throw logic_error("Expecting a location point on the format: longitude latitude");

   point = *points.begin();
}

/**
* Decode a string on the form: longitude latitude, ...,longitude latitude
*/
void
LocationPoint::
decodePointList( const std::string &toDecode, LocationPointList &points  )
{
   points.clear();
   decodeLocationList( toDecode, points );

   if( points.empty() )
      throw logic_error("Expecting a location point on the format: longitude latitude, .. , longitude latitude");
}

bool
LocationPoint::
decodeGisPoint( const string &sPoint, LocationPoint &locationPoint )
{
   string::size_type iStart = sPoint.find("(");
   string::size_type iEnd;

   if( iStart == string::npos )
      return false;

   iEnd = sPoint.find(")", iStart );

   if( iEnd == string::npos )
      return false;

   string buf = sPoint.substr(iStart+1, iEnd - iStart -1 );

   if( buf.empty() )
      return false;

   float lon;
   float lat;

   if( sscanf( buf.c_str(), " %f %f ", &lon, &lat )  != 2 )
      return false;

   try {
      locationPoint.set( lat, lon );
   }
   catch( const std::exception &ex ) {
      return false;
   }

   return true;
}

LocationPointList::iterator
insertLocationPoint( LocationPointList &locations, LocationPoint  &locationPoint, bool replace )
{
   LocationPointList::iterator retIt=locations.end();
   LocationPointList::iterator it = locations.begin();
   for( ; it != locations.end() && *it<locationPoint; ++it );

   if( it==locations.end() ) {
      locations.push_back( locationPoint );
      retIt = locations.end();
      --retIt;
   } else if( *it == locationPoint ) {
      if( replace ) {
         *it = locationPoint;
         retIt = it;
      } else {
         retIt = locations.end();
      }
   } else {
      retIt = locations.insert( it, locationPoint );
   }

   return retIt;
}

LocationPointMatrix::
LocationPointMatrix()
{
}

LocationPointMatrix::
LocationPointMatrix( const LocationPointMatrix &cc )
   : LocationPoint2DimMatrix( static_cast<const LocationPoint2DimMatrix&>(cc) )
{
}

LocationPointMatrix::
LocationPointMatrix( int xSize, int ySize )
   : LocationPoint2DimMatrix( boost::extents[xSize][ySize] )
{
}

LocationPointMatrix&
LocationPointMatrix::
operator=( const LocationPointMatrix &rhs )
{
   if( this != &rhs ) {
      if( shape()[0] != rhs.shape()[0] ||
          shape()[1] != rhs.shape()[1] ) {
         resize( boost::extents[rhs.shape()[0]][rhs.shape()[1]] );
      }
   }

   for( LocationPointMatrix::size_type x=0; x < this->shape()[0]; ++x )
      for( LocationPointMatrix::size_type y=0; y < this->shape()[1]; ++y )
         (*this)[x][y] = rhs[x][y];

   return *this;
}



LocationPointMatrixTimeserie::
Value::Value()
   : second( 6, 6 )
{
}

LocationPointMatrixTimeserie::
Value::Value( const boost::posix_time::ptime &first_, const LocationPointMatrix &second_ )
   : first( first_ ), second( second_ )
{
}

LocationPointMatrixTimeserie::
Value::Value( const Value &cc )
   : first( cc.first ), second( cc.second )
{
}

LocationPointMatrixTimeserie::Value&
LocationPointMatrixTimeserie::
Value::operator=( const Value &rhs )
{

   if( this != &rhs ) {
      first = rhs.first;
      second = rhs.second;
   }

   return *this;
}

LocationPointMatrixTimeserie::
LocationPointMatrixTimeserie()
   : logger( "nearest_land")
{

}
LocationPointMatrixTimeserie::
LocationPointMatrixTimeserie( const std::string &logger_ )
   : logger( logger_ )
{

}

LocationPointMatrixTimeserie::
LocationPointMatrixTimeserie( const LocationPointMatrixTimeserie &lp )
   :data( lp.data ), fromTimeIndex( lp.fromTimeIndex ), logger( lp.logger )
{
}

LocationPointMatrixTimeserie::
~LocationPointMatrixTimeserie()
{
}

LocationPointMatrixTimeserie&
LocationPointMatrixTimeserie::
operator=( const LocationPointMatrixTimeserie &rhs )
{
   if( this != &rhs ) {
      logger = rhs.logger;
      data = rhs.data;
      fromTimeIndex = rhs.fromTimeIndex;
   }

   return *this;
}


void
LocationPointMatrixTimeserie::
clear()
{
   data.clear();
   fromTimeIndex.clear();
}

bool
LocationPointMatrixTimeserie::
insert( const boost::posix_time::ptime &validTo, const boost::posix_time::ptime &validFrom,
        const LocationPointMatrix &point, bool replace )
{
   DataType::iterator it=data.find( validTo );

   if( it == data.end() ) {
      data[validTo] = Value( validFrom, point );
      fromTimeIndex[validFrom] = validTo;
      return true;
   }

   if( ! replace )
      return false;

   if( validFrom != it->second.first ) {
      ostringstream ost;
      ost << "Trying to replace an LocationPoint with validTo=" << validTo << " and validFrom=" << validFrom
            << ". The element to be replaced has validTo=" << it->first
            << " and validFrom=" << it->second.first << " this inconcistence is prohibited.";
      throw logic_error( ost.str().c_str() );
   }

   data[validTo] = Value( validFrom, point );

   return true;
}


#if 0
LocationPointMatrixTimeserie::Index::const_iterator
LocationPointMatrixTimeserie::
beginFromTime( const boost::posix_time::ptime &fromTime, bool exact )const
{
   if( invalidFromTimeIndex ) {
      const_cast<LocationPointMatrixTimeserie*>(this)->fromTimeIndex.clear();

      for( Index::const_iterator it=toTimeIndex.begin(); it != toTimeIndex.end(); ++it )
         const_cast<LocationPointMatrixTimeserie*>(this)->fromTimeIndex[it->second.first] =
               IndexValue( it->first, it->second.second );

      const_cast<LocationPointMatrixTimeserie*>(this)->invalidFromTimeIndex = false;
   }

   if( fromTime.is_special() )
      return fromTimeIndex.begin();

   Index::const_iterator it=fromTimeIndex.begin();

   for( ; it != fromTimeIndex.end() && it->first < fromTime; ++it );

   if( it == fromTimeIndex.end() )
      return fromTimeIndex.end();

   if( exact && it->first != fromTime  )
      return fromTimeIndex.end();

   return it;

}

LocationPointMatrixTimeserie::Index::const_iterator
LocationPointMatrixTimeserie::
endFromTime()const
{
   return fromTimeIndex.end();
}
#endif


LocationPointMatrixTimeserie::DataType::const_iterator
LocationPointMatrixTimeserie::
beginToTime( const boost::posix_time::ptime &fromTime, bool exact )const
{
   if( fromTime.is_special() )
      return data.begin();

   DataType::const_iterator it=data.begin();

   for( ; it != data.end() && it->first < fromTime; ++it );

   if( it == data.end() )
      return data.end();

   if( exact && it->first != fromTime  )
      return data.end();

   return it;

}

LocationPointMatrixTimeserie::DataType::const_iterator
LocationPointMatrixTimeserie::
endToTime()const
{
   return data.end();
}


int
LocationPointMatrixTimeserie::
valuesGreaterThan( const LocationPointMatrix &values,
                   int suroundLevel, float checkValue, XYPoints &points )
{
   //WEBFW_USE_LOGGER( logger );
   int nX = values.shape()[0];
   int nY = values.shape()[1];
   int level;
   int N = 2*suroundLevel;

   points.clear();

   if( (nX % 2) != 0  || nX != nY) {
      ostringstream ost;
      ost << "LocationPointMatrixTimeserie::valuesGreaterThan: The matrix must be a multiple of 2. The size is "
            << nX << "x" << nY << ". Expecting at least " << N << "x" << N;
      throw range_error( ost.str().c_str() );
   }

   if( N > nX ) {
      ostringstream ost;
      ost << "LocationPointMatrixTimeserie::valuesGreaterThan: The size is "
            << nX << "x" << nY << ". Expecting at least " << N << "x" << N;
      throw range_error( ost.str().c_str() );
   }


   level = nX/2;

//   WEBFW_LOG_DEBUG( "nX: " << nX << " nY: " << nY << " N: " << N << " sl: " << suroundLevel
//                    <<   " level: " << level );
   int l = level - suroundLevel;
   int r=nX-l;
   int x=0;
   int y=0;
   float val;
   int nGreater=0;

   for( int yy=l; yy<r; ++yy,++y ) {
      for( int xx=l; xx<r; ++xx,++x ) {
         val = values[yy][xx].value();

         if( val != FLT_MIN && val > checkValue ) {
            ++nGreater;
            points.push_back( XY( x, y ) );
         }
      }
   }

   return nGreater;
}

}
