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

#include <boost/utility.hpp>

#include <limits.h>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <LocationPoint.h>
#include <UpdateProviderReftimes.h>
#include <ProviderList.h>
#include <exception.h>
#include <wdb2TsApp.h>
#include <NearestHeight.h>
#include <ParamDef.h>


#ifndef __REQUESTITERATOR_H__
#define __REQUESTITERATOR_H__


namespace wdb2ts {
   class RequestIterator : boost::noncopyable
   {
      Wdb2TsApp *app_;
      std::string wdbDB;
      int wciProtocol;
      wdb2ts::config::Config::Query urlQuerys;
      NearestHeights      nearestHeights;
      LocationPointListPtr locationPoints;
      boost::posix_time::ptime from;
      boost::posix_time::ptime to;
      bool isPolygon;
      int altitude;
      const PtrProviderRefTimes refTimes_;
      ParamDefListPtr  paramDefs;
      ProviderList providerPriority_;
      LocationPointList::const_iterator itPoint;

      void nearestHeightPoint( LocationPointDataPtr data );

   public:
      RequestIterator(): app_(0){}
      RequestIterator( Wdb2TsApp *app,
                       const std::string &wdbDB,
                       int wciProtocol,
                       const wdb2ts::config::Config::Query &urlQuerys,
                       const NearestHeights      &nearestHeights,
                       const LocationPointListPtr locationPoints,
					   const boost::posix_time::ptime &from,
                       const boost::posix_time::ptime &to,
                       bool isPolygon, int altitude,
                       const PtrProviderRefTimes refTimes,
                       ParamDefListPtr  paramDefs,
                       const ProviderList &providerPriority );


      Wdb2TsApp *app() { return app_; }
      bool polygon()const { return isPolygon; }

      float longitude()const{ return locationPoints->begin()->longitude(); }
      float latitude()const{ return locationPoints->begin()->latitude(); }
      int   height()const { return altitude; }

      ProviderList providerPriority() const { return providerPriority_; }
      const PtrProviderRefTimes refTimes() const { return refTimes_; }
      /**
       * @throws std::logic_error on failure.
       */
      LocationPointDataPtr next();
   };


}

#endif
