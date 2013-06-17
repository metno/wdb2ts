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


}
