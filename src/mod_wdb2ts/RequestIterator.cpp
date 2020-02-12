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
/*
RequestIterator reqit(
	app,
	configData.get(),
	wdbDB, wciProtocol,
	urlQuerys, nearestHeights, locationPoints, webQuery.from(), webQuery.to(),
                       webQuery.isPolygon(), altitude, refTimes, paramDefPtr, providerPriority );
*/

RequestIterator::
RequestIterator( ConfigData *config_,
                 int wciProtocol_,
                 const wdb2ts::config::Config::Query &urlQuerys_,
                 const NearestHeights      &nearestHeights_,
                 const LocationPointListPtr locationPoints_,
                 ParamDefListPtr  paramDefs_,
                 const ProviderList &providerPriority__)
   : config(config_),
     wciProtocol( wciProtocol_ ),
     urlQuerys( urlQuerys_ ),
     nearestHeights( nearestHeights_ ),
     locationPoints( locationPoints_ ),
	 from( config->webQuery.from() ),
     to( config->webQuery.to() ),
     isPolygon(config->webQuery.isPolygon() ),
     altitude( config->altitude ),
     paramDefs( paramDefs_ ),
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

   NearestHeight::processNearestHeightPoint( config, *locationPoints, to, data, altitude,
                                             providerPriority_, *paramDefs, nearestHeights,
                                             wciProtocol);
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

      data = requestManager.requestData( config, paramDefs, providerPriority_, urlQuerys, wciProtocol );

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

