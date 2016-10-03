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


#include <math.h>
#include <iostream>
#include "metfunctions.h"

namespace {
const float ewt[41] = { .000034,.000089,.000220,.000517,.001155,.002472,
                         .005080,.01005, .01921, .03553, .06356, .1111,
                         .1891,  .3139,  .5088,  .8070,  1.2540, 1.9118,
                         2.8627, 4.2148, 6.1078, 8.7192, 12.272, 17.044,
                         23.373, 31.671, 42.430, 56.236, 73.777, 95.855,
                         123.40, 157.46, 199.26, 250.16, 311.69, 385.56,
                         473.67, 578.09, 701.13, 845.28, 1013.25 };
}

using namespace std;

namespace miutil {

float
dewPointTemperatureToRelativeHumidity( float temperature, float dewPointTemperature )
{
   float x, et, etd, rh;
   int l;

   if( temperature == FLT_MAX || dewPointTemperature == FLT_MAX )
      return FLT_MAX;

   if( temperature == FLT_MAX || dewPointTemperature == FLT_MAX ||
       temperature >= 105 || temperature <= -100 ||
       dewPointTemperature >= 105 || dewPointTemperature <= -100 )
      return FLT_MAX;

   x = (temperature + 100.) * 0.2;
   l = int(x);
   et = ewt[l] + (ewt[l + 1] - ewt[l]) * (x - float(l));

   x = (dewPointTemperature + 100.) * 0.2;
   l = int(x);
   etd = ewt[l] + (ewt[l + 1] - ewt[l]) * (x - float(l));
   rh = etd / et;

   return rh * 100.;
}


float
dewPointTemperature( float ta, float rh )
{
   if( ta == FLT_MAX || rh == FLT_MAX )
      return FLT_MAX;

   float e = (rh/100)*0.611*exp( (17.63 * ta) / (ta + 243.04) );
   float td = (116.9 + 243.04 * log( e ))/(16.78-log( e ));

   return (td<=ta ? td : ta);
}


std::string toBeaufort( float mps, std::string &name )
{
	if( mps == FLT_MAX ){
		name = "";
		return "";
	}

	//Round to nearst 10th and convert to integer.
	long int iMps=lround(mps*10);

	if( iMps < 3 ){  //0.0-0.2 m/s
		name = "Stille";
		return "0";
	}else if( iMps < 16 ){ //0.3-1.5 m/s
		name = "Flau vind";
		return "1";
	}else if( iMps < 34 ){ //1.6-3.3 m/s
		name = "Svak vind";
		return "2";
	}else if( iMps < 55 ){ //3.4-5.4 m/s
		name = "Lett bris";
		return "3";
	}else if( iMps < 80 ){ //5.5-7.9 m/s
		name = "Laber bris";
		return "4";
	}else if( iMps < 108 ){ //8.0-10.7 m/s
		name = "Frisk bris";
		return "5";
	}else if( iMps < 139 ){ //10.8-13.8 m/s
		name = "Liten kuling";
		return "6";
	}else if( iMps < 172 ){ //13.9-17.1 m/s
		name = "Stiv kuling";
		return "7";
	}else if( iMps < 208 ){ //17.2-20.7 m/s
		name = "Sterk kuling";
		return "8";
	}else if( iMps < 245 ){ //20.8-24.4 m/s
		name = "Liten storm";
		return "9";
	}else if( iMps < 285 ){ //24.5-28.4 m/s
		name = "Full storm";
		return "10";
	}else if( iMps < 327 ){ //28.5-32.6 m/s
		name = "Sterk storm";
		return "11";
	}else{
		name = "Orkan";
		return "12";
	}

	return name;
}


std::string windDirectionName( float dd )
{
	if( dd == FLT_MAX )
		return "";

	if( dd < 0 || dd > 360 )
		dd = 360;

	if( dd < 22.5 || dd >= 337.5 )
		return "N";
	else if( dd >= 22.5 && dd < 67.5 )
		return "NE";
	else if( dd >= 67.5 && dd < 112.5 )
		return "E";
	else if( dd >= 112.5 && dd < 157.5 )
		return "SE";
	else if( dd >= 157.5 && dd < 202.5 )
		return "S";
	else if( dd >= 202.5 && dd < 247.5 )
		return "SW";
	else if( dd >= 247.5 && dd < 292.5 )
		return "W";
	else
		return "NW";
}

std::string symbolidToName( int id )
{
	if( id < 0 )
		return "";

	switch( id ){
	case 1:
	case 16:
		return "SUN";
	case 2:
	case 17:
		return "LIGHTCLOUD";
	case 3:
		return "PARTLYCLOUD";
	case 4:
		return "CLOUD";
	case 5:
		return "LIGHTRAINSUN";
	case 18:
		return "LIGHTRAINSUN";
	case 6:
		return "LIGHTRAINTHUNDERSUN";
	case 7:
		return "SLEETSUN";
	case 8:
		return "SNOWSUN";
	case 9:
		return "LIGHTRAIN";
	case 10:
		return "RAIN";
	case 11:
		return "RAINTHUNDER";
	case 12:
		return "SLEET";
	case 13:
		return "SNOW";
	case 14:
		return "SNOWTHUNDER";
	case 15:
		return "FOG";
	case 19:
		return "SNOWSUN";
	case 20:
		return "SLEETSUNTHUNDER";
	case 21:
		return "SNOWSUNTHUNDER";
	case 22:
		return "LIGHTRAINTHUNDER";
	case 23:
		return "SLEETTHUNDER";
	default:
		return "";         //Unknown symbol
	}
}



}
