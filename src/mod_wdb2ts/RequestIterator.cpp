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

#include "RequestIterator.h"
#include "WdbDataRequest.h"


namespace wdb2ts {

RequestIterator::
RequestIterator( Wdb2TsApp *app__,
                 const std::string &wdbDB_,
                 int wciProtocol_,
                 const wdb2ts::config::Config::Query &urlQuerys_,
                 const NearestHeights      &nearestHeights_,
                 const LocationPointListPtr locationPoints_,
                 const boost::posix_time::ptime &to_,
                 bool isPolygon_, int altitude_,
                 PtrProviderRefTimes refTimes__,
                 const ProviderList &providerPriority__)
   : app_( app__ ),
     wdbDB( wdbDB_ ),
     wciProtocol( wciProtocol_ ),
     urlQuerys( urlQuerys_ ),
     nearestHeights( nearestHeights_ ),
     locationPoints( locationPoints_ ),
     to( to_ ),
     isPolygon( isPolygon_ ),
     altitude( altitude_ ),
     refTimes_( refTimes__ ),
     providerPriority_( providerPriority__ )
{
   itPoint = locationPoints->begin();
}

void
RequestIterator::
nearestHeightPoint( LocationPointDataPtr data )
{

   if( nearestHeights.empty() || locationPoints->empty() )
      return;

   ParamDefList params = app_->paramDefs();
   WciConnectionPtr wciConnection = app_->newWciConnection( wdbDB );

   NearestHeight::processNearestHeightPoint( *locationPoints, to, data, altitude, refTimes_,
                                             providerPriority_, params, nearestHeights,
                                             wciProtocol, wciConnection );

}



/**
* @throws std::logic_error on failure.
*/
LocationPointDataPtr
RequestIterator::
next()
{
   LocationPointDataPtr data;

   if( itPoint == locationPoints->end() )
      return LocationPointDataPtr();

   try {
      WdbDataRequestManager requestManager;
      LocationPointList pointList;

      if( ! isPolygon ) {
         pointList = *locationPoints;
         itPoint = locationPoints->end();
      } else {
         pointList.push_back( *itPoint );
         ++itPoint;
      }

      data = requestManager.requestData( app_, wdbDB, pointList, to, false, altitude,
                                         refTimes_, providerPriority_, urlQuerys, wciProtocol );

      if( !isPolygon ) {
         nearestHeightPoint( data );
      }

      return data;
   }
   catch( const std::exception &ex ) {
      throw std::logic_error( ex.what() );
   }
   catch( ... ) {
      throw std::logic_error( "RequestIterator::next: Unknown exception." );
   }

}


}

