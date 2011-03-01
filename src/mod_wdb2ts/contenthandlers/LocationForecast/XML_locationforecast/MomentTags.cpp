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
#include <iostream>
#include <sstream>
#include <MomentTags.h>
#include <probabilityCode.h>
#include <Logger4cpp.h>

namespace wdb2ts {

using namespace std;
void 
MomentTags::
init( LocationElem &pointData )
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
MomentTags::
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

float 
MomentTags::
getTemperatureProability( float temp, bool tryHard )const
{
	float prob;
	std::string provider = pd->forecastprovider();

	if( temp > -99.0 && temp < -10.0 )
		prob = pd->T2M_PROBABILITY_2( tryHard );
	else if( temp >= -10.0 && temp < 100.0 ) 
		prob = pd->T2M_PROBABILITY_1( tryHard );
	else
		prob = FLT_MAX;
	
	pd->forecastprovider( provider );

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
MomentTags::
doSymbol( float temperature )
{
   WEBFW_USE_LOGGER( "encode" );
	boost::posix_time::ptime fromTime; 
	
	int symNumber = pd->symbol( fromTime );
	
	//cerr << "doSymbol: "  << symNumber << endl;
	
	if( symNumber == INT_MAX ) {
	   WEBFW_LOG_DEBUG("doSymbol: NO SYMBOL for fromtime: " << fromTime );
	   cerr << "doSymbol: NO SYMBOL for fromtime: " << fromTime << " Provider: " << pd->forecastprovider()
	        <<endl;
		return;
	}

	//cerr << "doSymbol: AddSymbol: " << symNumber << " (" << pd->symbol_PROBABILITY() << ") t: "
	//     << pd->time() << " p: " << pd->forecastprovider() << " lat: "  << pd->latitude()
	//     << endl;

	//Allow the symbol probability to came from another provider than the symbol.
	ostringstream o;

	string savedProvider = pd->forecastprovider();

	o << "doSymbol: savedProvider: '" << savedProvider << "' fromTime: " << fromTime;
	float prob = pd->symbol_PROBABILITY( fromTime, true );

	o << " symbol: " << symNumber << " prob: " << prob << " provider: '" << pd->forecastprovider()
	  << "' time: " << pd->time() << " temperature: " << temperature;
	symbolContext->addSymbol( symNumber, prob,
			                    pd->time(), temperature, fromTime, pd->forecastprovider(), pd->latitude() );

	//Add the symbol probability to the saved forecast provider in addition to
	//the provider for probability.
	if( savedProvider !=  pd->forecastprovider() )
	   symbolContext->addSymbol( symNumber, prob,
	                             pd->time(), temperature, fromTime, savedProvider, pd->latitude() );
	pd->forecastprovider( savedProvider );
	cerr << o.str() << endl;
	WEBFW_LOG_DEBUG( o.str() );
}

void 
MomentTags::
output( std::ostream &out, const std::string &indent ) 
{
	WEBFW_USE_LOGGER( "encode" );
	log4cpp::Priority::Value loglevel = WEBFW_GET_LOGLEVEL();
	ostringstream tmpout;
	string description;
	float value;
	float tempNoAdiabatic;
	float tempAdiabatic;
	float tempUsed=FLT_MAX;
	float dd_, ff_;
	string provider;
	string tempNoAdiabaticProvider;
	float tempCorrection;
	float tempProb = FLT_MAX;
	float windProb = FLT_MAX;
	bool hasTempCorr;
	int oldPrec = out.precision();
	int modelTopo;
	int nForecast=0;
	int relTopo; //The difference between modelTopo and real topography

	if( ! pd )
		return;
	
	out.precision(1);
	tmpout.flags( out.flags() );
	tmpout.precision(1);
	
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

		if( loglevel >= log4cpp::Priority::DEBUG )
			tmpout << indent << "<!-- Dataprovider: " << provider << " tempAdiabatic: " << tempAdiabatic
			       << " tempAdiabatic: " << value << " (uncorrected) modelTopo: " << modelTopo << " relTopo: " << relTopo << " -->\n";
	}

	if( tempNoAdiabatic != FLT_MAX ) {
		tempUsed = tempNoAdiabatic;
	} else 	if( tempAdiabatic != FLT_MAX ) {
		tempUsed = tempAdiabatic;
	}

	if( tempUsed != FLT_MAX )
	   tempProb = getTemperatureProability( tempUsed, true );

	if( tempUsed != FLT_MAX ) {
		if( loglevel >= log4cpp::Priority::DEBUG ) {
			if( tempNoAdiabaticProvider.empty())
				tmpout << indent << "<!-- Dataprovider: " << provider << " -->\n";
			else
				tmpout << indent << "<!-- Dataprovider: " << tempNoAdiabaticProvider << " -->\n";

			if( tempNoAdiabatic != FLT_MAX && tempAdiabatic!=FLT_MAX ) {
				tmpout << indent << "<!-- Dataprovider: " << provider << " tempAdiabatic: " << tempAdiabatic
					   << " tempNoAdiabatic: " << tempNoAdiabatic << " " <<  tempNoAdiabaticProvider << " -->\n";
			}
		}

		//doSymbol( tempUsed );
		nForecast++;
		tmpout << indent << "<temperature id=\"TTT\" unit=\"celcius\" value=\""<< tempUsed << "\"/>\n";
	}

	//If we only have a temperature for the no "height correction" choice we need
	//another parameter to lock into the provider to use. We use the U component to the wind.
	if( provider.empty() && tempNoAdiabatic != FLT_MAX  ) {
		float tmp = pd->windU10m( true );

		if( tmp != FLT_MAX )
			provider = pd->forecastprovider();
	}
	
	if( ! provider.empty() ) {
		pd->temperatureCorrected( tempUsed, provider );
		computeWind( pd->windU10m(true), pd->windV10m() );
		projectionHelper->convertToDirectionAndLength( provider, pd->latitude(), pd->longitude(),
		   	                                           pd->windU10m(), pd->windV10m(),
			                                           dd_, ff_ );
		if( dd!=FLT_MAX && ff != FLT_MAX ) {
			if( loglevel >= log4cpp::Priority::DEBUG )
				tmpout << indent << "<!-- dd: " << dd << " ff: " << ff << " -->\n";

			tmpout << indent
			    << "<windDirection id=\"dd\" deg=\""<< dd_ << "\" " 
			    << "name=\"" << windDirectionName(dd_ ) <<"\"/>\n"
			    << indent 
			    << "<windSpeed id=\"ff\" mps=\""<< ff_ << "\" " 
			    << "beaufort=\"" << toBeaufort( ff_, description) << "\" "
			    << "name=\"" << description << "\"/>\n";
		
			windProb = pd->WIND_PROBABILITY( true );
			pd->forecastprovider( provider ); //Reset the forecastprovider back to provider.
			nForecast++;
		}

		pd->temperatureCorrected( tempUsed, provider );
		value = pd->RH2M( );
		if (value != FLT_MAX) {
			tmpout << indent << "<humidity value=\"" << value << "\" unit=\"percent\"/>\n";
			nForecast++;
		}

		value = pd->PR( );
		if( value != FLT_MAX ) {
			tmpout << indent << "<pressure id=\"pr\" unit=\"hPa\" value=\""<< value << "\"/>\n";
			nForecast++;
		}
	
		value = pd->NN(  );
		if( value != FLT_MAX ) {
			tmpout << indent << "<cloudiness id=\"NN\" percent=\"" << value << "\"/>\n";
			nForecast++;
		}

		value = pd->fog();
		if( value != FLT_MAX ) {
			tmpout << indent <<"<fog id=\"FOG\" percent=\"" << value << "\"/>\n";
			nForecast++;
		}

		value = pd->lowCloud();
		if( value != FLT_MAX ){
			tmpout << indent << "<lowClouds id=\"LOW\" percent=\"" << value << "\"/>\n";
			nForecast++;
		}

		value = pd->mediumCloud();
		if( value !=FLT_MAX ) {
			tmpout << indent << "<mediumClouds id=\"MEDIUM\" percent=\"" << value << "\"/>\n";
			nForecast++;
		}

		value = pd->highCloud(  );
		if( value != FLT_MAX ) {
			tmpout << indent << "<highClouds id=\"HIGH\" percent=\"" << value << "\"/>\n";
			nForecast++;
		}
	
		tmpout.precision( 0 );
	
		if( tempProb != FLT_MAX )
			tmpout << indent << "<temperatureProbability unit=\"probabilitycode\" value=\"" << probabilityCode( tempProb )<< "\"/>\n";
	
		if( windProb != FLT_MAX )
			tmpout << indent << "<windProbability unit=\"probabilitycode\" value=\"" << probabilityCode( windProb ) << "\"/>\n";
		
		out.precision( 1 );

		//must have at least 2 elements.
		if( nForecast > 1 )
			out << tmpout.str();
	}
	
	hasTempCorr = false;
	tempCorrection = 0.0;
	value = pd->T2M_PERCENTILE_10( true );
	if( value != FLT_MAX ) {
		if( !hasTempCorr ) {
			tempCorrection = pd->computeTempCorrection( pd->percentileprovider(), relTopo, modelTopo );
			hasTempCorr = true;
		} 
		
		value += tempCorrection;	
		out << indent << "<probability type=\"exact\" parameter=\"temperature\""
		    << " percentile=\"10\" unit=\"celcius\" value=\"" << value << "\"/>\n";
	}
	
	value = pd->T2M_PERCENTILE_25();
	if( value != FLT_MAX ) {
		if( !hasTempCorr ) {
			tempCorrection = pd->computeTempCorrection( pd->percentileprovider(), relTopo, modelTopo );
			hasTempCorr = true;
		} 
	
		value += tempCorrection;;
		out << indent << "<probability type=\"exact\" parameter=\"temperature\""
		    << " percentile=\"25\" unit=\"celcius\" value=\"" << value << "\"/>\n";
	}
	
	value = pd->T2M_PERCENTILE_50(); 
	if( value != FLT_MAX ) {
		if( !hasTempCorr ) {
			tempCorrection = pd->computeTempCorrection( pd->percentileprovider(), relTopo, modelTopo );
			hasTempCorr = true;
		} 

		value += tempCorrection;
		out << indent << "<probability type=\"exact\" parameter=\"temperature\""
		    << " percentile=\"50\" unit=\"celcius\" value=\"" << value << "\"/>\n";
	}
	
	value = pd->T2M_PERCENTILE_75(); 
	if( value != FLT_MAX ){
		if( !hasTempCorr ) {
			tempCorrection = pd->computeTempCorrection( pd->percentileprovider(), relTopo, modelTopo );
			hasTempCorr = true;
		} 

		value += tempCorrection;
		out << indent << "<probability type=\"exact\" parameter=\"temperature\""
		    << " percentile=\"75\" unit=\"celcius\" value=\"" << value << "\"/>\n";
	}
	
	value = pd->T2M_PERCENTILE_90();
	if( pd->T2M_PERCENTILE_90() != FLT_MAX ){
		if( !hasTempCorr ) {
			tempCorrection = pd->computeTempCorrection( pd->percentileprovider(), relTopo, modelTopo );
			hasTempCorr = true;
		} 

		value += tempCorrection;
		out << indent << "<probability type=\"exact\" parameter=\"temperature\""
		    << " percentile=\"90\" unit=\"celcius\" value=\"" << value << "\"/>\n";
	}
	
	out.precision( oldPrec );
}

}
