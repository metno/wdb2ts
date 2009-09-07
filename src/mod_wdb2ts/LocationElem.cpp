/*
    wdb - weather and water data storage

    Copyright (C) 2008 met.no

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

#include <iostream>
#include <float.h>
#include <limits.h>
#include <LocationElem.h>
#include <Logger4cpp.h>

namespace wdb2ts {

using namespace std;
using namespace boost::posix_time;

LocationElem::
LocationElem():
	topoTime( boost::gregorian::date(1970, 1, 1),
		   	 boost::posix_time::time_duration( 0, 0, 0 ) ),
	topoPostfix("__MODEL_TOPO__"),
	seaIceTime( boost::gregorian::date(1970, 1, 1),
			   	boost::posix_time::time_duration( 0, 0, 0 ) ),
	seaBottomTopographyTime( boost::gregorian::date(1970, 1, 1),
	   	                   boost::posix_time::time_duration( 0, 0, 0 ) ),
   timeSerie( 0 ),
   latitude_( FLT_MAX ), longitude_( FLT_MAX ),
	hight_( INT_MIN ), topoHight_( INT_MIN ),
	topoSearched(false)
{
}

LocationElem::
LocationElem( const ProviderList &providerPriority_,
				  const TopoProviderMap &modelTopoProviders_,
				float longitude, float latitude, int hight)
	: topoTime( boost::gregorian::date(1970, 1, 1),
	            boost::posix_time::time_duration( 0, 0, 0 ) ),
	  topoPostfix("__MODEL_TOPO__"),
	  seaIceTime( boost::gregorian::date(1970, 1, 1),
		           boost::posix_time::time_duration( 0, 0, 0 ) ),
	  seaBottomTopographyTime( boost::gregorian::date(1970, 1, 1),
		                        boost::posix_time::time_duration( 0, 0, 0 ) ),
	  timeSerie(0),
     providerPriority( providerPriority_ ),
     modelTopoProviders( modelTopoProviders_ ),
     latitude_( latitude ), longitude_( longitude ),
     hight_( hight ), topoHight_( INT_MIN ), topoSearched( false )
{
	
}

void
LocationElem::
init( CITimeSerie itTimeSerie, const TimeSerie *timeSerie )
{
	this->itTimeSerie = itTimeSerie;
	this->timeSerie = timeSerie;
	
	//forecastProvider.erase();
   //percentileProvider.erase();
	//precipRefTime = ptime();
}

const double HEIGHT_CORRECTION_PER_METER = 0.006;

float
LocationElem::
computeTempCorrection( const std::string &provider ) const
{
	
	//cerr << "HIGHT CORRECTION: Realight=" << hight_ << " provider: "<< provider<< endl;
	if( hight_ == INT_MIN )
		return 0.0;
		
	int modelTopo = modeltopography( provider );
	
	if( modelTopo == INT_MIN )
		return 0.0;
		
	int relTopo  = modelTopo -  hight_ ;
	
	//cerr << "HIGHT CORRECTION: Realight=" << hight_ << " modelTopo: "<< modelTopo << 
	//        " relTopo: " << relTopo << " corr: " << relTopo * 0.006 << endl;
	return relTopo * HEIGHT_CORRECTION_PER_METER;
}


float 
LocationElem::
windV10m(bool tryHard)const
{
	return getValue( &PData::windV10m,
						  itTimeSerie->second,
						  const_cast<ptime&>(itTimeSerie->first), 
			           const_cast<string&>(forecastProvider), FLT_MAX, tryHard );
}

float 
LocationElem::
windU10m(bool tryHard)const
{
	return getValue( &PData::windU10m, 
			           itTimeSerie->second,			 
			           const_cast<ptime&>(itTimeSerie->first), 
			           const_cast<string&>(forecastProvider), FLT_MAX, tryHard );
}

float 
LocationElem::
PP(bool tryHard)const
{
	return getValue( &PData::PP,
			           itTimeSerie->second,
			           const_cast<ptime&>(itTimeSerie->first), 
			           const_cast<string&>(forecastProvider) , FLT_MAX, tryHard );
}

float 
LocationElem::
PR(bool tryHard)const
{
	return getValue( &PData::PR,
			           itTimeSerie->second,
			           const_cast<ptime&>(itTimeSerie->first), 
						 const_cast<string&>(forecastProvider), FLT_MAX, tryHard );
}
   

float 
LocationElem::
TA(bool tryHard)const
{
	string providerT2m;
	
	float t2m = getValue( &PData::T2M,
			                itTimeSerie->second,
			                const_cast<ptime&>(itTimeSerie->first), 
			                const_cast<string&>(forecastProvider), FLT_MAX, tryHard );
	
	if( ! forecastProvider.empty() )
		providerT2m = forecastProvider;
	
	float t2m_land = getValue( &PData::T2M_LAND,
			                     itTimeSerie->second,
			                     const_cast<ptime&>(itTimeSerie->first), 
										const_cast<string&>(forecastProvider), FLT_MAX, tryHard );
	
	if( t2m_land != FLT_MAX )
		return t2m_land;
	
	const_cast<string&>(forecastProvider) = providerT2m;

	return t2m;
}

float 
LocationElem::
T2M(bool tryHard)const
{
	return getValue( &PData::T2M,
			           itTimeSerie->second,
			           const_cast<ptime&>(itTimeSerie->first), 
			           const_cast<string&>(forecastProvider), FLT_MAX, tryHard );
}

float
LocationElem::
T2M_LAND(bool tryHard)const
{
	return getValue( &PData::T2M_LAND,
			           itTimeSerie->second,
			           const_cast<ptime&>(itTimeSerie->first), 
			           const_cast<string&>(forecastProvider), FLT_MAX, tryHard );
}
   

float 
LocationElem::
UU(bool tryHard)const
{
	return getValue( &PData::UU,
			           itTimeSerie->second,
			           const_cast<ptime&>(itTimeSerie->first), 
			           const_cast<string&>(forecastProvider), FLT_MAX, tryHard );
}


float
LocationElem::
PRECIP( int hoursBack, boost::posix_time::ptime &backTime_, bool tryHard )const
{
	const float MIN_PRECIP = 0.1f;
	
	float precip;
	
	precip = PRECIP_N( hoursBack, backTime_, tryHard );
	
	if( precip == FLT_MAX )
		precip = PRECIP_1H( hoursBack, backTime_, tryHard );
	
	if( precip == FLT_MAX )
		return FLT_MAX;
	
   if ( (int) floorf( precip * 10 ) == (int)floorf((precip+0.05f)*10) )
      precip -= 0.0001; 

   if( precip <= MIN_PRECIP )
      precip = 0.0f;

	
	return  precip;
}

float 
LocationElem::
PRECIP_1H( int hoursBack, boost::posix_time::ptime &backTime_, bool tryHard )const
{
	float sumPrecip=0.0;
	float precipNow;
	int   count=0;
	boost::posix_time::ptime fromTime;
	boost::posix_time::ptime stopTime;
	boost::posix_time::ptime startTime;
	boost::posix_time::ptime savedFromTime;
	
	WEBFW_USE_LOGGER( "main" );

	backTime_=boost::posix_time::ptime(); //Set it to undefined.
	
	stopTime=itTimeSerie->first;
	
	//Find the start of the accumulation period.
	fromTime = itTimeSerie->first - hours( hoursBack );
	savedFromTime = fromTime;

	//We must adjust the startime with one hour because
	//the one hour precipitation is for the hour preceeding
	//the totime.
	startTime = fromTime + hours( 1 );
	
	WEBFW_LOG_DEBUG( "LocationElem::PRECIP_1H: hoursBack: " << hoursBack << " startTime: " << startTime << " stopTime: " << stopTime );
	for( CITimeSerie it = timeSerie->find( startTime );
	     it != timeSerie->end() && it->first <= stopTime;
	     ++it ) 
	{

		precipNow = getValue( &PData::PRECIP_1T, 
				                it->second,
				                fromTime , 
		                      const_cast<string&>(forecastProvider), FLT_MAX, tryHard );

		//WEBFW_LOG_DEBUG( "LocationElem::PRECIP_1H: loop: fromTime: " << fromTime << " toTime: " << it->first  << " precip: " << precipNow);
		fromTime = it->first;
		

		if( precipNow == FLT_MAX )
			return FLT_MAX;
		
		sumPrecip += precipNow;
		count++;
	}

	if( count == 0 ) {
		WEBFW_LOG_DEBUG( "LocationElem::PRECIP_1H: return sumPrecip: FLT_MAX" );
		return FLT_MAX;
	}

	backTime_ = savedFromTime;
	
	if( sumPrecip < 0 )
		sumPrecip = 0;
	
	WEBFW_LOG_DEBUG( "LocationElem::PRECIP_1H: return sumPrecip:" << sumPrecip );
	return sumPrecip;
}


float 
LocationElem::
PRECIP_N( int hoursBack, boost::posix_time::ptime &backTime_, bool tryHard )const
{
	float precipBack;
	float precipNow;
	float precip=FLT_MAX;
	ptime fromTime;
	boost::posix_time::ptime backTime;
	boost::posix_time::time_duration h;
	
	backTime_=boost::posix_time::ptime(); //Set it to undefined.
	
	backTime=itTimeSerie->first - hours( hoursBack );
	
	precipNow = getValue( &PData::PRECIP_ACCUMULATED, 
			                itTimeSerie->second,
			                fromTime , 
                         const_cast<string&>(forecastProvider), FLT_MAX, tryHard, true );

	if( precipNow == FLT_MAX || fromTime.is_special() )
		return FLT_MAX;

	h = itTimeSerie->first - fromTime;
	
	if( h.is_negative() ) 
		h.invert_sign();
	
	if( fromTime == itTimeSerie->first ) {
		//Accumulated precip with fromtime same as to time.
		//We must check if we have data for the same dataprovider
		//in the previous time in the dataset and with at time difference
		//equal to hoursBack.
		
		//cerr << "PRECIP_N: fromTime == toTime. from: " << fromTime << " backTime: " << backTime << endl;
		
		CITimeSerie itTmp = itTimeSerie;
		
		if( itTmp == timeSerie->begin() )
			return FLT_MAX;
		
		--itTmp;
		
		//cerr << "PRECIP_N: itTmp. totime: " << itTmp->first << endl;
		
		if( itTmp->first != backTime )
			return FLT_MAX;

		//We reset the fromTime to is_special so getValue 
	   //ignore the fromtime.
		fromTime = boost::posix_time::ptime();
		
		precipBack = getValue( &PData::PRECIP_ACCUMULATED,
					              itTmp->second,
					              fromTime, 
			                    const_cast<string&>(forecastProvider), FLT_MAX, tryHard );
		
		
		if( precipBack == FLT_MAX ) 
			return FLT_MAX;
		
		precip = precipNow;
	} else if( h.hours() == hoursBack ) {
		precip = precipNow;
   } else {
	   //cerr <<"PRECIP_N: ft: " << fromTime << "hb: " << hoursBack << " P: " << forecastProvider << endl;;

		CITimeSerie itTimeSerieBack=timeSerie->find( backTime );
		
		if( itTimeSerieBack == timeSerie->end() )
			return FLT_MAX;

		precipBack = getValue( &PData::PRECIP_ACCUMULATED,
				                 itTimeSerieBack->second,
			                    fromTime, 
	                          const_cast<string&>(forecastProvider), FLT_MAX, tryHard );

		if( precipBack == FLT_MAX ) 
		    return FLT_MAX;

		precip = precipNow - precipBack;
	}

	backTime_ = backTime;
	
	if( precip < 0 )
		precip = 0.0;
	
	//cerr << "PRECIP_N prec: " << precip << endl;
	return precip;
}


float
LocationElem::
NN(bool tryHard)const
{
	return getValue( &PData::NN,
			           itTimeSerie->second,
			           const_cast<ptime&>(itTimeSerie->first), 
			           const_cast<string&>(forecastProvider), FLT_MAX, tryHard );
}


float
LocationElem::
fog(bool tryHard)const
{
	return getValue( &PData::fog, 
			           itTimeSerie->second,
			           const_cast<ptime&>(itTimeSerie->first), 
						  const_cast<string&>(forecastProvider), FLT_MAX, tryHard );
}


float
LocationElem::
highCloud(bool tryHard)const
{
	return getValue( &PData::highCloud,
			           itTimeSerie->second,
			           const_cast<ptime&>(itTimeSerie->first), 
			           const_cast<string&>(forecastProvider), FLT_MAX, tryHard );
}


float 
LocationElem::
mediumCloud(bool tryHard)const
{
	return getValue( &PData::mediumCloud, 
			           itTimeSerie->second,
			           const_cast<ptime&>(itTimeSerie->first), 
		              const_cast<string&>(forecastProvider), FLT_MAX, tryHard );
}


float 
LocationElem::
lowCloud(bool tryHard)const
{
	return getValue( &PData::lowCloud,
			           itTimeSerie->second,
			           const_cast<ptime&>(itTimeSerie->first), 
			           const_cast<string&>(forecastProvider), FLT_MAX, tryHard );
}


float 
LocationElem::
RH2M(bool tryHard)const
{
	return getValue( &PData::RH2M,
			           itTimeSerie->second,
			           const_cast<ptime&>(itTimeSerie->first), 
			           const_cast<string&>(forecastProvider), FLT_MAX, tryHard );
}

float 
LocationElem::
thunderProbability( bool tryHard )const
{
	return getValue( &PData::thunderProability,
				        itTimeSerie->second,
				        const_cast<ptime&>(itTimeSerie->first), 
				        const_cast<string&>(forecastProvider), FLT_MAX, tryHard );
}

float 
LocationElem::
fogProbability( bool tryHard )const
{
	return getValue( &PData::fogProability,
					     itTimeSerie->second,
					     const_cast<ptime&>(itTimeSerie->first), 
					     const_cast<string&>(forecastProvider), FLT_MAX, tryHard );
}
 

float
LocationElem::
WIND_PROBABILITY( bool tryHard )const
{
	return getValue( &PData::WIND_PROBABILITY, 
			           itTimeSerie->second,
			           const_cast<ptime&>(itTimeSerie->first), 
			           const_cast<string&>(forecastProvider), FLT_MAX, tryHard );
}

float 
LocationElem::
T2M_PROBABILITY_1( bool tryHard )const
{
	return getValue( &PData::T2M_PROBABILITY_1, 
			           itTimeSerie->second,
			           const_cast<ptime&>(itTimeSerie->first), 
			           const_cast<string&>(forecastProvider), FLT_MAX, tryHard );
}

float 
LocationElem::
T2M_PROBABILITY_2( bool tryHard )const
{
	return getValue( &PData::T2M_PROBABILITY_2, 
			           itTimeSerie->second,
			           const_cast<ptime&>(itTimeSerie->first), 
			           const_cast<string&>(forecastProvider), FLT_MAX, tryHard );
}


float 
LocationElem::
T2M_PROBABILITY_3( bool tryHard )const
{
	return getValue( &PData::T2M_PROBABILITY_3, 
			           itTimeSerie->second,
			           const_cast<ptime&>(itTimeSerie->first), 
			           const_cast<string&>(forecastProvider), FLT_MAX, tryHard );
}

float 
LocationElem::
T2M_PROBABILITY_4( bool tryHard )const
{
	return getValue( &PData::T2M_PROBABILITY_4, 
			           itTimeSerie->second,
			           const_cast<ptime&>(itTimeSerie->first), 
			           const_cast<string&>(forecastProvider), FLT_MAX, tryHard );
}


float 
LocationElem::
T2M_PERCENTILE_10(bool tryHard)const
{
	return getValue( &PData::T2M_PERCENTILE_10, 
			           itTimeSerie->second,
			           const_cast<ptime&>(itTimeSerie->first), 
			           const_cast<string&>(percentileProvider), FLT_MAX, tryHard );
}

   
float 
LocationElem::
T2M_PERCENTILE_25(bool tryHard)const
{
	return getValue( &PData::T2M_PERCENTILE_25, 
			           itTimeSerie->second,           
			           const_cast<ptime&>(itTimeSerie->first), 
			           const_cast<string&>(percentileProvider), FLT_MAX, tryHard );
}

   
float 
LocationElem::
T2M_PERCENTILE_50(bool tryHard)const
{
	return getValue( &PData::T2M_PERCENTILE_50,
			           itTimeSerie->second,
			           const_cast<ptime&>(itTimeSerie->first), 
			           const_cast<string&>(percentileProvider), FLT_MAX, tryHard );
}


float 
LocationElem::
T2M_PERCENTILE_75(bool tryHard)const
{
	return getValue( &PData::T2M_PERCENTILE_75,
			           itTimeSerie->second,
			           const_cast<ptime&>(itTimeSerie->first), 
			           const_cast<string&>(percentileProvider), FLT_MAX, tryHard );
}


  
float 
LocationElem::
T2M_PERCENTILE_90(bool tryHard)const
{
	return getValue( &PData::T2M_PERCENTILE_90,
			           itTimeSerie->second,
			           const_cast<ptime&>(itTimeSerie->first), 
			           const_cast<string&>(percentileProvider), FLT_MAX, tryHard );
}

   
float 
LocationElem::
PRECIP_PERCENTILE_10( int hoursBack, bool tryHard)const
{
	ptime fromTime;
	
	fromTime=itTimeSerie->first - hours( hoursBack );
	
	return getValue( &PData::PRECIP_PERCENTILE_10, 
			           itTimeSerie->second,
			           fromTime, 
                    const_cast<string&>(percentileProvider), FLT_MAX, tryHard);
}


float 
LocationElem::
PRECIP_PERCENTILE_25( int hoursBack, bool tryHard)const
{
	ptime fromTime;
	
	fromTime=itTimeSerie->first - hours( hoursBack );

	return getValue( &PData::PRECIP_PERCENTILE_25, 
						  itTimeSerie->second, 
			           fromTime,
			           const_cast<string&>(percentileProvider), FLT_MAX, tryHard );
}


float 
LocationElem::
PRECIP_PERCENTILE_50( int hoursBack, bool tryHard)const
{
	ptime fromTime;
	
	fromTime=itTimeSerie->first - hours( hoursBack );

	return getValue( &PData::PRECIP_PERCENTILE_50, 
						  itTimeSerie->second, 
			           fromTime,
			           const_cast<string&>(percentileProvider), FLT_MAX, tryHard );
}


float 
LocationElem::
PRECIP_PERCENTILE_75( int hoursBack, bool tryHard)const
{
	ptime fromTime;
	
	fromTime=itTimeSerie->first - hours( hoursBack );

	return getValue( &PData::PRECIP_PERCENTILE_75, 
						  itTimeSerie->second, 
			           fromTime,
			           const_cast<string&>(percentileProvider), FLT_MAX, tryHard );
}

   
float 
LocationElem::
PRECIP_PERCENTILE_90( int hoursBack, bool tryHard)const
{
	ptime fromTime;
	
	fromTime=itTimeSerie->first - hours( hoursBack );

	return getValue( &PData::PRECIP_PERCENTILE_90, 
						  itTimeSerie->second, 
			           fromTime,
			           const_cast<string&>(percentileProvider), FLT_MAX, tryHard );
}


float 
LocationElem::
PRECIP_PROBABILITY_0_1_MM( int hoursBack, bool tryHard )const
{
	ptime fromTime;
	
	fromTime=itTimeSerie->first - hours( hoursBack );

	return getValue( &PData::PRECIP_PROBABILITY_0_1MM,
			           itTimeSerie->second,
			           fromTime, 
			           const_cast<string&>(percentileProvider), FLT_MAX, tryHard );
}

   
float 
LocationElem::
PRECIP_PROBABILITY_0_2_MM( int hoursBack, bool tryHard)const
{
	ptime fromTime;
	
	fromTime=itTimeSerie->first - hours( hoursBack );

	return getValue( &PData::PRECIP_PROBABILITY_0_2MM,
			           itTimeSerie->second,
			           fromTime, 
			           const_cast<string&>(percentileProvider), FLT_MAX, tryHard );
}


float 
LocationElem::
PRECIP_PROBABILITY_0_5_MM( int hoursBack, bool tryHard)const
{
	ptime fromTime;
	
	fromTime=itTimeSerie->first - hours( hoursBack );

	return getValue( &PData::PRECIP_PROBABILITY_0_5MM,
			           itTimeSerie->second,
			           fromTime, 
			           const_cast<string&>(percentileProvider), FLT_MAX, tryHard );
}

   
float 
LocationElem::
PRECIP_PROBABILITY_1_0_MM( int hoursBack, bool tryHard)const
{
	ptime fromTime;
	
	fromTime=itTimeSerie->first - hours( hoursBack );

	return getValue( &PData::PRECIP_PROBABILITY_1_0MM,
			           itTimeSerie->second,
			           fromTime, 
			           const_cast<string&>(percentileProvider), FLT_MAX, tryHard );
}

   
float
LocationElem::
PRECIP_PROBABILITY_2_0_MM( int hoursBack, bool tryHard)const
{
	ptime fromTime;
	
	fromTime=itTimeSerie->first - hours( hoursBack );

	return getValue( &PData::PRECIP_PROBABILITY_2_0MM,
			           itTimeSerie->second,
			           fromTime, 
			           const_cast<string&>(percentileProvider), FLT_MAX, tryHard );
}


float 
LocationElem::
PRECIP_PROBABILITY_5_0_MM( int hoursBack, bool tryHard)const
{
	ptime fromTime;
	
	fromTime=itTimeSerie->first - hours( hoursBack );

	return getValue( &PData::PRECIP_PROBABILITY_5_0MM,
						  itTimeSerie->second,
						  fromTime, 
			           const_cast<string&>(percentileProvider), FLT_MAX, tryHard );
}


   
float 
LocationElem::
symbol_PROBABILITY( boost::posix_time::ptime &fromTime,bool tryHard )const
{                                   
	return   getValue( &PData::symbol_PROBABILITY,
			             itTimeSerie->second,
			             fromTime,
			             const_cast<string&>(symbolProvider), FLT_MAX, tryHard );	
}

int   
LocationElem::
symbol( boost::posix_time::ptime &fromTime, bool tryHard)const
{
	string provider(symbolProvider);
	
	fromTime = ptime(); //Set to is_special
	
	if( provider.empty() )
		provider = forecastProvider;
	
	float s = getValue( &PData::symbol,
			              itTimeSerie->second,
			              fromTime,
			              provider, FLT_MAX, tryHard );
	
	if( s == FLT_MAX )
		return INT_MAX;
	
	const_cast<string&>( symbolProvider ) = provider;
	return int( s + 0.5 );
}

float 
LocationElem::
seaCurrentVelocityU( bool tryHard )const
{
	return getValue( &PData::seaCurrentVelocityU, 
			           itTimeSerie->second,
			           const_cast<ptime&>(itTimeSerie->first), 
			           const_cast<string&>(forecastProvider), FLT_MAX, tryHard );
}

float 
LocationElem::
seaCurrentVelocityV( bool tryHard )const
{
	return getValue( &PData::seaCurrentVelocityV, 
			           itTimeSerie->second,
			           const_cast<ptime&>(itTimeSerie->first), 
			           const_cast<string&>(forecastProvider), FLT_MAX, tryHard );
}

float 
LocationElem::
seaSalinity( bool tryHard )const
{
	return getValue( &PData::seaSalinity, 
			           itTimeSerie->second,
			           const_cast<ptime&>(itTimeSerie->first), 
			           const_cast<string&>(forecastProvider), FLT_MAX, tryHard );
}

float 
LocationElem::
seaSurfaceHeight( bool tryHard )const
{
	return getValue( &PData::seaSurfaceHeight, 
			                itTimeSerie->second,
			                const_cast<ptime&>(itTimeSerie->first), 
						       const_cast<string&>(forecastProvider), FLT_MAX, tryHard );
}

float 
LocationElem::
seaTemperature( bool tryHard )const
{
	return getValue( &PData::seaTemperature, 
			           itTimeSerie->second,
			           const_cast<ptime&>(itTimeSerie->first), 
			           const_cast<string&>(forecastProvider), FLT_MAX, tryHard );
}
  
float 
LocationElem::
meanTotalWaveDirection( bool tryHard )const
{
	return getValue( &PData::meanTotalWaveDirection, 
			           itTimeSerie->second,
			           const_cast<ptime&>(itTimeSerie->first), 
			           const_cast<string&>(forecastProvider), FLT_MAX, tryHard );
}

float 
LocationElem::
significantTotalWaveHeight( bool tryHard )const
{
	return getValue( &PData::significantTotalWaveHeight, 
			           itTimeSerie->second,
			           const_cast<ptime&>(itTimeSerie->first), 
			           const_cast<string&>(forecastProvider), FLT_MAX, tryHard );
}

int   
LocationElem::
seaBottomTopography( std::string &usedProvider, const std::string &useProvider )const
{
	float retVal=FLT_MAX;
	int count=providerPriority.size();
	
	ProviderList::const_iterator it=providerPriority.begin();
	
	if( ! useProvider.empty() ) {
		it = providerPriority.findProvider( useProvider );
		count=1;
	}

	for( ; it != providerPriority.end() && count>0; ++it, count-- )
	{
		TimeSerie::const_iterator it1=timeSerie->find( seaBottomTopographyTime );
	
		if( it1 == timeSerie->end() )
			continue;
	
		FromTimeSerie::const_iterator it2=it1->second.find( seaBottomTopographyTime );
	
		if( it2 == it1->second.end() )
			continue;
	
		usedProvider = it->providerWithPlacename();
		ProviderPDataList::const_iterator it3=it2->second.find( usedProvider );
	
		if( it3 == it2->second.end() )
			continue;
	
		retVal = it3->second.seaBottomTopography;
		
		if( retVal == FLT_MAX )
			continue;
		
		break;
	}
	
	if( retVal == FLT_MAX )
		return INT_MAX;
	
	return static_cast<int>( retVal );
}
	

int 
LocationElem::
seaIcePresence( std::string &usedProvider, const std::string &useProvider )const
{
	float retVal=FLT_MAX;
	int count=providerPriority.size();
		
	ProviderList::const_iterator it=providerPriority.begin();
		
	if( ! useProvider.empty() ) {
		it = providerPriority.findProvider( useProvider );
		count=1;
	}

	for( ; it != providerPriority.end() && count>0; ++it, count-- )
	{
		TimeSerie::const_iterator it1=timeSerie->find( seaIceTime );
		
		if( it1 == timeSerie->end() )
			continue;
		
		FromTimeSerie::const_iterator it2=it1->second.find( seaIceTime );
		
		if( it2 == it1->second.end() )
			continue;
		
		usedProvider = it->providerWithPlacename();
		ProviderPDataList::const_iterator it3=it2->second.find( usedProvider );
		
		if( it3 == it2->second.end() )
			continue;
		
		retVal = it3->second.seaIcePresence;
			
		if( retVal == FLT_MAX )
			continue;
			
		break;
	}
		
	if( retVal == FLT_MAX )
		return INT_MAX;
		
	return static_cast<int>( retVal );
}

int
LocationElem::
modeltopography( const std::string &provider_ )const
{
	string provider( provider_ );
	TopoProviderMap::const_iterator it=modelTopoProviders.find( provider_ );
	
	if( it != modelTopoProviders.end() )
		provider = it->second;
	
	provider += topoPostfix;
	
	//cerr << "TopoGraphy:  topoTime: " << topoTime << " provider: " << provider << endl;
	

	TimeSerie::const_iterator it1=timeSerie->find( topoTime );
	
	if( it1 == timeSerie->end() )
		return INT_MIN;
	
	FromTimeSerie::const_iterator it2=it1->second.find( topoTime );
	
	if( it2 == it1->second.end() )
		return INT_MIN;
	
	ProviderPDataList::const_iterator it3=it2->second.find( provider );
	
	if( it3 == it2->second.end() )
		return INT_MIN;
	
	if( it3->second.modeltopography == FLT_MAX )
		return INT_MIN;
	
	return static_cast<int>( it3->second.modeltopography );

}

}

