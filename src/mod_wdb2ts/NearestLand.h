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

#ifndef __NEARESTLAND_H__
#define __NEARESTLAND_H__


#include <string>
#include <map>
#include <list>
#include <ostream>

#include <configparser/RequestConf.h>
#include <DbManager.h>
#include <LocationData.h>
#include <UpdateProviderReftimes.h>
#include <ParamDef.h>

namespace wdb2ts {

class NearestLandConfElement
{
   std::map< std::string, std::string> paramWithProvider;
   std::string landmaskProvider_;
   std::string modelTopoProvider_;
   std::string renameTo_;
   std::string provider_;

   friend class NearestLand;

public:
   NearestLandConfElement(){};

   std::map< std::string, std::string> params()const { return paramWithProvider; }
   bool rename() const { return ! renameTo_.empty(); }
   std::string renameTo() const { return renameTo_;}
   std::string landmaskProvider() const { return landmaskProvider_; }
   std::string modelTopoProvider()const { return modelTopoProvider_; }
   std::string provider()const { return provider_; }
};

typedef std::map< std::string, NearestLandConfElement > NearestLandConf;

class NearestLand
{

   const LocationPointList &locationPoints;
   const boost::posix_time::ptime &to;
   LocationPointDataPtr data;
   int altitude;
   PtrProviderRefTimes refTimes;
   const ProviderList &providerPriority;
   const ParamDefList &params;
   const NearestLandConf &nearestLands;
   int wciProtocol;
   WciConnectionPtr wciConnection;

   static void decode( const wdb2ts::config::ActionParam &conf,
                       const std::string &prefix,
                       wdb2ts::NearestLandConf &nearestLand,
                       std::ostream &msg );

   void computeNearestLand( const NearestLandConfElement &conf,
                            const LocationPointMatrix &modeltopo,
                            const LocationPointMatrixTimeserie::XYPoints &xyPoints,
                            int suroundLevel );

   void setData( const ParamDef &param,
                 const std::string &paramname,
                 const std::string &provider,
                 const LocationPointMatrix &modeltopo,
                 const LocationPointMatrixTimeserie::XYPoints &xyPoints,
                 int suroundLevel,
                 LocationPointMatrixTimeserie &data
               );

   /**
    * The temperature is adjusted, based on the time on the day, after the
    * following algorithm.
    *
    * We have two temperatures for a given point. The temperatures comes from
    * different algorithms. We select one of the temperatures or a linear
    * combination of them based on the time of the day. The time in the
    * following table is geographical local time.
    *
    * The input to the method is the time in utc, and two temperatures t1 and t2.
    *
    * At 12-18 return t1.
    * At 18-00 Select a linear combination of t1 and t2
    * At 00-06 return t2.
    * At 06-12 Select a linear combination of t2 and t1
    *
    * @param time in utc.
    * @param The longitude for the location at the requested point.
    *  This is needed to calculate the geographical local time.
    * @param t1 Temperature from algorithm 1.
    * @param t2 Temperature from algorithm 2.
    * @return The new temperature.
    */
   float correctValue( const boost::posix_time::ptime &time,
                       float longitude,
                       float t1, float t2 );

   float linearInterpolateValue( const boost::posix_time::time_duration &clock,
                                 const boost::posix_time::time_duration &c1,
                                 const boost::posix_time::time_duration &c2,
                                 float t1, float t2 );
public:
   NearestLand(const LocationPointList &locationPoints,
               const boost::posix_time::ptime &to,
               LocationPointDataPtr data,
               int altitude,
               PtrProviderRefTimes refTimes,
               const ProviderList &providerPriority,
               const ParamDefList &params,
               const NearestLandConf &nearestLands,
               int wciProtocol,
               WciConnectionPtr wciConnection);

   static NearestLandConf configureNearestLand( const wdb2ts::config::ActionParam &conf );
   void processNearestLandPoint();

};

}

#endif
