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


#ifndef __WDB2TS_POINTDATAHELPER_H__
#define __WDB2TS_POINTDATAHELPER_H__

#include <float.h>
#include <string>
#include <string>
#include <pqxx/pqxx>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <ParamDef.h>
#include <ProviderList.h>
#include <UpdateProviderReftimes.h>

namespace wdb2ts {

struct PData{
	float windV10m;
   float windU10m;
   float PP;
   float PR;
   float TA;
   float T2M;
   float T2M_LAND;
   float UU;
   float PRECIP;
   float PRECIP_ACCUMULATED;
   float PRECIP_1T;
   float PRECIP_3T;
   float PRECIP_6T;
   float PRECIP_12T;
   float PRECIP_24T;
   float seaCurrentVelocityU;
   float seaCurrentVelocityV;
   float seaSalinity;
   float seaSurfaceHeight;
   float seaTemperature;
   float meanTotalWaveDirection;
   float significantTotalWaveHeight;
   float seaIcePresence;
   float seaBottomTopography;
   float NN;
   float fog;
   float highCloud;
   float mediumCloud;
   float lowCloud;
   float RH2M;
   float thunderProability;
   float fogProability;
   float WIND_PROBABILITY;
   float T2M_PROBABILITY_1;
   float T2M_PROBABILITY_2;
   float T2M_PROBABILITY_3;
   float T2M_PROBABILITY_4;
   float T2M_PERCENTILE_10;
   float T2M_PERCENTILE_25;
   float T2M_PERCENTILE_50;
   float T2M_PERCENTILE_75;
   float T2M_PERCENTILE_90;
   float PRECIP_PERCENTILE_10;
   float PRECIP_PERCENTILE_25;
   float PRECIP_PERCENTILE_50;
   float PRECIP_PERCENTILE_75;
   float PRECIP_PERCENTILE_90;
   float PRECIP_PROBABILITY_0_1MM;
   float PRECIP_PROBABILITY_0_2MM;
   float PRECIP_PROBABILITY_0_5MM;
   float PRECIP_PROBABILITY_1_0MM;
   float PRECIP_PROBABILITY_2_0MM;
   float PRECIP_PROBABILITY_5_0MM;
   float symbol;
   float symbol_PROBABILITY;
   float modeltopography;
   
   PData()
   	:windV10m( FLT_MAX ), windU10m( FLT_MAX ), PP( FLT_MAX ), PR( FLT_MAX ), 
   	 TA( FLT_MAX ), T2M( FLT_MAX ), T2M_LAND( FLT_MAX ), UU( FLT_MAX ), 
   	 PRECIP( FLT_MAX ), PRECIP_ACCUMULATED( FLT_MAX ), PRECIP_1T( FLT_MAX ), 
   	 PRECIP_3T( FLT_MAX ), PRECIP_6T( FLT_MAX ), PRECIP_12T( FLT_MAX ), 
   	 PRECIP_24T( FLT_MAX ), seaCurrentVelocityU( FLT_MAX ), 
   	 seaCurrentVelocityV( FLT_MAX ), seaSalinity( FLT_MAX ),
   	 seaSurfaceHeight( FLT_MAX ), seaTemperature( FLT_MAX ),
   	 meanTotalWaveDirection( FLT_MAX ), significantTotalWaveHeight( FLT_MAX ),
   	 seaIcePresence( FLT_MAX), 
   	 seaBottomTopography( FLT_MAX ),
   	 NN( FLT_MAX ), fog( FLT_MAX ), 
   	 highCloud( FLT_MAX ), mediumCloud( FLT_MAX ), lowCloud( FLT_MAX ),
   	 RH2M( FLT_MAX ), thunderProability( FLT_MAX ), fogProability( FLT_MAX ),
   	 WIND_PROBABILITY( FLT_MAX ),
   	 T2M_PROBABILITY_1( FLT_MAX ),
   	 T2M_PROBABILITY_2( FLT_MAX ),
   	 T2M_PROBABILITY_3( FLT_MAX ),
   	 T2M_PROBABILITY_4( FLT_MAX ),
   	 T2M_PERCENTILE_10( FLT_MAX ),
   	 T2M_PERCENTILE_25( FLT_MAX ),
   	 T2M_PERCENTILE_50( FLT_MAX ),
   	 T2M_PERCENTILE_75( FLT_MAX ),
   	 T2M_PERCENTILE_90( FLT_MAX ),
   	 PRECIP_PERCENTILE_10( FLT_MAX ),
   	 PRECIP_PERCENTILE_25( FLT_MAX ),
   	 PRECIP_PERCENTILE_50( FLT_MAX ),
   	 PRECIP_PERCENTILE_75( FLT_MAX ),
   	 PRECIP_PERCENTILE_90( FLT_MAX ),
   	 PRECIP_PROBABILITY_0_1MM( FLT_MAX ),
   	 PRECIP_PROBABILITY_0_2MM( FLT_MAX ),
   	 PRECIP_PROBABILITY_0_5MM( FLT_MAX ),
   	 PRECIP_PROBABILITY_1_0MM( FLT_MAX ),
   	 PRECIP_PROBABILITY_2_0MM( FLT_MAX ),
   	 PRECIP_PROBABILITY_5_0MM( FLT_MAX ),
   	 symbol_PROBABILITY( FLT_MAX ),
   	 symbol( FLT_MAX ), 
   	 modeltopography( FLT_MAX ){}
   
   PData(const PData &pd)
      :windV10m(pd.windV10m), windU10m(pd.windU10m), 
       PP(pd.PP), PR(pd.PR), TA(pd.TA), T2M(pd.T2M), 
       T2M_LAND(pd.T2M_LAND), UU(pd.UU), PRECIP( pd.PRECIP),
       PRECIP_ACCUMULATED( pd.PRECIP_ACCUMULATED ),
       PRECIP_1T(pd.PRECIP_1T), PRECIP_3T(pd.PRECIP_3T), 
       PRECIP_6T(pd.PRECIP_6T), PRECIP_12T(pd.PRECIP_12T), 
       PRECIP_24T(pd.PRECIP_24T), seaCurrentVelocityV( pd.seaCurrentVelocityV ), 
       seaCurrentVelocityU( pd.seaCurrentVelocityU ), seaSalinity( pd.seaSalinity ),
       seaSurfaceHeight( pd.seaSurfaceHeight ), seaTemperature( pd.seaTemperature ),
       meanTotalWaveDirection( pd.meanTotalWaveDirection ), significantTotalWaveHeight( pd.significantTotalWaveHeight ),
       seaIcePresence( pd.seaIcePresence ), 
       seaBottomTopography( pd.seaBottomTopography ),
       NN(pd.NN), fog(pd.fog),
       highCloud(pd.highCloud), mediumCloud(pd.mediumCloud),
       lowCloud(pd.lowCloud),RH2M(pd.RH2M), 
       thunderProability( pd.thunderProability ), fogProability( pd.fogProability ),
       WIND_PROBABILITY( pd.WIND_PROBABILITY ),
       T2M_PROBABILITY_1( pd.T2M_PROBABILITY_1 ),
       T2M_PROBABILITY_2( pd.T2M_PROBABILITY_2 ),
       T2M_PROBABILITY_3( pd.T2M_PROBABILITY_3 ),
       T2M_PROBABILITY_4( pd.T2M_PROBABILITY_4 ),
       T2M_PERCENTILE_10( pd.T2M_PERCENTILE_10 ),
       T2M_PERCENTILE_25( pd.T2M_PERCENTILE_25 ),
       T2M_PERCENTILE_50( pd.T2M_PERCENTILE_50 ),
       T2M_PERCENTILE_75( pd.T2M_PERCENTILE_75 ),
       T2M_PERCENTILE_90( pd.T2M_PERCENTILE_90 ),
       PRECIP_PERCENTILE_10( pd.PRECIP_PERCENTILE_10 ),
       PRECIP_PERCENTILE_25( pd.PRECIP_PERCENTILE_25 ),
       PRECIP_PERCENTILE_50( pd.PRECIP_PERCENTILE_50 ),
       PRECIP_PERCENTILE_75( pd.PRECIP_PERCENTILE_75 ),
       PRECIP_PERCENTILE_90( pd.PRECIP_PERCENTILE_90 ),
       PRECIP_PROBABILITY_0_1MM( pd.PRECIP_PROBABILITY_0_1MM ),
       PRECIP_PROBABILITY_0_2MM( pd.PRECIP_PROBABILITY_0_2MM ),
       PRECIP_PROBABILITY_0_5MM( pd.PRECIP_PROBABILITY_0_5MM ),
       PRECIP_PROBABILITY_1_0MM( pd.PRECIP_PROBABILITY_1_0MM ),
       PRECIP_PROBABILITY_2_0MM( pd.PRECIP_PROBABILITY_2_0MM ),
       PRECIP_PROBABILITY_5_0MM( pd.PRECIP_PROBABILITY_5_0MM ),
       symbol_PROBABILITY( pd.symbol_PROBABILITY ),
       symbol( pd.symbol ), 
       modeltopography( pd.modeltopography ){}
   
   PData& operator=(const PData &rhs){
      if(this!=&rhs){
         windV10m = rhs.windV10m; 
         windU10m = rhs.windU10m;
         PP       = rhs.PP;
         PR       = rhs.PR;
         TA       = rhs.TA;
         T2M      = rhs.T2M;
         T2M_LAND = rhs.T2M_LAND;
         UU       = rhs.UU;
         PRECIP     = rhs.PRECIP;
         PRECIP_ACCUMULATED = rhs.PRECIP_ACCUMULATED;
         PRECIP_1T  = rhs.PRECIP_1T;
         PRECIP_3T  = rhs.PRECIP_3T;
         PRECIP_6T  = rhs.PRECIP_6T;
         PRECIP_12T = rhs.PRECIP_12T;
         PRECIP_24T = rhs.PRECIP_24T;
         seaCurrentVelocityV = rhs.seaCurrentVelocityV; 
         seaCurrentVelocityU = rhs.seaCurrentVelocityU;
         seaSalinity         = rhs.seaSalinity;
         seaSurfaceHeight    = rhs.seaSurfaceHeight;
         seaTemperature      = rhs.seaTemperature;
         meanTotalWaveDirection       = rhs.meanTotalWaveDirection;
         significantTotalWaveHeight = rhs.significantTotalWaveHeight;
         seaIcePresence = rhs.seaIcePresence;
         seaBottomTopography = rhs.seaBottomTopography;
         NN       = rhs.NN;
         fog      = rhs.fog;
         highCloud   = rhs.highCloud;
         mediumCloud = rhs.mediumCloud;
         lowCloud = rhs.lowCloud;
         RH2M     = rhs.RH2M;
         thunderProability = rhs.thunderProability;
         fogProability = rhs.fogProability;
         WIND_PROBABILITY = rhs.WIND_PROBABILITY;
         T2M_PROBABILITY_1 = rhs.T2M_PROBABILITY_1;
         T2M_PROBABILITY_2 = rhs.T2M_PROBABILITY_2;
         T2M_PROBABILITY_3 = rhs.T2M_PROBABILITY_3;
         T2M_PROBABILITY_4 = rhs.T2M_PROBABILITY_4;
         T2M_PERCENTILE_10 = rhs.T2M_PERCENTILE_10;
         T2M_PERCENTILE_25 = rhs.T2M_PERCENTILE_25;
         T2M_PERCENTILE_50 = rhs.T2M_PERCENTILE_50;
         T2M_PERCENTILE_75 = rhs.T2M_PERCENTILE_75;
         T2M_PERCENTILE_90 = rhs.T2M_PERCENTILE_90;
         PRECIP_PERCENTILE_10 = rhs.PRECIP_PERCENTILE_10;
         PRECIP_PERCENTILE_25 = rhs.PRECIP_PERCENTILE_25;
         PRECIP_PERCENTILE_50 = rhs.PRECIP_PERCENTILE_50;
         PRECIP_PERCENTILE_75 = rhs.PRECIP_PERCENTILE_75;
         PRECIP_PERCENTILE_90 = rhs.PRECIP_PERCENTILE_90;
         PRECIP_PROBABILITY_0_1MM = rhs.PRECIP_PROBABILITY_0_1MM;
         PRECIP_PROBABILITY_0_2MM = rhs.PRECIP_PROBABILITY_0_2MM;
         PRECIP_PROBABILITY_0_5MM = rhs.PRECIP_PROBABILITY_0_5MM;
         PRECIP_PROBABILITY_1_0MM = rhs.PRECIP_PROBABILITY_1_0MM;
         PRECIP_PROBABILITY_2_0MM = rhs.PRECIP_PROBABILITY_2_0MM;
         PRECIP_PROBABILITY_5_0MM = rhs.PRECIP_PROBABILITY_5_0MM;
         symbol   = rhs.symbol;
         symbol_PROBABILITY = rhs.symbol_PROBABILITY;
         modeltopography = rhs.modeltopography;
      }
      
      return *this;
   }
};

typedef std::map<std::string, std::string > RenameTopoProvider;

typedef std::map<std::string, PData>                   ProviderPDataList;
typedef std::map<std::string, PData>::iterator        IProviderPDataList;
typedef std::map<std::string, PData>::const_iterator CIProviderPDataList;

typedef std::map<boost::posix_time::ptime, ProviderPDataList >                  FromTimeSerie;
typedef std::map<boost::posix_time::ptime, ProviderPDataList>::iterator        IFromTimeSerie;
typedef std::map<boost::posix_time::ptime, ProviderPDataList>::const_iterator CIFromTimeSerie;


///My access the map as map[totime][fromtime][dataprovider]
typedef std::map<boost::posix_time::ptime, FromTimeSerie>                   TimeSerie;
typedef std::map<boost::posix_time::ptime, FromTimeSerie>::iterator        ITimeSerie;
typedef std::map<boost::posix_time::ptime, FromTimeSerie>::const_iterator CITimeSerie;

typedef boost::shared_ptr<TimeSerie> TimeSeriePtr;

std::string
toBeaufort( float mps, std::string &description );

std::string
windDirectionName( float dd );

std::string 
symbolidToName( int id );


void 
decodePData( const ParamDefList &paramDefs,
		       const ProviderList &providers,
		       const ProviderRefTimeList &refTimeList,
		       int   protocol,
		       const pqxx::result &result,
		       TimeSerie &timeSerie );

}

#endif

