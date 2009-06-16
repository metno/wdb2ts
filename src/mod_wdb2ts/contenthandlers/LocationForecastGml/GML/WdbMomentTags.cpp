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
	string description;
	string indent( indent_.spaces() );
	float value;
	float iValue;
	float dd_, ff_;
	string prevProvider( pd->forecastprovider() );
	string provider;
	string tmpProvider;
	float tempCorrection;
	float tempProb = FLT_MAX;
	float windProb = FLT_MAX;
	int ice=INT_MAX;
	bool hasTempCorr;
	float length, direction;
	float u, v;
	ostringstream out;
	int count=0;
	
	if( ! pd )
		return;
	
	out.setf(ios::floatfield, ios::fixed);
	out.precision(1);
	
	value = pd->TA( true );
	if( value != FLT_MAX ) {
		provider = pd->forecastprovider();
		tempCorrection = pd->computeTempCorrection( provider ); 
		
		out << indent << "<!-- Dataprovider: " << provider << " -->\n";
		
		prevProvider = provider;
		value += tempCorrection;
		doSymbol( value );
		tempProb = getTemperatureProability( value );

		out << indent << "<mox:airTemperature uom=\"Cel\">" << value << "</mox:airTemperature>\n";
		count++;
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
	
	value = pd->RH2M();
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

	/* Ocean paramaters */
	
	iValue = pd->seaBottomTopography( tmpProvider );
	if( iValue != INT_MAX ) {
		out << "<!-- provider: " << tmpProvider << " (seaBottomTopography) -->\n";
		out << indent << "<mox:seaBottomTopography>" << iValue << "</mox:seaBottomTopography>\n";
      count++;
   }

	
	ice = pd->seaIcePresence( tmpProvider );
	
	if( ice != INT_MAX ) {
		out << "<!-- provider: " << tmpProvider << " (seaIcePresence) ice: " << ice << " -->\n";
		out << indent << "<mox:seaIcePresence>" << ice << "</mox:seaIcePresence>\n";
	} else { 
		ice = 0;
	}
	
	//Wave parameters
	value = pd->meanTotalWaveDirection( true );
	if( value != FLT_MAX ) {
		out << "<!-- provider: " << pd->forecastprovider() << " (meanWaveDirection) ice: " << ice << " -->\n";

		if( ice ) {
			out << indent << "<mox:meanTotalWaveDirection xsi:nil=\"true\" nilReason=\"inapplicable\"/>\n";
		} else {
			out << indent << "<mox:meanTotalWaveDirection uom=\"deg\">" << value << "</mox:meanTotalWaveDirection>\n";
		}
		count++;
   }

	value = pd->significantTotalWaveHeight( true );
	if( value != FLT_MAX ){
		
		out << "<!-- provider: " << pd->forecastprovider() << " (significantTotalWaveHeight) ice: " << ice << " -->\n";

		if(  ice )
			out << indent << "<mox:significantTotalWaveHeight xsi:nil=\"true\" nilReason=\"inapplicable\"/>\n";
		else
			out << indent << "<mox:significantTotalWaveHeight uom=\"m\">" << value << "</mox:significantTotalWaveHeight>\n";

		count++;
   }

	
	//Circulation parameters
	
	u = pd->seaCurrentVelocityU(true);
	v = pd->seaCurrentVelocityV();
	
	if( vectorLengthAndDirection( pd->forecastprovider(), 
				                     pd->latitude(), pd->longitude(),
				                     u, v,
				                     direction, length, true )  )
	{
		out << "<!-- provider: " << pd->forecastprovider() << " (seaCurrentDirection/seaCurrentSpeed) -->\n";
		out << indent << "<mox:seaCurrentDirection uom=\"deg\">" << direction << "</mox:seaCurrentDirection>\n";
		out << indent << "<mox:seaCurrentSpeed uom=\"m/s\">" << setprecision(2) << length << setprecision(1) << "</mox:seaCurrentSpeed>\n";
      count += 2;
   }
	

	value = pd->seaSalinity( true );
	if( value != FLT_MAX ) {
		out << "<!-- provider: " << pd->forecastprovider() << " (seaSalinity) -->\n";
		out << indent << "<mox:seaSalinity>" << value << "</mox:seaSalinity>\n";
      count++;
   }

	value = pd->seaSurfaceHeight( true );
	if( value != FLT_MAX ) {
		out << "<!-- provider: " << pd->forecastprovider() << " (seaSurfaceHeight) -->\n";
		out << indent << "<mox:seaSurfaceHeight uom=\"m\">" << setprecision(2) << value << setprecision(1) << "</mox:seaSurfaceHeight>\n";
      count++;
   }

	value = pd->seaTemperature( true );
	if( value != FLT_MAX ) {
		out << "<!-- provider: " << pd->forecastprovider() << " (seaTemperature) -->\n";
		out << indent << "<mox:seaTemperature uom=\"Cel\">" << value << "</mox:seaTemperature>\n";
      count++;
   }

	
	if( count > 0 ) //count > 2
		out_ << out.str();
	
#if 0	
	out.precision( 0 );
	
	if( tempProb != FLT_MAX )
		out << indent << "<mox:airTemperatureProbability uom=\"probabilitycode\">" << probabilityCode( tempProb )<< "</mox:airTemperatureProbability>\n";
	
	if( windProb != FLT_MAX )
		out << indent << "<mox:windProbability uom=\"probabilitycode\">" << probabilityCode( windProb ) << "</mox:windProbability>\n";
	
	out.precision( oldPrec );
	
	hasTempCorr = false;
	tempCorrection = 0.0;
	value = pd->T2M_PERCENTILE_10( true );
	if( value != FLT_MAX ) {
		if( !hasTempCorr ) {
			tempCorrection = pd->computeTempCorrection( pd->percentileprovider() );
			hasTempCorr = true;
		} 
		
		value += tempCorrection;	
		out << indent << "<probability type=\"exact\" parameter=\"temperature\""
		    << " percentile=\"10\" unit=\"celcius\" value=\"" << value << "\"/>\n";
	}
	
	value = pd->T2M_PERCENTILE_25();
	if( value != FLT_MAX ) {
		if( !hasTempCorr ) {
			tempCorrection = pd->computeTempCorrection( pd->percentileprovider() );
			hasTempCorr = true;
		} 
	
		value += tempCorrection;;
		out << indent << "<probability type=\"exact\" parameter=\"temperature\""
		    << " percentile=\"25\" unit=\"celcius\" value=\"" << value << "\"/>\n";
	}
	
	value = pd->T2M_PERCENTILE_50(); 
	if( value != FLT_MAX ) {
		if( !hasTempCorr ) {
			tempCorrection = pd->computeTempCorrection( pd->percentileprovider() );
			hasTempCorr = true;
		} 

		value += tempCorrection;
		out << indent << "<probability type=\"exact\" parameter=\"temperature\""
		    << " percentile=\"50\" unit=\"celcius\" value=\"" << value << "\"/>\n";
	}
	
	value = pd->T2M_PERCENTILE_75(); 
	if( value != FLT_MAX ){
		if( !hasTempCorr ) {
			tempCorrection = pd->computeTempCorrection( pd->percentileprovider() );
			hasTempCorr = true;
		} 

		value += tempCorrection;
		out << indent << "<probability type=\"exact\" parameter=\"temperature\""
		    << " percentile=\"75\" unit=\"celcius\" value=\"" << value << "\"/>\n";
	}
	
	value = pd->T2M_PERCENTILE_90();
	if( pd->T2M_PERCENTILE_90() != FLT_MAX ){
		if( !hasTempCorr ) {
			tempCorrection = pd->computeTempCorrection( pd->percentileprovider() );
			hasTempCorr = true;
		} 

		value += tempCorrection;
		out << indent << "<probability type=\"exact\" parameter=\"temperature\""
		    << " percentile=\"90\" unit=\"celcius\" value=\"" << value << "\"/>\n";
	}
#endif	
}

}
