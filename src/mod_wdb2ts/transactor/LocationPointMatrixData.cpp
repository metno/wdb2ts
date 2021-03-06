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

#include <fstream>
#include <sstream>
#include <ptimeutil.h>
#include <Logger4cpp.h>
#include <ProviderList.h>
#include <transactor/LocationPointMatrixData.h>
#include <wciHelper.h>

#ifdef DEBUGSTRM
#define DEBUG_STRM( sout ) {\
   sout; }
#else
#define DEBUG_STRM( sout )
#endif


using namespace std;

namespace wdb2ts {

LocationPointMatrixData::
LocationPointMatrixData()
   : dataversion_( -1 ),
     query_( new std::string() ), locations_( new  LocationPointMatrixTimeserie() ),
     logger_( "wdb" )
{
}

LocationPointMatrixData::
LocationPointMatrixData( const LocationPointMatrixData &lpmd )
 : latitude_( lpmd.latitude_ ), longitude_( lpmd.longitude_ ),
   params_( lpmd.params_ ), paramDef_( lpmd.paramDef_ ),
   provider_( lpmd.provider_ ), reftimespec_( lpmd.reftimespec_ ),
   dataversion_( lpmd.dataversion_ ),
   surroundLevel_( lpmd.surroundLevel_ ), wciProtocol_( lpmd.wciProtocol_ ),
   query_( lpmd.query_ ), locations_( lpmd.locations_ ),
   logger_( lpmd.logger_ )
{
}


LocationPointMatrixData::
LocationPointMatrixData( float latitude, float longitude,
		    const ParamDefList &params,
		    const ParamDef &paramDef,
		    const std::string &provider,
		    const boost::posix_time::ptime &reftimespec,
	       int dataversion,
		    int surroundLevel,
		    int wciProtocol,
		    const std::string &logger)
	: latitude_( latitude ), longitude_( longitude ),
	  params_( params ), paramDef_( paramDef ),
	  reftimespec_( reftimespec ), dataversion_( dataversion ),
	  surroundLevel_( surroundLevel ), wciProtocol_( wciProtocol ),
	  query_( new std::string() ),
	  locations_( new LocationPointMatrixTimeserie() ),
	  logger_( logger )
{
   ProviderItem pi = ProviderList::decodeItem( provider );
   provider_ = pi.provider;
}


LocationPointMatrixData::
~LocationPointMatrixData()
{
}


LocationPointMatrixData&
LocationPointMatrixData::
operator=( const LocationPointMatrixData &rhs )
{
   if( this != &rhs ) {
      latitude_ = rhs.latitude_;
      longitude_ = rhs.longitude_;
      params_ = rhs.params_;
      paramDef_ = rhs.paramDef_;
      provider_ = rhs.provider_;
      reftimespec_ = rhs.reftimespec_;
      dataversion_ = rhs.dataversion_;
      surroundLevel_ = rhs.surroundLevel_;
      wciProtocol_ = rhs.wciProtocol_;
      query_ = rhs.query_ ;
      locations_ = rhs.locations_;
      logger_ = rhs.logger_;
   }
   return *this;
}

void
LocationPointMatrixData::
operator () ( argument_type &t )
{
   ostringstream q;
   string sSurround;

   boost::posix_time::ptime validtimeto;
   boost::posix_time::ptime validtimefrom;
   boost::posix_time::ptime prevValidTimeTo;
   boost::posix_time::ptime prevValidTimeFrom;
   ParamDefPtr itPar;
   LocationPoint locationPoint;
   string dummyGroupProvider;
   int expextNValues=1;
   int nValues=0;
   int n=1;
   float value;
   int x=0;
   int y=0;
   int myDataversion( dataversion_ );

   WEBFW_USE_LOGGER( logger_ );

   locations_->clear();
   query_->clear();

   if( surroundLevel_ < 0 ) {
      WEBFW_LOG_WARN("The SUROUND level is less than 0, setting it to 0.");
      surroundLevel_ = 0;
   }

   if( wciProtocol_ < 6) {
      sSurround ="SURROUND ";
      expextNValues = 4;
      n=2;
   } else {
      if( surroundLevel_ > 0 ) {
         n = 2*surroundLevel_;
         expextNValues = 4*surroundLevel_*surroundLevel_; // (2*surroundLevel_)^2
         q << "SURROUND(" << surroundLevel_ << ") ";
         sSurround = q.str();
        // n=surroundLevel_;
      }
   }

   LocationPointMatrix pointMatrix( n, n );
   LocationPointMatrix undefPointMatrix( n, n );

   for( int x=0; x<n; ++x)
      for( int y=0; y<n; ++y )
         undefPointMatrix[x][y] = LocationPoint();

   if( myDataversion < -1 )
      myDataversion = -1;

   q.str("");
   q << "SELECT " << wciReadReturnColoumns( wciProtocol_ ) << " FROM wci.read(" << endl
     << "ARRAY['" << provider_ << "'], " << endl
     << "'" << sSurround << "POINT(" << longitude_ << " " << latitude_ << ")', " << endl
     << wciTimeSpec( wciProtocol_, reftimespec_ ) << ", " << endl
     << "NULL, " << endl
     << "ARRAY['" << paramDef_.valueparametername() << "'], " << endl
     << wciLevelSpec( wciProtocol_, paramDef_ ) << ", " << endl
     << "ARRAY[" << myDataversion << "], NULL::wci.returnfloat ) ORDER BY referencetime, validtimeto, validtimefrom";

   WEBFW_LOG_DEBUG( "LocationPointMatrixData: transactor: SQL ["   << q.str() << "]" );
   *query_ = q.str();
   pqxx::result  res = t.exec( q.str() );

   for( pqxx::result::const_iterator it=res.begin(); it != res.end(); ++it ) {
      if( it.at("value").is_null() )
         continue;

      if( params_.findParam( it, itPar, dummyGroupProvider ) ) {
         if( !LocationPoint::decodeGisPoint( it.at("point").c_str(), locationPoint ) )
            continue;

         validtimefrom = miutil::ptimeFromIsoString( it.at("validtimefrom").c_str() );
         validtimeto = miutil::ptimeFromIsoString( it.at("validtimeto").c_str() );

         if( prevValidTimeTo.is_special() ) {
            prevValidTimeTo = validtimeto;
            prevValidTimeFrom = validtimefrom;
         } else if( prevValidTimeTo != validtimeto ) {
            if( nValues != expextNValues ) {
               WEBFW_LOG_DEBUG("LocationPointMatrixData: expected: " << expextNValues << ", but got " << nValues << ".");
            }

            if( WEBFW_GET_LOGLEVEL() == log4cpp::Priority::DEBUG ) {
               ostringstream ost;
               ost << "LocationPointMatrixData: " << prevValidTimeFrom << " - " << prevValidTimeTo;
                for( int x=0; x<n; ++x)
                   for( int y=0; y<n; ++y )
                      ost << "\n                        [" << x << ", " << y <<"] = " << pointMatrix[x][y].value();

                WEBFW_LOG_DEBUG( ost.str() );
            }

            locations_->insert( prevValidTimeFrom, prevValidTimeTo, pointMatrix, false );
            pointMatrix = undefPointMatrix;
            nValues = 0;
            x=0;
            y=0;
            prevValidTimeTo = validtimeto;
            prevValidTimeFrom = validtimefrom;
         }

         if( x >= n && y < n ) {
            x = 0;
            y++;
         }

         if( x < n && y < n ) {
            value = it.at("value").as<float>()*itPar->scale()+itPar->offset();
            locationPoint.value( value );
            //WEBFW_LOG_DEBUG("LocationPointMatrixData: pointMatrix[" << y << "][" << x <<"] = " << locationPoint.value() );
            pointMatrix[y][x] = locationPoint;
            nValues++;
            x++;
         }
      }
   }

   if( pointMatrix != undefPointMatrix ) {
      if( WEBFW_GET_LOGLEVEL() == log4cpp::Priority::DEBUG ) {
         ostringstream ost;
         ost << "LocationPointMatrixData: " << prevValidTimeFrom << " - " << prevValidTimeTo;
         for( int x=0; x<n; ++x)
            for( int y=0; y<n; ++y )
               ost << "\n                        [" << x << ", " << y <<"] = " << pointMatrix[x][y].value();

         WEBFW_LOG_DEBUG( ost.str() );
      }

      locations_->insert( prevValidTimeFrom, prevValidTimeTo, pointMatrix, false );
   }

}


}

