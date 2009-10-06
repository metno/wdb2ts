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
			
			if( ! findParam( it, paramDef, paramDefs ) ) {
				++it;
				STOP_MARK_MI_PROFILE("findParam");
				STOP_MARK_MI_PROFILE("db::decode");
				continue;
			}
				
			STOP_MARK_MI_PROFILE("findParam");
			
			if( alias !=  paramDef->alias() ) {
				alias = paramDef->alias();
				//WEBFW_LOG_DEBUG( "Decoding: " << paramDef->alias() << "  (" << it.at("dataprovidername").c_str() <<")" );
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

			dataversion = refTimeList.getDataversion( providerWithPlacename );
			
			if( dataversion > -1 ) {
				//cerr << "Decoding: " << paramDef->alias() << "  (" << providerWithPlacename <<") dv: " << it.at("dataversion").as<int>() 
				//     << "  dataversion: " << dataversion << endl;
				if( it.at("dataversion").as<int>() != dataversion  ) {
					++it;
					continue;
				}
			}

			if( !decodePoint( it.at("point").c_str(), locationPoint ) ) {
				++it;
				continue;
			}
			
			//WEBFW_LOG_DEBUG( "decode: pdRef: timeSerie[" << to << "]["<<from <<"][" << providerWithPlacename <<"]" );

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
			
			WEBFW_LOG_DEBUG("decode: '" << paramDef->alias() << "'=" << value << " timeSerie[" << to << "]["<<from <<"][" << providerWithPlacename <<"]" );
			
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
			else if( paramDef->alias() == "PRECIP" )
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
				WEBFW_LOG_DEBUG( "[" << topoTime <<"][" << topoTime << "]["<<topo<< "]="<< value );
				(*itLpd->second)[topoTime][topoTime][topo].topography = value;
			}else if( paramDef->alias() == "TOTAL.CLOUD" )
				pd.NN = value;
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
