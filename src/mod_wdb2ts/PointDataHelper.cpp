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

#include <stdexcept>
#include <iostream>
#include <float.h>
#include <PointDataHelper.h>
#include <ptimeutil.h>
#include <stdexcept>
#include <wdb2tsProfiling.h>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <Logger4cpp.h>


DEFINE_MI_PROFILE;

using namespace std;

namespace {

	/*
	 * Decode a string on the format point( longitude latitude )
	 */
	 bool decodePoint( const std::string &sPoint, wdb2ts::LocationPoint &locationPoint )
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
		 catch( const exception &ex ) {
			 WEBFW_USE_LOGGER( "decode" );
			 WEBFW_LOG_ERROR( "EXCEPTION: decodePoint:   " << ex.what()  );
			 return false;
		 }

		 return true;
	 }

}


namespace wdb2ts {


void
PData::
merge( const PData &other )
{

	if( other.windV10m != FLT_MAX ) windV10m = other.windV10m;
    if( other.windU10m != FLT_MAX ) windU10m = other.windU10m;
    if( other.PP != FLT_MAX ) PP = other.PP;
    if( other.PR != FLT_MAX ) PR = other.PR;
    if( other.TA != FLT_MAX ) TA = other.TA;
    if( other.T2M != FLT_MAX ) T2M = other.T2M;
    if( other.T2M_LAND  != FLT_MAX ) T2M_LAND = other.T2M_LAND;
    if( other.T2M_NO_ADIABATIC_HIGHT_CORRECTION != FLT_MAX )
		T2M_NO_ADIABATIC_HIGHT_CORRECTION = other.T2M_NO_ADIABATIC_HIGHT_CORRECTION;
    if( other.temperatureCorrected != FLT_MAX ) temperatureCorrected = other.temperatureCorrected;
    if( other.UU != FLT_MAX ) UU = other.UU;
    if( other.PRECIP_PROBABILITY != FLT_MAX ) PRECIP_PROBABILITY = other.PRECIP_PROBABILITY;
    if( other.PRECIP_MIN != FLT_MAX ) PRECIP_MIN = other.PRECIP_MIN;
    if( other.PRECIP_MAX != FLT_MAX ) PRECIP_MAX = other.PRECIP_MAX;
    if( other.PRECIP_MEAN != FLT_MAX ) PRECIP_MEAN = other.PRECIP_MEAN;
    if( other.PRECIP != FLT_MAX ) PRECIP = other.PRECIP;
    if( other.PRECIP_ACCUMULATED != FLT_MAX ) PRECIP_ACCUMULATED = other.PRECIP_ACCUMULATED;
    if( other.PRECIP_1T != FLT_MAX ) PRECIP_1T = other.PRECIP_1T;
    if( other.PRECIP_3T != FLT_MAX ) PRECIP_3T = other.PRECIP_3T;
    if( other.PRECIP_6T != FLT_MAX ) PRECIP_6T  = other.PRECIP_6T;
    if( other.PRECIP_12T != FLT_MAX ) PRECIP_12T = other.PRECIP_12T;
    if( other.PRECIP_24T != FLT_MAX ) PRECIP_24T = other.PRECIP_24T;
    if( other.seaCurrentVelocityV != FLT_MAX ) seaCurrentVelocityV = other.seaCurrentVelocityV;
    if( other.seaCurrentVelocityU != FLT_MAX ) seaCurrentVelocityU = other.seaCurrentVelocityU;
    if( other.seaSalinity != FLT_MAX ) seaSalinity = other.seaSalinity;
    if( other.seaSurfaceHeight != FLT_MAX ) seaSurfaceHeight = other.seaSurfaceHeight;
    if( other.seaTemperature != FLT_MAX ) seaTemperature = other.seaTemperature;
    if( other.meanTotalWaveDirection != FLT_MAX ) meanTotalWaveDirection = other.meanTotalWaveDirection;
    if( other.significantTotalWaveHeight != FLT_MAX ) significantTotalWaveHeight = other.significantTotalWaveHeight;
    if( other.seaIcePresence != FLT_MAX ) seaIcePresence = other.seaIcePresence;
    if( other.iceingIndex != FLT_MAX ) iceingIndex = other.iceingIndex;
    if( other.seaBottomTopography != FLT_MAX ) seaBottomTopography = other.seaBottomTopography;
    if( other.NN != FLT_MAX ) NN = other.NN;
    if( other.visibility != FLT_MAX ) visibility = other.visibility;
    if( other.fog != FLT_MAX ) fog = other.fog;
    if( other.highCloud != FLT_MAX ) highCloud = other.highCloud;
    if( other.mediumCloud != FLT_MAX ) mediumCloud = other.mediumCloud;
    if( other.lowCloud != FLT_MAX ) lowCloud = other.lowCloud;
    if( other.RH2M != FLT_MAX ) RH2M = other.RH2M;
    if( other.thunderProability != FLT_MAX ) thunderProability = other.thunderProability;
    if( other.fogProability != FLT_MAX ) fogProability = other.fogProability;
    if( other.WIND_PROBABILITY != FLT_MAX ) WIND_PROBABILITY = other.WIND_PROBABILITY;
    if( other.T2M_PROBABILITY_1 != FLT_MAX ) T2M_PROBABILITY_1 = other.T2M_PROBABILITY_1;
    if( other.T2M_PROBABILITY_2 != FLT_MAX ) T2M_PROBABILITY_2 = other.T2M_PROBABILITY_2;
    if( other.T2M_PROBABILITY_3 != FLT_MAX ) T2M_PROBABILITY_3 = other.T2M_PROBABILITY_3;
    if( other.T2M_PROBABILITY_4 != FLT_MAX ) T2M_PROBABILITY_4 = other.T2M_PROBABILITY_4;
    if( other.T2M_PERCENTILE_10 != FLT_MAX ) T2M_PERCENTILE_10 = other.T2M_PERCENTILE_10;
    if( other.T2M_PERCENTILE_25 != FLT_MAX ) T2M_PERCENTILE_25 = other.T2M_PERCENTILE_25;
    if( other.T2M_PERCENTILE_50 != FLT_MAX ) T2M_PERCENTILE_50 = other.T2M_PERCENTILE_50;
    if( other.T2M_PERCENTILE_75 != FLT_MAX ) T2M_PERCENTILE_75 = other.T2M_PERCENTILE_75;
    if( other.T2M_PERCENTILE_90 != FLT_MAX ) T2M_PERCENTILE_90 = other.T2M_PERCENTILE_90;
    if( other.PRECIP_PERCENTILE_10 != FLT_MAX ) PRECIP_PERCENTILE_10 = other.PRECIP_PERCENTILE_10;
    if( other.PRECIP_PERCENTILE_25 != FLT_MAX ) PRECIP_PERCENTILE_25 = other.PRECIP_PERCENTILE_25;
    if( other.PRECIP_PERCENTILE_50 != FLT_MAX ) PRECIP_PERCENTILE_50 = other.PRECIP_PERCENTILE_50;
    if( other.PRECIP_PERCENTILE_75 != FLT_MAX ) PRECIP_PERCENTILE_75 = other.PRECIP_PERCENTILE_75;
    if( other.PRECIP_PERCENTILE_90 != FLT_MAX ) PRECIP_PERCENTILE_90 = other.PRECIP_PERCENTILE_90;
    if( other.PRECIP_PROBABILITY_0_1MM != FLT_MAX ) PRECIP_PROBABILITY_0_1MM = other.PRECIP_PROBABILITY_0_1MM;
    if( other.PRECIP_PROBABILITY_0_2MM != FLT_MAX ) PRECIP_PROBABILITY_0_2MM = other.PRECIP_PROBABILITY_0_2MM;
    if( other.PRECIP_PROBABILITY_0_5MM != FLT_MAX ) PRECIP_PROBABILITY_0_5MM = other.PRECIP_PROBABILITY_0_5MM;
    if( other.PRECIP_PROBABILITY_1_0MM != FLT_MAX ) PRECIP_PROBABILITY_1_0MM = other.PRECIP_PROBABILITY_1_0MM;
    if( other.PRECIP_PROBABILITY_2_0MM != FLT_MAX ) PRECIP_PROBABILITY_2_0MM = other.PRECIP_PROBABILITY_2_0MM;
    if( other.PRECIP_PROBABILITY_5_0MM != FLT_MAX ) PRECIP_PROBABILITY_5_0MM = other.PRECIP_PROBABILITY_5_0MM;
    if( other.symbol != FLT_MAX ) symbol = other.symbol;
    if( other.symbol_PROBABILITY != FLT_MAX ) symbol_PROBABILITY = other.symbol_PROBABILITY;
    if( other.modeltopography != FLT_MAX ) modeltopography = other.modeltopography;
    if( other.topography != FLT_MAX ) topography = other.topography;
}

int
PData::
count()const
{
	int n=0;

	if( windV10m != FLT_MAX ) ++n;
    if( windU10m != FLT_MAX ) ++n;
    if( PP != FLT_MAX ) ++n;
    if( PR != FLT_MAX ) ++n;
    if( TA != FLT_MAX ) ++n;
    if( T2M != FLT_MAX ) ++n;
    if( T2M_LAND  != FLT_MAX ) ++n;
    if( T2M_NO_ADIABATIC_HIGHT_CORRECTION != FLT_MAX ) ++n;
    if( temperatureCorrected != FLT_MAX ) ++n;
    if( UU != FLT_MAX ) ++n;
    if( PRECIP_PROBABILITY != FLT_MAX ) ++n;
    if( PRECIP_MIN != FLT_MAX ) ++n;
    if( PRECIP_MAX != FLT_MAX ) ++n;
    if( PRECIP_MEAN != FLT_MAX ) ++n;
    if( PRECIP != FLT_MAX ) ++n;
    if( PRECIP_ACCUMULATED != FLT_MAX ) ++n;
    if( PRECIP_1T != FLT_MAX ) ++n;
    if( PRECIP_3T != FLT_MAX ) ++n;
    if( PRECIP_6T != FLT_MAX ) ++n;
    if( PRECIP_12T != FLT_MAX ) ++n;
    if( PRECIP_24T != FLT_MAX ) ++n;
    if( seaCurrentVelocityV != FLT_MAX ) ++n;
    if( seaCurrentVelocityU != FLT_MAX ) ++n;
    if( seaSalinity != FLT_MAX ) ++n;
    if( seaSurfaceHeight != FLT_MAX ) ++n;
    if( seaTemperature != FLT_MAX ) ++n;
    if( meanTotalWaveDirection != FLT_MAX ) ++n;
    if( significantTotalWaveHeight != FLT_MAX ) ++n;
    if( seaIcePresence != FLT_MAX ) ++n;
    if( iceingIndex != FLT_MAX ) ++n;
    if( NN != FLT_MAX ) ++n;
    if( visibility != FLT_MAX ) ++n;
    if( fog != FLT_MAX ) ++n;
    if( highCloud != FLT_MAX ) ++n;
    if( mediumCloud != FLT_MAX ) ++n;
    if( lowCloud != FLT_MAX ) ++n;
    if( RH2M != FLT_MAX ) ++n;
    if( thunderProability != FLT_MAX ) ++n;
    if( fogProability != FLT_MAX ) ++n;
    if( WIND_PROBABILITY != FLT_MAX ) ++n;
    if( T2M_PROBABILITY_1 != FLT_MAX ) ++n;
    if( T2M_PROBABILITY_2 != FLT_MAX ) ++n;
    if( T2M_PROBABILITY_3 != FLT_MAX ) ++n;
    if( T2M_PROBABILITY_4 != FLT_MAX ) ++n;
    if( T2M_PERCENTILE_10 != FLT_MAX ) ++n;
    if( T2M_PERCENTILE_25 != FLT_MAX ) ++n;
    if( T2M_PERCENTILE_50 != FLT_MAX ) ++n;
    if( T2M_PERCENTILE_75 != FLT_MAX ) ++n;
    if( T2M_PERCENTILE_90 != FLT_MAX ) ++n;
    if( PRECIP_PERCENTILE_10 != FLT_MAX ) ++n;
    if( PRECIP_PERCENTILE_25 != FLT_MAX ) ++n;
    if( PRECIP_PERCENTILE_50 != FLT_MAX ) ++n;
    if( PRECIP_PERCENTILE_75 != FLT_MAX ) ++n;
    if( PRECIP_PERCENTILE_90 != FLT_MAX ) ++n;
    if( PRECIP_PROBABILITY_0_1MM != FLT_MAX ) ++n;
    if( PRECIP_PROBABILITY_0_2MM != FLT_MAX ) ++n;
    if( PRECIP_PROBABILITY_0_5MM != FLT_MAX ) ++n;
    if( PRECIP_PROBABILITY_1_0MM != FLT_MAX ) ++n;
    if( PRECIP_PROBABILITY_2_0MM != FLT_MAX ) ++n;
    if( PRECIP_PROBABILITY_5_0MM != FLT_MAX ) ++n;
    if( symbol != FLT_MAX ) ++n;
    if( symbol_PROBABILITY != FLT_MAX ) ++n;

    return n;
}

std::string
toBeaufort(float mps, std::string &description)
{
   float knop;
      
   if(mps==FLT_MAX){
      description=""; 
      return "";
   }
   
   knop=mps*1.94384449244;
         
   if (knop<1.0f){
      description="Stille"; return "0";
   }else if (knop>=1.0f && knop<4.0f){
      description="Flau vind"; return "1";
   }else if (knop>=4.0f && knop<7.0f){
      description="Svak vind"; return "2";
   }else if (knop>=7.0f && knop<11.0f){
      description="Lett bris"; return "3";
   }else if (knop>=11.0f && knop<17.0f){
      description="Laber bris"; return "4";
   }else if (knop>=17.0f && knop<22.0f){
      description="Frisk bris"; return "5";
   }else if (knop>=22.0f && knop<28.0f){
      description="Liten kuling"; return "6";
   }else if (knop>=28.0f && knop<34.0f){
      description="Stiv kuling"; return "7";
   }else if (knop>=34.0f && knop<41.0f){
      description="Sterk kuling"; return "8";
   }else if (knop>=41.0f && knop<48.0f){
      description="Liten storm"; return "9";
   }else if (knop>=48.0f && knop<56.0f){
      description="Full storm"; return "10";
   }else if (knop>=56.0f && knop<=63.0f){
      description="Sterk storm"; return "11";
   }else{ 
      description="Orkan"; return "12";
   }
}
   
std::string
windDirectionName(float dd)
{
   if(dd==FLT_MAX)
      return "";
      
   if(dd<0 || dd>360 )
      dd=360;

   if (dd<22.5 || dd>=337.5)
      return "N";
   else if (dd>=22.5 && dd<67.5)
      return "NE";
   else if (dd>=67.5 && dd<112.5)
      return "E";
   else if (dd>=112.5 && dd<157.5)
      return "SE";
   else if (dd>=157.5 && dd<202.5)
      return "S";
   else if (dd>=202.5 && dd<247.5)
      return "SW";
   else if (dd>=247.5 && dd<292.5)
      return "W";
   else 
      return "NW";
}

void 
decodePData( const ParamDefList &paramDefs, 
			 const ProviderList &providers,
			 const ProviderRefTimeList &refTimeList,
			 int   protocol,
		     const pqxx::result &result,
		     const bool isPolygonRequest,
		     LocationPointData &locationPointData )
{
	using namespace boost::posix_time;
	using namespace miutil;
	
	ptime from;
	ptime to;
	ptime reftime;
	ParamDefPtr paramDef;
	IFromTimeSerie itTimeSerie;
	string alias;
	ProviderList::const_iterator itProvider;
	string provider;
	string providerWithPlacename;
	string newProviderWithPlacename;
	string providerGroup;
	float value;
	int dataversion;
	LocationPoint locationPoint;
	LocationPointData::iterator itLpd;
	
	USE_MI_PROFILE;
	WEBFW_USE_LOGGER( "decode" );
	
	try {
		START_MARK_MI_PROFILE("db::it");
		pqxx::result::const_iterator it=result.begin();
		STOP_MARK_MI_PROFILE("db::it");
		
		while( it != result.end() ) {
			START_MARK_MI_PROFILE("db::decode");
			
			START_MARK_MI_PROFILE("findParam");

//			//DEBUG
//			string provider_dbg = it.at("dataprovidername").c_str();
////			if( provider_dbg.find("ecmwf atmo") != string::npos )
////			      provider_dbg.clear();
//
//			//DEBUGEND
			if( ! paramDefs.findParam( it, paramDef, providerGroup ) ) {
				++it;

//				//DEBUG
//				if( ! provider_dbg.empty() )
//				   cerr << "Cant find param: " << provider_dbg << "\n";
//				//DEBUGEND

				STOP_MARK_MI_PROFILE("findParam");
				STOP_MARK_MI_PROFILE("db::decode");
				continue;
			}
				
			STOP_MARK_MI_PROFILE("findParam");
			
			if( alias !=  paramDef->alias() ) {
				alias = paramDef->alias();

			}
			
			START_MARK_MI_PROFILE("db::it");
			reftime = ptimeFromIsoString( it.at("referencetime").c_str() );
			
			if( protocol > 2 ) {
				from = ptimeFromIsoString( it.at("validtimefrom").c_str() );
				to = ptimeFromIsoString( it.at("validtimeto").c_str() );
			} else {
				from = ptimeFromIsoString( it.at("validfrom").c_str() );
				to = ptimeFromIsoString( it.at("validto").c_str() );
			}
			
			STOP_MARK_MI_PROFILE("db::it");
			
			START_MARK_MI_PROFILE("timeSerie");
			
			itProvider = providers.findProvider( it.at("dataprovidername").c_str(), 
					                               it.at("placename").c_str(), 
					                               providerWithPlacename );

			if( itProvider == providers.end() ) {
				if( paramDef->alias() == "TOPOGRAPHY" ) {
					if( providerWithPlacename.empty() )
						providerWithPlacename = it.at("dataprovidername").c_str();
				}else  if( providerWithPlacename.empty() ){
					++it;
					continue;
				}
			}

			dataversion = refTimeList.getDataversion( providerWithPlacename );
			
			if( dataversion > -1 ) {
				if( it.at("dataversion").as<int>() != dataversion  ) {
					++it;
					continue;
				}
			}

			if( !decodePoint( it.at("point").c_str(), locationPoint ) ) {
				++it;
				continue;
			}
			
			itLpd = locationPointData.find( locationPoint );

			if( itLpd == locationPointData.end() ) {
				if( ! isPolygonRequest &&  ! locationPointData.empty() )
					itLpd = locationPointData.begin();

				if( itLpd == locationPointData.end() ) {
					//WEBFW_LOG_DEBUG( "decodePData: new location point: " << locationPoint.iLatitude() << "/" << locationPoint.iLongitude() );
					locationPointData[locationPoint] = TimeSeriePtr( new TimeSerie() );
					itLpd = locationPointData.find( locationPoint );

					if( itLpd == locationPointData.end() ) {
						WEBFW_LOG_ERROR( "decodePData: Unexpected serious error. Problem to lookup newly inserted TimeSerie." );
						++it;
						continue;
					}
				}
			}

//			//DBUG
//   		string oldProviderWithPlacename = providerWithPlacename;
//			if( !dataprovider_dbg.empty() )
//			   cerr << "Decode: provider: " << dataprovider_dbg << " withPlacename: " << providerWithPlacename << endl;
//			//DBUGEND

			renameProvider( providerWithPlacename, providerGroup  );
			PData &pd = (*itLpd->second)[to][from][providerWithPlacename];

 			STOP_MARK_MI_PROFILE("timeSerie");
	
 			if( it.at("value").is_null() ) {
 				++it;
 				continue;
 			}

			value = it.at("value").as<float>();
			
			//Check if it is a null value. Most relevant to fields
			//that has invalid values. ex ocean fields that has invalid
			//values on land. The value that identifies a null value is
			//defined in the paramdef section for the parameter in the
			//configuration file.
			if( paramDef->isNullValue( value ) ) {
				//WEBFW_LOG_DEBUG( "decode (null): " << paramDef->alias() << " value: " << value << " (" << providerWithPlacename << ")" )
				//     << "  " << *paramDef << endl;
				++it;
				continue;
			}
			
			value = value*paramDef->scale()+paramDef->offset();
			
			//WEBFW_LOG_DEBUG("decode: '" << paramDef->alias() << "'=" << value << " timeSerie[" << to << "]["<<from <<"][" << providerWithPlacename <<"]" );
			

//			//DBUG
//			if( ! provider_dbg.empty()  )
//			   cerr << "Decode: provider: " << provider_dbg << " Group: " << providerGroup
//                 << " nwPlacename: '" << providerWithPlacename << "' (" << oldProviderWithPlacename << ") Param: "
//                 << paramDef->alias() << " value: " << value << endl;
//			//DBUGEND

			if ( paramDef->alias() == "WIND.U10M" )
				pd.windU10m = value;
			else if( paramDef->alias() == "WIND.V10M")
				pd.windV10m = value;
			else if( paramDef->alias() == "PP" )
				pd.PP = value;
			else if( paramDef->alias() == "MSLP" )
			   pd.PR = value;
			else if( paramDef->alias() == "T.2M" ) {
				//WEBFW_LOG_DEBUG( "decode: T.2M: " << value );
				pd.T2M = value;
			} else if( paramDef->alias() == "T.2M.LAND" )
				pd.T2M_LAND = value;
			else if( paramDef->alias() == "T.2M.NO_ADIABATIC_HIGHT_CORRECTION" )
				pd.T2M_NO_ADIABATIC_HIGHT_CORRECTION = value;
			else if( paramDef->alias() == "UU" )
				pd.UU = value;
			else if( paramDef->alias() == "PRECIP.ACCUMULATED" )
				pd.PRECIP_ACCUMULATED = value;
			else if( paramDef->alias() == "PRECIP.PROBABILITY" )  {
            WEBFW_LOG_DEBUG( "decodePData: PRECIP.PROBABILITY: " << value << " (" << providerWithPlacename << " ["<<  from << " - " << to << ")");
			   pd.PRECIP_PROBABILITY = value;
			} else if( paramDef->alias() == "PRECIP.MIN" ) {
			   WEBFW_LOG_DEBUG( "decodePData: PRECIP.MIN: " << value << " (" << providerWithPlacename << " ["<<  from << " - " << to<< ")");
			   pd.PRECIP_MIN = value;
			} else if( paramDef->alias() == "PRECIP.MAX" ) {
            WEBFW_LOG_DEBUG( "decodePData: PRECIP.MAX: " << value << " (" << providerWithPlacename << " ["<<  from << " - " << to<< ")");
			   pd.PRECIP_MAX = value;
			} else if( paramDef->alias() == "PRECIP.MEAN" )  {
            WEBFW_LOG_DEBUG( "decodePData: PRECIP.MEAN: " << value << " (" << providerWithPlacename << " ["<<  from << " - " << to<< ")");
			   pd.PRECIP_MEAN = value;
			} else if( paramDef->alias() == "PRECIP" )
				pd.PRECIP = value;	
			else if( paramDef->alias() == "PRECIP.1H" )
				pd.PRECIP_1T = value;
			else if( paramDef->alias() == "PRECIP.3H" )
				pd.PRECIP_3T = value;
			else if( paramDef->alias() == "PRECIP.6H")
				pd.PRECIP_6T = value;
			else if( paramDef->alias() == "PRECIP.12H")
				pd.PRECIP_12T = value;
			else if( paramDef->alias() == "PRECIP.24H" )
				pd.PRECIP_24T = value;
			else if( paramDef->alias() == "seaCurrentVelocityV" )
				pd.seaCurrentVelocityV = value;
			else if( paramDef->alias() == "seaCurrentVelocityU" )
				pd.seaCurrentVelocityU = value;
			else if( paramDef->alias() == "seaSalinity" )
				pd.seaSalinity = value;
			else if( paramDef->alias() == "seaSurfaceHeight" )
				pd.seaSurfaceHeight = value;
			else if( paramDef->alias() == "seaTemperature" )
				pd.seaTemperature = value;
			else if( paramDef->alias() == "meanTotalWaveDirection" ) {
				//WEBFW_LOG_DEBUG( "decode: waveDirection: " << value );
				pd.meanTotalWaveDirection = value;
			} else if( paramDef->alias() == "significantTotalWaveHeight" ) {
				//WEBFW_LOG_DEBUG( "decode: significantWaveHeight: " << value );
				pd.significantTotalWaveHeight = value;
			}else if( paramDef->alias() == "seaBottomTopography" ) {
				ptime seaBottomTopographyTime( boost::gregorian::date(1970, 1, 1),
						                         boost::posix_time::time_duration( 0, 0, 0 ) );
				//WEBFW_LOG_DEBUG( "seaBottomTopography: [" << seaBottomTopographyTime <<"][" << seaBottomTopographyTime << "]["<<providerWithPlacename << "]="<< value );
				(*itLpd->second)[seaBottomTopographyTime][seaBottomTopographyTime][providerWithPlacename].seaBottomTopography = value;
			}else if( paramDef->alias() == "seaIcePresence" ) {
				ptime iceTime( boost::gregorian::date(1970, 1, 1),
						         boost::posix_time::time_duration( 0, 0, 0 ) );
				//WEBFW_LOG_DEBUG( "seaIcePresence: [" << iceTime <<"][" << iceTime << "]["<<providerWithPlacename << "]="<< value );
				(*itLpd->second)[iceTime][iceTime][providerWithPlacename].seaIcePresence = value;
			}else if( paramDef->alias() == "iceingIndex" ) {
				pd.iceingIndex = value;
			} else if( paramDef->alias() == "MODEL.TOPOGRAPHY" ) {
				string topo=providerWithPlacename+string("__MODEL_TOPO__");
				ptime topoTime( boost::gregorian::date(1970, 1, 1),
						          boost::posix_time::time_duration( 0, 0, 0 ) );
				//WEBFW_LOG_DEBUG( "[" << topoTime <<"][" << topoTime << "]["<<topo<< "]="<< value );
				(*itLpd->second)[topoTime][topoTime][topo].modeltopography = value;
			} else if( paramDef->alias() == "TOPOGRAPHY" ) {
				string topo=providerWithPlacename+string("__TOPOGRAPHY__");
				ptime topoTime( boost::gregorian::date(1970, 1, 1),
						boost::posix_time::time_duration( 0, 0, 0 ) );
				//WEBFW_LOG_DEBUG( "[" << topoTime <<"][" << topoTime << "]["<<topo<< "]="<< value );
				(*itLpd->second)[topoTime][topoTime][topo].topography = value;
			}else if( paramDef->alias() == "TOTAL.CLOUD" )
				pd.NN = value;
			else if( paramDef->alias() == "visibility" )
				pd.visibility = value;
			else if( paramDef->alias() == "FOG" )
				pd.fog = value;
			else if( paramDef->alias() == "LOW.CLOUD" )
				pd.lowCloud = value;
			else if( paramDef->alias() == "MEDIUM.CLOUD" )
				pd.mediumCloud = value;
			else if( paramDef->alias() == "HIGH.CLOUD" )
				pd.highCloud = value;
			else if( paramDef->alias() == "RH.2M" )
				pd.RH2M = value;
			else if( paramDef->alias() == "SYMBOL") 
				pd.symbol = value;
			else if( paramDef->alias() == "SYMBOL.PROBABILITY")
				pd.symbol_PROBABILITY = value;
			else if( paramDef->alias() == "THUNDER.PROBABILITY" )
				pd.thunderProability = value;
			else if( paramDef->alias() == "FOG.PROBABILITY" )
				pd.fogProability = value;
			else if( paramDef->alias() == "WIND.PROBABILITY")
				pd.WIND_PROBABILITY = value;
			else if( paramDef->alias() == "T.2M.PROBABILITY.1")
				pd.T2M_PROBABILITY_1 = value;
			else if( paramDef->alias() == "T.2M.PROBABILITY.2")
				pd.T2M_PROBABILITY_2 = value;
			else if( paramDef->alias() == "T.2M.PROBABILITY.3")
				pd.T2M_PROBABILITY_3 = value;
			else if( paramDef->alias() == "T.2M.PROBABILITY.4")
				pd.T2M_PROBABILITY_4 = value;
			else if( paramDef->alias() == "T.2M.PERCENTILE.10")
				pd.T2M_PERCENTILE_10 = value;
			else if( paramDef->alias() == "T.2M.PERCENTILE.25")
				pd.T2M_PERCENTILE_25 = value;
			else if( paramDef->alias() == "T.2M.PERCENTILE.50")
				pd.T2M_PERCENTILE_50 = value;
			else if( paramDef->alias() == "T.2M.PERCENTILE.75")
				pd.T2M_PERCENTILE_75 = value;
			else if( paramDef->alias() == "T.2M.PERCENTILE.90")
				pd.T2M_PERCENTILE_90 = value;
			else if( paramDef->alias() == "PRECIP.PERCENTILE.10")
				pd.PRECIP_PERCENTILE_10 = value;
			else if( paramDef->alias() == "PRECIP.PERCENTILE.25")
				pd.PRECIP_PERCENTILE_25 = value;
			else if( paramDef->alias() == "PRECIP.PERCENTILE.50")
				pd.PRECIP_PERCENTILE_50 = value;
			else if( paramDef->alias() == "PRECIP.PERCENTILE.75")
				pd.PRECIP_PERCENTILE_75 = value;
			else if( paramDef->alias() == "PRECIP.PERCENTILE.90")
				pd.PRECIP_PERCENTILE_90 = value;
			else if( paramDef->alias() == "PRECIP.PROBABILITY.0,1MM")
				pd.PRECIP_PROBABILITY_0_1MM = value;
			else if( paramDef->alias() == "PRECIP.PROBABILITY.0,2MM")
				pd.PRECIP_PROBABILITY_0_2MM = value;
			else if( paramDef->alias() == "PRECIP.PROBABILITY.0,5MM")
				pd.PRECIP_PROBABILITY_0_5MM = value;
			else if( paramDef->alias() == "PRECIP.PROBABILITY.1,0MM")
				pd.PRECIP_PROBABILITY_1_0MM = value;
			else if( paramDef->alias() == "PRECIP.PROBABILITY.2,0MM")
				pd.PRECIP_PROBABILITY_2_0MM = value;
			else if( paramDef->alias() == "PRECIP.PROBABILITY.5,0MM")
				pd.PRECIP_PROBABILITY_5_0MM = value;

			STOP_MARK_MI_PROFILE("db::decode");
			START_MARK_MI_PROFILE("db::it");
			++it;
			STOP_MARK_MI_PROFILE("db::it");
		}
	}
	catch( const std::ios_base::failure &ex ) {
		throw;
	}
	catch( const std::runtime_error &ex ) {
		throw std::logic_error( ex.what() );
	}
	catch( const std::logic_error &ex ) {
		throw;
	}
	catch( ... ) {
		throw std::logic_error( "Unknown error while decoding the result set." );
	}
}

SetPDataHelper::
SetPDataHelper()
   : pPM( 0 )
{
}

bool
SetPDataHelper::
init( const std::string &param )
{
   if ( param == "WIND.U10M" )
      pPM = &PData::windU10m;
   else if( param == "WIND.V10M")
      pPM = &PData::windV10m;
   else if( param == "PP" )
      pPM = &PData::PP;
   else if( param == "MSLP" )
      pPM = &PData::PR;
   else if( param == "T.2M" ) {
      pPM = &PData::T2M;
   } else if( param == "T.2M.LAND" )
      pPM = &PData::T2M_LAND;
   else if( param == "T.2M.NO_ADIABATIC_HIGHT_CORRECTION" )
      pPM = &PData::T2M_NO_ADIABATIC_HIGHT_CORRECTION;
   else if( param == "UU" )
      pPM = &PData::UU;
   else if( param == "PRECIP.PROBABILITY" )
      pPM = &PData::PRECIP_PROBABILITY;
   else if( param == "PRECIP.MIN" )
      pPM = &PData::PRECIP_MIN;
   else if( param == "PRECIP.MAX" )
      pPM = &PData::PRECIP_MAX;
   else if( param == "PRECIP.MEAN" )
      pPM = &PData::PRECIP_MEAN;
   else if( param == "PRECIP.ACCUMULATED" )
      pPM = &PData::PRECIP_ACCUMULATED;
   else if( param == "PRECIP" )
      pPM = &PData::PRECIP;
   else if( param == "PRECIP.1H" )
      pPM = &PData::PRECIP_1T;
   else if( param == "PRECIP.3H" )
      pPM = &PData::PRECIP_3T;
   else if( param == "PRECIP.6H")
      pPM = &PData::PRECIP_6T;
   else if( param == "PRECIP.12H")
      pPM = &PData::PRECIP_12T;
   else if( param == "PRECIP.24H" )
      pPM = &PData::PRECIP_24T;
   else if( param == "seaCurrentVelocityV" )
      pPM = &PData::seaCurrentVelocityV;
   else if( param == "seaCurrentVelocityU" )
      pPM = &PData::seaCurrentVelocityU;
   else if( param == "seaSalinity" )
      pPM = &PData::seaSalinity;
   else if( param == "seaSurfaceHeight" )
      pPM = &PData::seaSurfaceHeight;
   else if( param == "seaTemperature" )
      pPM = &PData::seaTemperature;
   else if( param == "meanTotalWaveDirection" )
      pPM = &PData::meanTotalWaveDirection;
   else if( param == "significantTotalWaveHeight" )
      pPM = &PData::significantTotalWaveHeight;
   else if( param == "seaBottomTopography" )
      pPM = &PData::seaBottomTopography;
   else if( param == "seaIcePresence" )
      pPM = &PData::seaIcePresence;
   else if( param == "iceingIndex" )
      pPM = &PData::iceingIndex;
   else if( param == "MODEL.TOPOGRAPHY" )
      pPM = &PData::modeltopography;
   else if( param == "TOPOGRAPHY" )
      pPM = &PData::topography;
   else if( param == "TOTAL.CLOUD" )
      pPM = &PData::NN;
   else if( param == "visibility" )
      pPM = &PData::visibility;
   else if( param == "FOG" )
      pPM = &PData::fog;
   else if( param == "LOW.CLOUD" )
      pPM = &PData::lowCloud;
   else if( param == "MEDIUM.CLOUD" )
      pPM = &PData::mediumCloud;
   else if( param == "HIGH.CLOUD" )
      pPM = &PData::highCloud;
   else if( param == "RH.2M" )
      pPM = &PData::RH2M;
   else if( param == "SYMBOL")
      pPM = &PData::symbol;
   else if( param == "SYMBOL.PROBABILITY")
      pPM = &PData::symbol_PROBABILITY;
   else if( param == "THUNDER.PROBABILITY" )
      pPM = &PData::thunderProability;
   else if( param == "FOG.PROBABILITY" )
      pPM = &PData::fogProability;
   else if( param == "WIND.PROBABILITY")
      pPM = &PData::WIND_PROBABILITY;
   else if( param == "T.2M.PROBABILITY.1")
      pPM = &PData::T2M_PROBABILITY_1;
   else if( param == "T.2M.PROBABILITY.2")
      pPM = &PData::T2M_PROBABILITY_2;
   else if( param == "T.2M.PROBABILITY.3")
      pPM = &PData::T2M_PROBABILITY_3;
   else if( param == "T.2M.PROBABILITY.4")
      pPM = &PData::T2M_PROBABILITY_4;
   else if( param == "T.2M.PERCENTILE.10")
      pPM = &PData::T2M_PERCENTILE_10;
   else if( param == "T.2M.PERCENTILE.25")
      pPM = &PData::T2M_PERCENTILE_25;
   else if( param == "T.2M.PERCENTILE.50")
      pPM = &PData::T2M_PERCENTILE_50;
   else if( param == "T.2M.PERCENTILE.75")
      pPM = &PData::T2M_PERCENTILE_75;
   else if( param == "T.2M.PERCENTILE.90")
      pPM = &PData::T2M_PERCENTILE_90;
   else if( param == "PRECIP.PERCENTILE.10")
      pPM = &PData::PRECIP_PERCENTILE_10;
   else if( param == "PRECIP.PERCENTILE.25")
      pPM = &PData::PRECIP_PERCENTILE_25;
   else if( param == "PRECIP.PERCENTILE.50")
      pPM = &PData::PRECIP_PERCENTILE_50;
   else if( param == "PRECIP.PERCENTILE.75")
      pPM = &PData::PRECIP_PERCENTILE_75;
   else if( param == "PRECIP.PERCENTILE.90")
      pPM = &PData::PRECIP_PERCENTILE_90;
   else if( param == "PRECIP.PROBABILITY.0,1MM")
      pPM = &PData::PRECIP_PROBABILITY_0_1MM;
   else if( param == "PRECIP.PROBABILITY.0,2MM")
      pPM = &PData::PRECIP_PROBABILITY_0_2MM;
   else if( param == "PRECIP.PROBABILITY.0,5MM")
      pPM = &PData::PRECIP_PROBABILITY_0_5MM;
   else if( param == "PRECIP.PROBABILITY.1,0MM")
      pPM = &PData::PRECIP_PROBABILITY_1_0MM;
   else if( param == "PRECIP.PROBABILITY.2,0MM")
      pPM = &PData::PRECIP_PROBABILITY_2_0MM;
   else if( param == "PRECIP.PROBABILITY.5,0MM")
      pPM = &PData::PRECIP_PROBABILITY_5_0MM;
   else
      pPM = 0;

   if( pPM )
      return true;

   return false;

}

void
SetPDataHelper::
set( PData &data, float value )
{
   if( pPM) {
      (&data)->*pPM = value;
   }
}

float
SetPDataHelper::
get( const PData &data ) const
{
   if( pPM )
      return (&data)->*pPM;

   return FLT_MIN;
}


std::string 
symbolidToName(int id) {
   if ( id<0 ) 
      return "";
      
   switch( id ){
   case  1:
   case 16: return "SUN";
   case  2:
   case 17: return "LIGHTCLOUD"; 
   case  3: return "PARTLYCLOUD"; 
   case  4: return "CLOUD";
   case  5: return "LIGHTRAINSUN"; 
   case 18: return "LIGHTRAINSUN"; 
   case  6: return "LIGHTRAINTHUNDERSUN";
   case  7: return "SLEETSUN";
   case  8: return "SNOWSUN"; 
   case  9: return "LIGHTRAIN";
   case 10: return "RAIN"; 
   case 11: return "RAINTHUNDER";
   case 12: return "SLEET";
   case 13: return "SNOW"; 
   case 14: return "SNOWTHUNDER";
   case 15: return "FOG";
   case 19: return "SNOWSUN"; 
   default:
      return "";         //Unknown symbol
   }
}

}
