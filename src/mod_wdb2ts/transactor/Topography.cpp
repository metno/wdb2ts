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
#include <transactor/Topography.h>
#include <wciHelper.h>

#ifdef DEBUGSTRM
#define DEBUG_STRM( sout ) {\
   sout; }
#else
#define DEBUG_STRM( sout )
#endif


using namespace std;

namespace wdb2ts {



Topography::
Topography( float latitude, float longitude,
		    const ParamDef &paramDef,
		    const std::string &provider,
		    const boost::posix_time::ptime &reftimespec,
		    bool surround,
		    int wciProtocol )
	: latitude_( latitude ), longitude_( longitude ),
	  paramDef_( paramDef), provider_( provider ),
	  reftimespec_( reftimespec ), surround_( surround ), wciProtocol_( wciProtocol ),
	  polygon( false ),
	  locations_( new LocationPointList() )
{
}

Topography::
Topography(  LocationPointListPtr polygonList,
             int skip,
             const ParamDef &paramDef,
             const std::string &provider,
             const boost::posix_time::ptime &reftimespec,
             int wciProtocol)
   : skip_( skip+1 ), paramDef_( paramDef), provider_( provider ),
     reftimespec_( reftimespec ), surround_( false ), wciProtocol_( wciProtocol ),
     polygon( true ),
     polygonList_( polygonList ),
     locations_( new LocationPointList() )
{
}


Topography::
~Topography()
{
}

/*
 * 	   ost << "SELECT " << returnColoumns << " FROM wci.read(" << endl
		   << "   " << dataprovider.selectPart() << ", " << endl
		   << "   '" << sPointInterpolation << " POINT(" <<  longitude.value() << " " << latitude.value() << ")', " << endl
//       	<< "   'POINT(" <<  longitude.value() << " " << latitude.value() << ")', " << endl
		   << "   " << reftime.selectPart() << ", " << endl
		   << "   " << validtime.selectPart() << ", " << endl
		   << "   " << parameter.selectPart() << ", " << endl
		   << "   " << levelspec.selectPart() << ", " << endl
		   << "   " << dataversion.selectPart() << ", " << endl
		   << "   NULL::wci.returnfloat )" << endl;
 *
 */

void
Topography::
doPolygon( argument_type &t )
{
   ostringstream q;
   string selectPolygon;
   ParamDefList params;
   ParamDefPtr itPar;
   string dummyGroupProvider; //Is not used.
   float value;
   int skip = 0;

#ifdef DEBUGSTRM
   ofstream of("/home/borgem/wdb2ts-doPloygon.txt");
#endif

   locations_->clear();

   if( !polygonList_  || polygonList_->empty() )
      return;

   LocationPoint locationPoint( *polygonList_->begin() );
   if( polygonList_->size() == 1 )
      polygonList_->push_back( locationPoint ); //Must be a closed polygon, this is a point.
   else if( *polygonList_->rbegin() != locationPoint )
      polygonList_->push_back( locationPoint ); //Make it a closed polygon.

   ostringstream ost;
   ost.setf(ios::floatfield, ios::fixed);
   ost.precision(5);

   LocationPointList::const_iterator it = polygonList_->begin();

   ost <<"'POLYGON((";
   ost << it->longitude() << " " << it->latitude();
   for( ++it; it != polygonList_->end(); ++it )
      ost << "," << it->longitude() << " " << it->latitude();
   ost << "))', ";

   selectPolygon = ost.str();

   params[provider_].push_back( paramDef_ );

   WEBFW_USE_LOGGER( "wdb" );
   WEBFW_LOG_DEBUG( "Topography: transactor: polygon: skip: " << skip_ - 1 << "(modulo " << skip_ << ").");

   DEBUG_STRM( of << "polygon: skip: " << skip_ - 1 << "(modulo " << skip_ << ")." << endl );
   q << "SELECT " << wciReadReturnColoumns( wciProtocol_ ) << " FROM wci.read(" << endl
     << "ARRAY['" << provider_ << "'], " << endl
     << selectPolygon << endl
     << wciTimeSpec( wciProtocol_, reftimespec_ ) << ", " << endl
     << "NULL, " << endl
     << "ARRAY['" << paramDef_.valueparametername() << "'], " << endl
     << wciLevelSpec( wciProtocol_, paramDef_ ) << ", " << endl
     << "ARRAY[-1], NULL::wci.returnfloat ) ORDER BY referencetime";

   WEBFW_LOG_DEBUG( "Topography: transactor: polygon SQL ["   << q.str() << "]" );

   DEBUG_STRM( of << "SQL ["   << q.str() << "]" << endl );

   pqxx::result  res = t.exec( q.str() );

   for( pqxx::result::const_iterator it=res.begin(); it != res.end(); ++it ) {
      if( it.at("value").is_null() )
         continue;

      //We are not using every point. It is requested that we skip some points.
      skip = (skip+1) % skip_;

      if( skip ) {
         //WEBFW_LOG_DEBUG( "Topography: transactor: skipping: " << it.at("point").c_str() );
         DEBUG_STRM( of << "Skipping: " << it.at("point").c_str() << endl );
         continue;
      }

      if( params.findParam( it, itPar, dummyGroupProvider ) ) {
         if( !LocationPoint::decodeGisPoint( it.at("point").c_str(), locationPoint ) )
            continue;

         value = it.at("value").as<float>()*itPar->scale()+itPar->offset();
         locationPoint.value(  value );
         if( insertLocationPoint( *locations_, locationPoint ) != locations_->end() ) {
           // WEBFW_LOG_DEBUG( "Topography: transactor: location: POINT("
           //     <<  locationPoint.longitude() << " " << locationPoint.latitude() << " " << locationPoint.height()
           //     << ")" );
            DEBUG_STRM( of << "POINT("
                        <<  locationPoint.longitude() << " " << locationPoint.latitude() << " " << locationPoint.asInt()
                        << ")" << endl );
         }
      }
   }
}

void
Topography::
doLocation( argument_type &t )
{
   ostringstream q;
   string sSurround;

   ParamDefList params;
   ParamDefPtr itPar;
   LocationPoint locationPoint;
   string dummyGProviderGroup; //Is not used.
   float value;

   locations_->clear();

   params[provider_].push_back( paramDef_ );

   if( surround_ )
      sSurround ="surround ";

   WEBFW_USE_LOGGER( "wdb" );

   q << "SELECT " << wciReadReturnColoumns( wciProtocol_ ) << " FROM wci.read(" << endl
         << "ARRAY['" << provider_ << "'], " << endl
         << "'" << sSurround << "POINT(" << longitude_ << " " << latitude_ << ")', " << endl
         << wciTimeSpec( wciProtocol_, reftimespec_ ) << ", " << endl
         << "NULL, " << endl
         << "ARRAY['" << paramDef_.valueparametername() << "'], " << endl
         << wciLevelSpec( wciProtocol_, paramDef_ ) << ", " << endl
         << "ARRAY[-1], NULL::wci.returnfloat ) ORDER BY referencetime";

   WEBFW_LOG_DEBUG( "Topography: transactor: SQL ["   << q.str() << "]" );
   pqxx::result  res = t.exec( q.str() );

   for( pqxx::result::const_iterator it=res.begin(); it != res.end(); ++it ) {
      if( it.at("value").is_null() )
         continue;

      if( params.findParam( it, itPar, dummyGProviderGroup ) ) {
         if( !LocationPoint::decodeGisPoint( it.at("point").c_str(), locationPoint ) )
            continue;

         value = it.at("value").as<float>()*itPar->scale()+itPar->offset();
         locationPoint.value( value );
         if( insertLocationPoint( *locations_, locationPoint ) != locations_->end() ) {
            WEBFW_LOG_DEBUG( "Topography: transactor: location: POINT("
                  <<  locationPoint.longitude() << " " << locationPoint.latitude() << " " << locationPoint.asInt()
                  << ")" );
         }
      }
   }
}


void
Topography::
operator () ( argument_type &t )
{
   if( polygon )
      doPolygon( t );
   else
      doLocation( t );
}


}

