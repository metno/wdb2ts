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


#include <float.h>
#include <math.h>
#include <contenthandlers/LocationForecastGml/GML/WdbMomentTags.h>
#include <iostream>
#include <iomanip>
#include <probabilityCode.h>
#include <Logger4cpp.h>
#include "metfunctions.h"


#define WRITE_DEBUG( loglevel, msg ) { \
   if( loglevel >= log4cpp::Priority::DEBUG ) { \
      msg; \
   } \
}

namespace wdb2ts {

using namespace std;
void 
WdbMomentTags::
init( const LocationElem &pointData ) 
{

   pd = &pointData;
   ff=FLT_MAX;
   dd=FLT_MAX;

}

//Compute the wind direction and speed. 
//TODO: The computation of wind direction maybe wrong check it.
//The problem maybe that the u and v components is oriented to the
//grid and must be corrected so they are oriented to the geographic
//directions.
void 
WdbMomentTags::
computeWind( float u, float v )
{
   if( u==FLT_MAX || v==FLT_MAX )
      return;

   ff = sqrtf( u*u + v*v);

   float deg=180./3.141592654;

   if( ff > 0.0001) {
      dd = 270.0 - deg * atan2( v, u );

      if( dd > 360 )
         dd -= 360;
      if( dd < 0 )
         dd += 360;
   } else {
      dd = 0;
   }

}

bool  
WdbMomentTags::
vectorLengthAndDirection( const std::string &provider,
                          float latitude, float longitude,
                          float u, float v,
                          float &direction,
                          float &length,
                          bool turn )
{
   if( u == FLT_MAX || v == FLT_MAX )
      return false;

   return gmlContext->projectionHelper->convertToDirectionAndLength( provider,
                                                                     latitude, longitude,
                                                                     u, v,
                                                                     direction, length,
                                                                     turn );
}


float 
WdbMomentTags::
getTemperatureProability( float temp )const
{
   float prob;

   if( temp > -99.0 && temp < -10.0 )
      prob = pd->T2M_PROBABILITY_2();
   else if( temp >= -10.0 && temp < 100.0 )
      prob = pd->T2M_PROBABILITY_1();
   else
      prob = FLT_MAX;

   /*
	if( temp > -99.0 && temp < -20.0 )
		prob = pd->T2M_PROBABILITY_4();
	else if( ( temp >= -20.0 && temp < -10.0 ) || ( temp >= 25.0 && temp < 99.0 ) )
		prob = pd->T2M_PROBABILITY_3();
	else if( ( temp >= -10.0 && temp < -2.0 )  || ( temp >= 2.0 && temp < 25.0) )
		prob = pd->T2M_PROBABILITY_2();
	else if( temp >= -2.0 && temp < 2.0 )
		prob = pd->T2M_PROBABILITY_1();
	else
		prob = FLT_MAX;
    */
   return prob;
}


/* The symbols in the database is just basis symbols, ie they
 * have to be corrected after we have corrected the temperature
 * with hight.
 * 
 * The symbols is corrected after the following table. Where
 * T is the hight adjusted temperature.
 * 
 * SYMBOLS           |       Hight adjusted temperature
 * from db           | 0.5 >= T < 1.5  |    T < 0.5          
 * ------------------+-----------------+----------------  
 *  (5) LIGHTRAINSUN |  (7) SLEETSUN   |  (8) SNOWSUN
 *  (9) LIGHTRAIN    | (12) SLEET      | (13) SNOW
 * (10) RAIN         | (12) SLEET      | (13) SNOW
 */
void 
WdbMomentTags::
doSymbol( float temperature )
{
   boost::posix_time::ptime fromTime;

   int symNumber = pd->symbol( fromTime );

   //cerr << "doSymbol: "  << symNumber << endl;

   if( symNumber == INT_MAX )
      return;
   /*
	// LIGHTRAINSUN (5)  || LIGHTRAIN (9)  ||  RAIN (10)
	if( symNumber == 5 || symNumber == 9 || symNumber == 10 ) {
		if( temperature < 0.5 ) {
			if( symNumber == 5 ) //LIGHTRAINSUN
				symNumber = 8;  //SNOWSUN
			else
				symNumber = 13; //SNOW
		} else if( temperature >= 0.5 && temperature < 1.5 ) {
			if( symNumber == 5 ) //LIGHTRAINSUN
				symNumber = 7;   //SLEETSUN
			else
				symNumber = 12;  //SLEET
		}
	}
    */
   //cerr << "doSymbol: AddSymbol: " << symNumber << " (" << pd->symbol_PROBABILITY() << ") t: "
   //     << pd->time() << " p: " << pd->forecastprovider() << " lat: "  << pd->latitude()
   //     << endl;
   gmlContext->symbolContext.addSymbol( symNumber,
                                        pd->symbol_PROBABILITY( fromTime ),
                                        pd->time(),
                                        temperature,
                                        fromTime,
                                        pd->forecastprovider(),
                                        pd->latitude() );
}

void 
WdbMomentTags::
output( std::ostream &out_, miutil::Indent &indent_ ) 
{
   WEBFW_USE_LOGGER( "encode" );
   log4cpp::Priority::Value loglevel = WEBFW_GET_LOGLEVEL( );
   string description;
   string indent( indent_.spaces() );
   float value;
   float tempNoAdiabatic;
   float tempAdiabatic;
   float tempUsed=FLT_MAX;
   float iValue;
   string provider;
   string tempNoAdiabaticProvider;
   string tmpProvider;
   float tempCorrection;
   float tempProb = FLT_MAX;
   float windProb = FLT_MAX;
   float ice=FLT_MAX;
   float length, direction;
   float u, v;
   ostringstream out;
   int count=0;
   int relTopo;
   int modelTopo;

   if( ! pd )
      return;

   out.setf(ios::floatfield, ios::fixed);
   out.precision(1);

   /*
    * Coding of temperature.
    * The temperature may or may not be corrected for height. If the
    * temperature is corrected the difference between the model height and
    * real height is used to do a adiabatic height correction.
    *
    * We decide if we shall do a height correction or not on the avalable
    * temperatures. If the parameter pd->T2M_NO_ADIABATIC_HIGHT_CORRECTION
    * has a value we do not do height correction, if no we do. ie. the no
    * height correction has priority.
    */

   tempNoAdiabatic = pd->T2M_NO_ADIABATIC_HIGHT_CORRECTION( true );

   if( tempNoAdiabatic != FLT_MAX )
      tempNoAdiabaticProvider = provider = pd->forecastprovider();

   value = pd->TA( true );
   tempAdiabatic = value;

   if( value != FLT_MAX ) {
      provider = pd->forecastprovider();
      tempCorrection = pd->computeTempCorrection( provider, relTopo, modelTopo );
      tempAdiabatic += tempCorrection;

      WRITE_DEBUG( loglevel, out << indent << "<!-- Dataprovider: " << provider << " tempAdiabatic: " << tempAdiabatic
             << " tempAdiabatic: " << value << " (uncorrected) modelTopo: " << modelTopo << " relTopo: " << relTopo << " -->\n");

   }

   if( tempNoAdiabatic != FLT_MAX ) {
      value = tempNoAdiabatic;
   } else 	if( tempAdiabatic != FLT_MAX ) {
      value = tempAdiabatic;
      tempProb = getTemperatureProability( value );
   }

   if( value != FLT_MAX ) {
      if( tempNoAdiabaticProvider.empty()) {
         WRITE_DEBUG( loglevel, out << indent << "<!-- Dataprovider: " << provider << " -->\n");
      } else {
         WRITE_DEBUG( loglevel, out << indent << "<!-- Dataprovider: " << tempNoAdiabaticProvider << " -->\n");
      }

      if( loglevel >= log4cpp::Priority::DEBUG &&
          tempNoAdiabatic != FLT_MAX && tempAdiabatic!=FLT_MAX ) {
         WRITE_DEBUG( loglevel, out << indent << "<!-- Dataprovider: " << provider << " tempAdiabatic: " << tempAdiabatic
               << " tempNoAdiabatic: " << tempNoAdiabatic << " " <<  tempNoAdiabaticProvider << " -->\n");
      }

      doSymbol( value );
      out << indent << "<mox:airTemperature uom=\"Cel\">" << value << "</mox:airTemperature>\n";
      tempUsed = value;
      count++;
   }

   //If we only have a temperature for the no "height correction" choice we need
   //another parameter to lock into the provider to use. We use the U component to the wind.
   if( provider.empty() && tempNoAdiabatic != FLT_MAX  ) {
      float tmp = pd->windU10m( true );

      if( tmp != FLT_MAX )
         provider = pd->forecastprovider();
   }

   if( vectorLengthAndDirection( provider,
                                 pd->latitude(), pd->longitude(),
                                 pd->windU10m(), pd->windV10m(),
                                 direction, length
   )  )
   {
      out << indent
            << "<mox:windDirection  uom=\"deg\">" << direction << "</mox:windDirection>\n"
            << indent
            << "<mox:windSpeed uom=\"m/s\">" << length << "</mox:windSpeed>\n";

      windProb = pd->WIND_PROBABILITY();
      count++;
   }

   value = pd->humidity( true );
   if (value != FLT_MAX) {
      out << indent << "<mox:humidity uom=\"percent\">" << value << "</mox:humidity>\n";
      count++;
   }

   value = pd->PR();
   if( value != FLT_MAX ) {
      out << indent << "<mox:airPressure uom=\"hPa\">" << value << "</mox:airPressure>\n";
      count++;
   }

   value = pd->NN();
   if( value != FLT_MAX ) {
      out << indent << "<mox:totalCloudCover uom=\"percent\">" << value << "</mox:totalCloudCover>\n";
      count++;
   }
   value = pd->visibility();
   if( value != FLT_MAX ) {
      out << indent << "<mox:visibility uom=\"m\">" << value << "</mox:visibility>\n";
      count++;
   }

   value = pd->fog();
   if( value != FLT_MAX ) {
      out << indent << "<mox:fogCover uom=\"percent\">" << value << "</mox:fogCover>\n";
      count++;
   }

   value = pd->lowCloud();
   if( value != FLT_MAX ) {
      out << indent << "<mox:lowCloudCover uom=\"percent\">" << value << "</mox:lowCloudCover>\n";
      count++;
   }

   value = pd->mediumCloud();
   if( value !=FLT_MAX ) {
      out << indent << "<mox:mediumCloudCover uom=\"percent\">" << value << "</mox:mediumCloudCover>\n";
      count++;
   }

   value = pd->highCloud();
   if( value != FLT_MAX ) {
      out << indent << "<mox:highCloudCover uom=\"percent\">" << value << "</mox:highCloudCover>\n";
      count++;
   }

   if( gmlContext->forecastType == GMLContext::LocationForecast &&
         pd->config && pd->config->outputParam("seaIceingIndex") ) {
      value = pd->iceingIndex( false, pd->forecastprovider() );
      if( value != FLT_MAX ) {
         out << indent << "<mox:seaIceingIndex>" << value << "</mox:seaIceingIndex>\n";
         count++;
      }
   }

   if( pd->config && pd->config->outputParam("dewpointTemperature") ) {
      float dewpoint = pd->dewPointTemperature( tempUsed, true );

      if( dewpoint != FLT_MAX ) {
         out << indent << "<mox:dewPointTemperature uom=\"Cel\">" << value << "</mox:dewPointTemperature>\n";
         count++;
      }
   }


   /* Ocean paramaters */
   if( gmlContext->forecastType == GMLContext::OceanForecast ) {
      iValue = pd->seaBottomTopography( tmpProvider );
      if( iValue != INT_MAX ) {
         WRITE_DEBUG( loglevel, out << "<!-- provider: " << tmpProvider << " (seaBottomTopography) -->\n");
         out << indent << "<mox:seaBottomTopography uom=\"m\">" << iValue << "</mox:seaBottomTopography>\n";
         count++;
      }

      ice = pd->seaIcePresence( tmpProvider );

      if( ice != FLT_MAX) {
         WRITE_DEBUG( loglevel,out << "<!-- provider: " << tmpProvider << " (seaIcePresence) ice: " << ice << " -->\n");
         out << indent << "<mox:seaIcePresence uom=\"%\">" << setprecision(0)<< ice << setprecision(1) << "</mox:seaIcePresence>\n";
         count++;
      }

      value = pd->iceingIndex( true );
      if( value != FLT_MAX ) {
         WRITE_DEBUG( loglevel,out << "<!-- provider: " << pd->oceanProvider() << " (seaIceingIndex) ice: " << ice << " -->\n");
         out << indent << "<mox:seaIceingIndex>" << value << "</mox:seaIceingIndex>\n";
         count++;
      }

      //Wave parameters
      value = pd->meanTotalWaveDirection( true );
      if( value != FLT_MAX ) {
         WRITE_DEBUG( loglevel, out << "<!-- provider: " << pd->oceanProvider() << " (meanWaveDirection) ice: " << ice << " -->\n");
         out << indent << "<mox:meanTotalWaveDirection uom=\"deg\">" << value << "</mox:meanTotalWaveDirection>\n";
         count++;
      }

      value = pd->meanTotalWavePeriode( true );
      if( value != FLT_MAX ){
      	out << indent << "<mox:meanTotalWavePeriode uom=\"m\">" << value << "</mox:meanTotalWavePeriode>\n";
         count++;
      }

      value =  pd->maximumTotalWaveHeight( true );
      if( value != FLT_MAX ){
      	out << indent << "<mox:maximumTotalWaveHeight uom=\"m\">" << value << "</mox:maximumTotalWaveHeight>\n";
         count++;
      }

      value = pd->significantTotalWaveHeight( true );
      if( value != FLT_MAX ){
      	out << indent << "<mox:significantTotalWaveHeight uom=\"m\">" << value << "</mox:significantTotalWaveHeight>\n";
         count++;
      }

      value = pd->significantSwellWaveHeight( true );
      if( value != FLT_MAX ) {
      	out << indent << "<mox:significantSwellWaveHeight uom=\"m\">" << value << "</mox:significantSwellWaveHeight>\n";
         count++;
      }

      value = pd->meanSwellWavePeriode( true );
      if( value != FLT_MAX ){
      	out << indent << "<mox:meanSwellWavePeriode uom=\"s\">" << value << "</mox:meanSwellWavePeriode>\n";
         count++;
      }


      value = pd->meanSwellWaveDirection( true );
      if( value != FLT_MAX ) {
      	out << indent << "<mox:meanSwellWaveDirection uom=\"deg\">" << value << "</mox:meanSwellWaveDirection>\n";
      	count++;
      }

      value = pd->peakSwellWavePeriode( true );

      if( value != FLT_MAX ) {
      	out << indent << "<mox:peakSwellWavePeriode uom=\"s\">" << value << "</mox:peakSwellWavePeriode>\n";
         count++;
      }


      value = pd->peakSwellWaveDirection( true );
      if( value != FLT_MAX ){
      	out << indent << "<mox:peakSwellWaveDirection uom=\"deg\">" << value << "</mox:peakSwellWaveDirection>\n";
         count++;
      }


      //Circulation parameters

      u = pd->seaCurrentVelocityU(true);
      v = pd->seaCurrentVelocityV();

      if( vectorLengthAndDirection( pd->oceanProvider() ,
                                    pd->latitude(), pd->longitude(),
                                    u, v,
                                    direction, length, true )  )
      {
         WRITE_DEBUG( loglevel,out << "<!-- provider: " << pd->oceanProvider()  << " (seaCurrentDirection/seaCurrentSpeed) -->\n");
         out << indent << "<mox:seaCurrentDirection uom=\"deg\">" << direction << "</mox:seaCurrentDirection>\n";
         out << indent << "<mox:seaCurrentSpeed uom=\"m/s\">" << setprecision(2) << length << setprecision(1) << "</mox:seaCurrentSpeed>\n";
         count += 2;
      }


      value = pd->seaSalinity( true );
      if( value != FLT_MAX ) {
         WRITE_DEBUG( loglevel,out << "<!-- provider: " << pd->oceanProvider()  << " (seaSalinity) -->\n");
         out << indent << "<mox:seaSalinity>" << value << "</mox:seaSalinity>\n";
         count++;
      }

      value = pd->seaSurfaceHeight( true );
      if( value != FLT_MAX ) {
         WRITE_DEBUG( loglevel,out << "<!-- provider: " << pd->oceanProvider()  << " (seaSurfaceHeight) -->\n");
         out << indent << "<mox:seaSurfaceHeight uom=\"m\">" << setprecision(2) << value << setprecision(1) << "</mox:seaSurfaceHeight>\n";
         count++;
      }

      value = pd->seaTemperature( true );
      if( value != FLT_MAX ) {
         WRITE_DEBUG( loglevel,out << "<!-- provider: " << pd->oceanProvider()  << " (seaTemperature) -->\n");
         out << indent << "<mox:seaTemperature uom=\"Cel\">" << value << "</mox:seaTemperature>\n";
         count++;
      }

   }

   if( count > 0 ) //count > 2
      out_ << out.str();
}

}
