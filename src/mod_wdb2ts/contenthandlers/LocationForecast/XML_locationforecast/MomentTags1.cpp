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
#include <MomentTags1.h>
#include <probabilityCode.h>
#include <Logger4cpp.h>
#include <metfunctions.h>

namespace wdb2ts {

using namespace std;
void 
MomentTags1::
init( LocationElem &pointData )
{
	pd = &pointData;
}
		

float 
MomentTags1::
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

	return prob;
}
	


void 
MomentTags1::
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
	float dd, ff;
	string savedProvider;
	string provider;
	string tempNoAdiabaticProvider;
	float tempCorrection;
	float tempProb = FLT_MAX;
	float windProb = FLT_MAX;
	bool hasTempCorr;
	int oldPrec = out.precision();
	int modelTopo;
	int nForecast=0;
	float dewpoint=FLT_MAX;
	int relTopo; //The difference between modelTopo and real topography

	if( ! pd ) {
		WEBFW_LOG_DEBUG("MomentTags: No data!");
		return;
	}
	

	if( ! pd->forecastprovider().empty() )
		savedProvider = pd->forecastprovider();

	out.precision(1);
	tmpout.setf( ios::fixed, ios::floatfield );
	tmpout.precision(1);
	
	/*
	 * Coding of temperature.
	 * The temperature may or may not be corrected for height. If the
	 * temperature is corrected the difference between the model height and
	 * real height is used to do a adiabatic height correction.
	 *
	 * We decide if we shall do a height correction or not on the available
	 * temperatures. If the parameter pd->T2M_NO_ADIABATIC_HIGHT_CORRECTION
	 * has a value we do no height correction. ie. the no
	 * height correction has priority.
	 */

	tempNoAdiabatic = pd->T2M_NO_ADIABATIC_HIGHT_CORRECTION( true );

	if( tempNoAdiabatic != FLT_MAX )
		tempNoAdiabaticProvider = pd->forecastprovider();

	//Reset the provider back to the provider before the call to
	//pd->T2M_NO_ADIABATIC_HIGHT_CORRECTION( true );
	pd->forecastprovider( savedProvider );

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

		nForecast++;
		tmpout << indent << "<temperature id=\"TTT\" unit=\"celsius\" value=\""<< tempUsed << "\"/>\n";
	}

	//If we only have a temperature for the no "height correction" choice we need
	//another parameter to lock into the provider to use. We use the U component to the wind.
	if( provider.empty() && tempNoAdiabatic != FLT_MAX  ) {
		float tmp = pd->windU10m( true );

		if( tmp != FLT_MAX )
			provider = pd->forecastprovider();
	}
	
	if( provider.empty() && pd->config ) {
		provider = pd->config->requestedProvider;

		if( loglevel >= log4cpp::Priority::DEBUG ) {
			tmpout << indent << "<!-- Using requested dataprovider: " << provider << " -->\n";
		}
	}

	if( ! provider.empty() ) {
		symData->provider = provider;
		symData->temperature = tempUsed;

		pd->temperatureCorrected( tempUsed, provider );

		symData->thunderProbability = pd->thunderProbability();
		symData->wetBulbTemperature = pd->wetBulbTemperature();

		if( projectionHelper ) {
			if( projectionHelper->convertToDirectionAndLengthMiLib( provider, pd->latitude(), pd->longitude(),
						                                            pd->windU10m(), pd->windV10m(),
			                                                        dd, ff ) ) {
				if( loglevel >= log4cpp::Priority::DEBUG ) {
					tmpout << "<!-- libmi reprojection: dd: " << dd << " ff: " << ff << " -->\n";
				}
			}
			projectionHelper->convertToDirectionAndLength( provider, pd->latitude(), pd->longitude(),
								                           pd->windU10m(), pd->windV10m(),
					                                       dd, ff );

			if( dd!=FLT_MAX && ff != FLT_MAX ) {
				tmpout << indent
				       << "<windDirection id=\"dd\" deg=\""<< dd << "\" "
				       << "name=\"" << windDirectionName(dd ) <<"\"/>\n"
				       << indent
				       << "<windSpeed id=\"ff\" mps=\""<< ff << "\" "
				       << "beaufort=\"" << toBeaufort( ff, description) << "\" "
				       << "name=\"" << description << "\"/>\n";
		
				windProb = pd->WIND_PROBABILITY( true );
				pd->forecastprovider( provider ); //Reset the forecastprovider back to provider.
				nForecast++;
			}
		}

		dewpoint = pd->dewPointTemperature( true );

		value = pd->RH2M( true );

		if( value == FLT_MAX && dewpoint != FLT_MAX) {
		   if( dewpoint != FLT_MAX )
		      value = miutil::dewPointTemperatureToRelativeHumidity( tempUsed, dewpoint );
		}

		if (value != FLT_MAX) {
			tmpout << indent << "<humidity value=\"" << value << "\" unit=\"percent\"/>\n";
			nForecast++;
		}

		value = pd->PR( true );
		if( value != FLT_MAX ) {
			tmpout << indent << "<pressure id=\"pr\" unit=\"hPa\" value=\""<< value << "\"/>\n";
			nForecast++;
		}
	
		value = pd->NN(  true );
		if( value != FLT_MAX ) {
			symData->totalCloudCover = value;
			tmpout << indent << "<cloudiness id=\"NN\" percent=\"" << value << "\"/>\n";
			nForecast++;
		}

		value = pd->fog();
		if( value != FLT_MAX ) {
			symData->fogCover = value;
			tmpout << indent <<"<fog id=\"FOG\" percent=\"" << value << "\"/>\n";
			nForecast++;
		}

		value = pd->lowCloud();
		if( value != FLT_MAX ){
			symData->lowCloudCover = value;
			tmpout << indent << "<lowClouds id=\"LOW\" percent=\"" << value << "\"/>\n";
			nForecast++;
		}

		value = pd->mediumCloud();
		if( value !=FLT_MAX ) {
			symData->mediumCloudCover = value;
			tmpout << indent << "<mediumClouds id=\"MEDIUM\" percent=\"" << value << "\"/>\n";
			nForecast++;
		}

		value = pd->highCloud(  );
		if( value != FLT_MAX ) {
			symData->highCloudCover = value;
			tmpout << indent << "<highClouds id=\"HIGH\" percent=\"" << value << "\"/>\n";
			nForecast++;
		}

		tmpout.precision( 0 );
	
		if( tempProb != FLT_MAX )
			tmpout << indent << "<temperatureProbability unit=\"probabilitycode\" value=\"" << probabilityCode( tempProb )<< "\"/>\n";
	
		if( windProb != FLT_MAX )
			tmpout << indent << "<windProbability unit=\"probabilitycode\" value=\"" << probabilityCode( windProb ) << "\"/>\n";

		tmpout.precision( 1 );

		if( pd->config && pd->config->outputParam("seaiceingindex") ) {
		   value = pd->iceingIndex( false, pd->forecastprovider() );
		   if( value != FLT_MAX )
		      tmpout << indent << "<seaIceingIndex value=\"" << value << "\"/>\n";
		}

		if( pd->config && pd->config->outputParam( "dewpointTemperature") ) {
		   float myDewPoint= dewpoint;

		   if( myDewPoint == FLT_MAX ){
		       //myDewPoint = miutil::dewPointTemperature( pd->T2M(), pd->RH2M() );
			   myDewPoint = miutil::dewPointTemperature( tempUsed, pd->RH2M( true ) );
		   }

		   if( myDewPoint != FLT_MAX )
		      tmpout << indent << "<dewpointTemperature id=\"TD\" unit=\"celsius\" value=\""<< myDewPoint << "\"/>\n";

		}

	//	WEBFW_LOG_DEBUG("MomentTags: " << nForecast << " element encoded!");
		
		out.precision( 1 );

		//must have at least 1 elements.
		if( nForecast > 0 )
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
		    << " percentile=\"10\" unit=\"celsius\" value=\"" << value << "\"/>\n";
	}
	
	value = pd->T2M_PERCENTILE_25();
	if( value != FLT_MAX ) {
		if( !hasTempCorr ) {
			tempCorrection = pd->computeTempCorrection( pd->percentileprovider(), relTopo, modelTopo );
			hasTempCorr = true;
		} 
	
		value += tempCorrection;;
		out << indent << "<probability type=\"exact\" parameter=\"temperature\""
		    << " percentile=\"25\" unit=\"celsius\" value=\"" << value << "\"/>\n";
	}
	
	value = pd->T2M_PERCENTILE_50(); 
	if( value != FLT_MAX ) {
		if( !hasTempCorr ) {
			tempCorrection = pd->computeTempCorrection( pd->percentileprovider(), relTopo, modelTopo );
			hasTempCorr = true;
		} 

		value += tempCorrection;
		out << indent << "<probability type=\"exact\" parameter=\"temperature\""
		    << " percentile=\"50\" unit=\"celsius\" value=\"" << value << "\"/>\n";
	}
	
	value = pd->T2M_PERCENTILE_75(); 
	if( value != FLT_MAX ){
		if( !hasTempCorr ) {
			tempCorrection = pd->computeTempCorrection( pd->percentileprovider(), relTopo, modelTopo );
			hasTempCorr = true;
		} 

		value += tempCorrection;
		out << indent << "<probability type=\"exact\" parameter=\"temperature\""
		    << " percentile=\"75\" unit=\"celsius\" value=\"" << value << "\"/>\n";
	}
	
	value = pd->T2M_PERCENTILE_90();
	if( pd->T2M_PERCENTILE_90() != FLT_MAX ){
		if( !hasTempCorr ) {
			tempCorrection = pd->computeTempCorrection( pd->percentileprovider(), relTopo, modelTopo );
			hasTempCorr = true;
		} 

		value += tempCorrection;
		out << indent << "<probability type=\"exact\" parameter=\"temperature\""
		    << " percentile=\"90\" unit=\"celsius\" value=\"" << value << "\"/>\n";
	}
	
	out.precision( oldPrec );
}

}
