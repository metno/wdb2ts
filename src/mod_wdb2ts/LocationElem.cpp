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

#include <math.h>
#include <algorithm>
#include <vector>
#include <iostream>
#include <float.h>
#include <limits.h>
#include <LocationElem.h>
#include <Logger4cpp.h>
#include <metfunctions.h>

namespace {
   float sumVector( const std::vector<float> &v ) {
      float sum=0.0;
      for( std::vector<float>::const_iterator it = v.begin(); it != v.end(); ++it ) {
         if( *it == FLT_MAX )
            return FLT_MAX;
         sum += *it;
      }

      return sum;
   }

   struct cmp {
     cmp(){}
     bool operator()( const std::vector<float>::value_type &a, const std::vector<float>::value_type &b ) {
       return b < a;
     }
   };


   bool sortVector( std::vector<float> &v ) {
      for( std::vector<float>::const_iterator it = v.begin(); it != v.end(); ++it ) {
         if( *it == FLT_MAX )
            return false;
      }

      std::sort( v.begin(), v.end(), cmp()  );
      return true;
   }

   float maxInVector( std::vector<float> &v ) {
      float ret=FLT_MIN;
      for( std::vector<float>::const_iterator it = v.begin(); it != v.end(); ++it ) {
         if( *it > ret )
            ret = *it;
      }
      return ret;
   }

   float minInVector( std::vector<float> &v ) {
         float ret=FLT_MAX;
         for( std::vector<float>::const_iterator it = v.begin(); it != v.end(); ++it ) {
            if( *it < ret )
               ret = *it;
         }
         return ret;
      }

   struct RestoreForecastProvider
	{
   	std::string provider_;
   	wdb2ts::LocationElem *l_;
   	RestoreForecastProvider(wdb2ts::LocationElem *l )
   	: l_( l ), provider_( l->forecastprovider()){}
   	~RestoreForecastProvider() { l_->forecastprovider( provider_ );}
	};
}

namespace wdb2ts {

using namespace std;
using namespace boost::posix_time;

LocationElem::
LocationElem():
	thunder( this ),
	topoTime( boost::gregorian::date(1970, 1, 1),
		   	 boost::posix_time::time_duration( 0, 0, 0 ) ),
	topoPostfix("__MODEL_TOPO__"), topographyPostfix( "__TOPOGRAPHY__" ),
	seaIceTime( boost::gregorian::date(1970, 1, 1),
			   	boost::posix_time::time_duration( 0, 0, 0 ) ),
	seaBottomTopographyTime( boost::gregorian::date(1970, 1, 1),
	   	                   boost::posix_time::time_duration( 0, 0, 0 ) ),
	epoch( boost::gregorian::date(1970, 1, 1),
	   	 boost::posix_time::time_duration( 0, 0, 0 ) ),
   timeSerie( 0 ),
   latitude_( FLT_MAX ), longitude_( FLT_MAX ),
   height_( INT_MIN ), topoHeight_( INT_MIN ),
   modeltopoSearched(false),
   config( new ConfigData() )
{
	itProviderPriorityBegin = providerPriority.begin();
}

LocationElem::
LocationElem( const ProviderList &providerPriority_,
			  const TopoProviderMap &modelTopoProviders_,
			  const std::list<std::string>  &topographyProviders_,
			  float longitude, float latitude, int hight)
	: thunder( this ),
	  topoTime( boost::gregorian::date(1970, 1, 1),
	            boost::posix_time::time_duration( 0, 0, 0 ) ),
	  topoPostfix("__MODEL_TOPO__"), topographyPostfix( "__TOPOGRAPHY__" ),
	  seaIceTime( boost::gregorian::date(1970, 1, 1),
		           boost::posix_time::time_duration( 0, 0, 0 ) ),
	  seaBottomTopographyTime( boost::gregorian::date(1970, 1, 1),
		                        boost::posix_time::time_duration( 0, 0, 0 ) ),
	  timeSerie(0),
     providerPriority( providerPriority_ ),
     modelTopoProviders( modelTopoProviders_ ),
     topographyProviders( topographyProviders_ ),
     latitude_( latitude ), longitude_( longitude ),
     height_( hight ), topoHeight_( INT_MIN ), modeltopoSearched( false ),
     config( new ConfigData() )
{
	itProviderPriorityBegin = providerPriority.begin();
}

bool
LocationElem::
EnableThunder::
hasThunder() const
{
	if( ! isInitialized ) {
		WEBFW_USE_LOGGER( "encode" );

		isInitialized = true;
		if( elem->config ) {
			boost::posix_time::ptime now=boost::posix_time::second_clock::universal_time();
			if( elem->config->thunder.valid() ) {
				thunder = elem->config->thunder.isEnabled( now );
				WEBFW_LOG_DEBUG("Thunder in symbols enabled: '" << elem->config->thunder.enabledPeriod( now.date() )<<"'. Spec: '" << elem->config->thunder << "'. Thunder enabled: " << (thunder?"true":"false")<<".");
			} else {
				WEBFW_LOG_ERROR("Thunder in symbols disabled. Invalid time period specification in 'symbols_enable_thunder'." );
			}
		} else {
			thunder = true;
			WEBFW_LOG_ERROR("Thunder. Internal error. Missing configuration 'LocationElem::EnableThunder::hasThunder'. (Thunder enabled)." );
		}
	}
	return thunder;
}

void
LocationElem::
init( ITimeSerie itTimeSerie, TimeSerie *timeSerie )
{
	this->itTimeSerie = itTimeSerie;
	this->timeSerie = timeSerie;
	
	lastUsedProvider_.erase();


	//forecastProvider.erase();
   //percentileProvider.erase();
	//precipRefTime = ptime();
}

bool
LocationElem::
startAtProvider( const std::string &providerWithPlacename )
{
	if( providerWithPlacename.empty() ) {
		itProviderPriorityBegin = providerPriority.begin();
	} else {
		itProviderPriorityBegin = providerPriority.findProviderWithoutPlacename( providerWithPlacename );

		if( itProviderPriorityBegin == providerPriority.end() ) {
			itProviderPriorityBegin = providerPriority.begin();
			return false;
		}
	}

	return true;
}

const double HEIGHT_CORRECTION_PER_METER = 0.006;

float
LocationElem::
computeTempCorrection( const std::string &provider, int &relTopo, int &modelTopo ) const
{
	//WEBFW_USE_LOGGER( "decode" );
	//WEBFW_LOG_DEBUG( "computeTempCorrection: " << "Realight=" << hight_ << " provider: "<< provider );
	
	if( height_ == INT_MIN ) {
		//WEBFW_LOG_DEBUG( "computeTempCorrection: " << "Realight=INT_MIN (" << hight_ << ") provider: "<< provider );
		return 0.0;
	}
		
	modelTopo = modeltopography( provider );
	
	if( modelTopo == INT_MIN ) {
	//	WEBFW_LOG_DEBUG( "computeTempCorrection: " << "modelTopo height=INT_MIN (" << modelTopo << ") provider: "<< provider );
		return 0.0;
	}
	
	relTopo  = modelTopo -  height_ ;


	return relTopo * HEIGHT_CORRECTION_PER_METER;
}

std::string
LocationElem::
forecastprovider() const
{
   if( ! forecastProvider.empty() )
      return forecastProvider;

   if( ! oceanProvider_.empty() )
      return oceanProvider_;

   if( ! percentileProvider.empty() )
      return percentileProvider;

   return "";
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
T2M_NO_ADIABATIC_HIGHT_CORRECTION( bool tryHard )const
{
	return getValue( &PData::T2M_NO_ADIABATIC_HIGHT_CORRECTION,
			         itTimeSerie->second,
			         const_cast<ptime&>(itTimeSerie->first),
			         const_cast<string&>(forecastProvider), FLT_MAX, tryHard );
}

float
LocationElem::
maxTemperature( int hours, bool tryHard )const
{
	switch( hours ) {
	case 6: return getValue( &PData::maxTemperature_6h,
	         	 	 	 	 itTimeSerie->second,
							 const_cast<ptime&>(itTimeSerie->first),
							 const_cast<string&>(forecastProvider), FLT_MAX, tryHard );
	default:
		return FLT_MAX;
	}
}

float
LocationElem::
minTemperature( int hours, bool tryHard )const
{
	switch( hours ) {
	case 6: return getValue( &PData::minTemperature_6h,
		         	 	 	 itTimeSerie->second,
							 const_cast<ptime&>(itTimeSerie->first),
							 const_cast<string&>(forecastProvider), FLT_MAX, tryHard );
	default:
		return FLT_MAX;
	}
}

float
LocationElem::
temperatureCorrected( bool tryHard )const
{
	return getValue( &PData::temperatureCorrected,
			         itTimeSerie->second,
			         const_cast<ptime&>(itTimeSerie->first),
			         const_cast<string&>(forecastProvider), FLT_MAX, tryHard );

}

void
LocationElem::
temperatureCorrected( float temperature, const std::string &provider_, bool all )
{
	string provider(provider_);
	WEBFW_USE_LOGGER( "encode" );

	if( provider.empty() && ! all ) {
		if( forecastProvider.empty() )
			return;
		else
			provider = forecastProvider;
	}

	FromTimeSerie::iterator itFrom = itTimeSerie->second.find( itTimeSerie->first );

	if( itFrom == itTimeSerie->second.end() )
		return;

	if( all ) {
		for( ProviderPDataList::iterator itProvider = itFrom->second.begin();
			itProvider !=  itFrom->second.end() ; ++itProvider )
		{

			if( itProvider->second.temperatureCorrected == FLT_MAX ) {
				WEBFW_LOG_DEBUG( "LocationElem::temperaturCorrected (all): Set to: " << temperature << " '" << itProvider->first << "'.'");
				itProvider->second.temperatureCorrected = temperature;
			}
		}
	} else {
		ProviderPDataList::iterator itProvider = itFrom->second.find( provider );

		if( itProvider == itFrom->second.end() )
			return;

		WEBFW_LOG_DEBUG( "LocationElem::temperaturCorrected: Set to: " << temperature << " '" << provider << "'.'");
		itProvider->second.temperatureCorrected = temperature;
	}
}

float
LocationElem::
wetBulbTemperature( bool tryHard )const
{
   //WEBFW_USE_LOGGER( "main" );
   WEBFW_USE_LOGGER( "+wetbulb" );
   log4cpp::Priority::Value logLevel=WEBFW_GET_LOGLEVEL();
   bool doLog = logLevel >= log4cpp::Priority::DEBUG;

   if( config && !config->outputParam("symbol:T.WB") ) {
	   WEBFW_LOG_INFO("WetBulb is not configured to be used.");
      return FLT_MAX;
   }

   float tUhc; //Model temperature without height correction.
   float tHc;  //Model temperature with height correction.
   float rh;   //Relative humidity at 2 meter.
   float h;      //"Real" height.

   tUhc = T2M( tryHard );
   tHc  = temperatureCorrected( tryHard );
   rh = RH2MFromModel( false );
   h = height();

   if( tUhc == FLT_MAX || tHc == FLT_MAX || rh == FLT_MAX ||
       h == INT_MIN || h == INT_MAX ) {
      ostringstream ost;

      ost << "t: " << time() << " MISSING data for parameter(s): ";
      if( tUhc == FLT_MAX )
         ost << "tUhc";
      if( tHc == FLT_MAX )
         ost << " tHc";
      if( rh == FLT_MAX )
         ost << " rh";
      if( h == INT_MIN || h == INT_MAX)
         ost << " h";

      WEBFW_LOG_DEBUG( ost.str() );

      return FLT_MAX;
   }

   float e = (rh/100)*0.611*exp( (17.63 * tUhc) / (tUhc + 243.04) );
   float td = (116.9 + 243.04 * log( e ))/(16.78-log( e ));
   float gamma = 0.00066*101300*exp(-1.21e-4*h)*0.001;
   float rho = (4098*e)/pow(td+243.04, 2);
   float tw = (gamma*tHc+rho*td)/(gamma+rho);
   float prevTw = tw;

   if( tw > tHc)
      tw = tHc;

   if( doLog ) {
	   ostringstream ost;

	   ost << "t: " << this->time() << " h: " << h  << " RH: " << rh
		   << " T.UHC: " << tUhc
	       << " T.HC: " << tHc;

	   if( prevTw != tw )
		   ost << " T.WB: " << prevTw <<" [T.WB: " << tw <<" > T.HC: "<< tHc << " => T.WB=T.HC]";
	   else
		   ost << " T.WB: " << tw;

	   WEBFW_LOG_DEBUG( ost.str() );
   }

   return tw;
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

bool
LocationElem::
PRECIP_MIN_MAX_MEAN( int hoursBack, boost::posix_time::ptime &backTime_,
                     float &minOut, float &maxOut, float &meanOut, float probOut,
                     bool tryHard )const
{
   const float MIN_PRECIP=0;
// static float a[]={ 1.0,  0.3370, 0.6121, 0.9307, 0.4207, 1.4325 };
   static float a[]={ 0.98, 0.60,   0.64,   0.32,   0.39,   1.75 };

   std::vector<float> min;
   std::vector<float> max;
   std::vector<float> mean;
   std::vector<float> prob;
   std::vector<float> minMaxDif;

   float precipMin;
   float precipMax;
   float precipMean;
   float precipProb;
   float maxPrecipMax=0.0;
   bool myTryHard = tryHard;
   bool isEqual=false;
   boost::posix_time::ptime fromTime;
   boost::posix_time::ptime stopTime;
   boost::posix_time::ptime startTime;
   boost::posix_time::ptime savedFromTime;

   //WEBFW_USE_LOGGER( "main" );

   min.reserve( hoursBack );
   max.reserve( hoursBack );
   mean.reserve( hoursBack );
   prob.reserve( hoursBack );
   minMaxDif.reserve( hoursBack );

   backTime_=boost::posix_time::ptime(); //Set it to undefined.

   stopTime=itTimeSerie->first;

   //Find the start of the accumulation period.
   fromTime = itTimeSerie->first - hours( hoursBack );
   savedFromTime = fromTime;

   //We must adjust the startime with one hour because
   //the one hour precipitation is for the hour preceeding
   //the totime.
   startTime = fromTime + hours( 1 );

   //WEBFW_LOG_DEBUG( "LocationElem::PRECIP_MIN_MAX_MEAN: hoursBack: " << hoursBack << " startTime: " << startTime << " stopTime: " << stopTime );
   CITimeSerie it = timeSerie->find( startTime );

   if( it == timeSerie->end() ) {
      //WEBFW_LOG_DEBUG("PRECIP_MIN_MAX_MEAN: No data. (Cant find a dataset for startTime '" << startTime << "').");
      return false;
   }

   for( ;
        it != timeSerie->end() && it->first <= stopTime;
        ++it )
   {

      if( isEqual )
         fromTime = it->first;

      precipMean = getValue( &PData::PRECIP_MEAN,
                             it->second,
                             fromTime ,
                             const_cast<string&>(forecastProvider), FLT_MAX, myTryHard );

      if( precipMean == FLT_MAX && !isEqual ) {
         isEqual = true;
         //Is fromtime and totime equal.
         fromTime = it->first;
         precipMean = getValue( &PData::PRECIP_MEAN,
                                it->second,
                                fromTime ,
                                const_cast<string&>(forecastProvider), FLT_MAX, myTryHard );

      }

      if( precipMean == FLT_MAX ) {
         //WEBFW_LOG_DEBUG("PRECIP_MIN_MAX_MEAN: no PRECIP.MEAN provider: " << forecastProvider  << " fromtime: " << fromTime << " to: " << it->first );
         return false;
      }

      myTryHard = false;

      precipMin = getValue( &PData::PRECIP_MIN,
                            it->second,
                            fromTime ,
                            const_cast<string&>(forecastProvider), FLT_MAX, myTryHard );


      if( precipMin == FLT_MAX ) {
         //WEBFW_LOG_DEBUG("PRECIP_MIN_MAX_MEAN: no PRECIP.MIN" );
         return false;
      }

      precipMax = getValue( &PData::PRECIP_MAX,
                            it->second,
                            fromTime ,
                            const_cast<string&>(forecastProvider), FLT_MAX, myTryHard );

      if( precipMax == FLT_MAX ) {
         //WEBFW_LOG_DEBUG("PRECIP_MIN_MAX_MEAN: no PRECIP.MAX" );
         return false;
      }

      precipProb = getValue( &PData::PRECIP_PROBABILITY,
                            it->second,
                            fromTime ,
                            const_cast<string&>(forecastProvider), FLT_MAX, myTryHard );

      //WEBFW_LOG_DEBUG("LocationElem::PRECIP_MIN_MAX_MEAN: " << fromTime << " mean: " << precipMean << " min: " << precipMin << " max: " << precipMax );

      fromTime = it->first;

      min.push_back( precipMin );
      max.push_back( precipMax );
      mean.push_back( precipMean );
      prob.push_back( precipProb );

      if( precipMax > maxPrecipMax )
    	  maxPrecipMax = precipMax;
   }

   backTime_ = savedFromTime;

   if( hoursBack == 1 ) {
      minOut = std::max( float(0), min[0] );
      maxOut = std::max( float(0), max[0] );
      meanOut = std::max( float(0), mean[0] );
      probOut = std::max( float(0), prob[0] );

      minOut = minOut <= MIN_PRECIP ? 0 : minOut;
      maxOut = maxOut <= MIN_PRECIP ? 0 : maxOut;
      meanOut = meanOut <= MIN_PRECIP ? 0 : meanOut;

      if( minOut>maxOut ) {
         float tmp = minOut;
         minOut = maxOut;
         maxOut = tmp;
      }
      return true;
   }

   if( hoursBack > 6 ) {
      minOut = FLT_MAX;
      maxOut = FLT_MAX;
      probOut = FLT_MAX;
      meanOut = std::max( float(0), sumVector( mean ) );

      return true;
   }

   float v;

   for( std::vector<float>::size_type i=0; i<min.size(); ++i ) {
      v = max[i] - min[i];
      minMaxDif.push_back( v<0?0:v );
   }

   if( ! sortVector( minMaxDif ) )
      return false;

   v=0.0;

   for( std::vector<float>::size_type i=0; i<min.size(); ++i )
      v += minMaxDif[i]*a[i];

   v = v/2;
   meanOut = std::max( float(0), sumVector( mean ) );
   minOut = std::max( float( 0 ), meanOut - v );
   maxOut = std::max( float(0), meanOut + v );

   //Algorithm change 5. july 2011.
   float sumMin = std::max( float(0), sumVector( min ) );
   minOut = std::max( sumMin, minOut );

   minOut = minOut<=MIN_PRECIP?0:minOut;
   maxOut = maxOut<maxPrecipMax?maxPrecipMax:maxOut;
   probOut = std::max( float(0), maxInVector( prob ) );

   if( minOut>maxOut ) {
      float tmp = minOut;
      minOut = maxOut;
      maxOut = tmp;
   }

   //WEBFW_LOG_DEBUG("PRECIP_MIN_MAX_MEAN: hoursBack: " << hoursBack << " (" << backTime_ << " - " << itTimeSerie->first << ") mean: " << meanOut << " min: " << minOut << " max: " << maxOut  );
   return true;
}





float
LocationElem::
PRECIP( int hoursBack, boost::posix_time::ptime &backTime_, bool tryHard )const
{
	const float MIN_PRECIP = 0.1f;
	
	float precip;

	//WEBFW_USE_LOGGER( "synop" );
	precip = PRECIP_MEAN( hoursBack, backTime_, tryHard );
	
/*	if( precip != FLT_MAX ) {
	   WEBFW_LOG_DEBUG("PRECIP: (PRECIP_MEAN): " << precip );
	} else {
	   WEBFW_LOG_DEBUG("PRECIP: (PRECIP_MEAN): Opppppppssssssss" );
	}*/

	if( precip == FLT_MAX )
	   precip = PRECIP_N( hoursBack, backTime_, tryHard );
	
	if( precip == FLT_MAX )
		precip = PRECIP_1H( hoursBack, backTime_, tryHard );
	
	if( precip == FLT_MAX )
		return FLT_MAX;
	
   if ( (int) floorf( precip * 10 ) == (int)floorf((precip+0.05f)*10) )
      precip -= 0.0001; 

   if( precip <= MIN_PRECIP )
      precip = 0;
	
	return precip;
}


float
LocationElem::
PRECIP_MEAN( int hoursBack, boost::posix_time::ptime &backTime_, bool tryHard )const
{
   float sumPrecip=0;
   float precipNow;
   int   count=0;
   boost::posix_time::ptime fromTime;
   boost::posix_time::ptime stopTime;
   boost::posix_time::ptime startTime;
   boost::posix_time::ptime savedFromTime;
   bool isEqual=false;
   bool myTryHard = tryHard;

   WEBFW_USE_LOGGER( "main" );

   backTime_=boost::posix_time::ptime(); //Set it to undefined.

   stopTime=itTimeSerie->first;

   //Find the start of the accumulation period.
   fromTime = itTimeSerie->first - hours( hoursBack );
   savedFromTime = fromTime;

   //We must adjust the startime with one hour because
   //the one hour precipitation is for the hour preceding
   //the totime.
   startTime = fromTime + hours( 1 );

   //WEBFW_LOG_DEBUG( "LocationElem::PRECIP_MEAN: hoursBack: " << hoursBack << " startTime: " << startTime << " stopTime: " << stopTime );
   for( CITimeSerie it = timeSerie->find( startTime );
        it != timeSerie->end() && it->first <= stopTime;
        ++it )
   {


      if( isEqual )
         fromTime = it->first;

      precipNow = getValue( &PData::PRECIP_MEAN,
                            it->second,
                            fromTime ,
                            const_cast<string&>(forecastProvider), FLT_MAX, myTryHard );

      if( !isEqual && precipNow == FLT_MAX  ) {
         isEqual = true;
         //Is fromtime and totime equal.
         fromTime = it->first;
         precipNow = getValue( &PData::PRECIP_MEAN,
                                it->second,
                                fromTime ,
                                const_cast<string&>(forecastProvider), FLT_MAX, myTryHard );
      }

      //WEBFW_LOG_DEBUG( "LocationElem::PRECIP_MEAN: loop: fromTime: " << fromTime << " toTime: " << it->first  << " precip: " << precipNow);
      fromTime = it->first;

      if( precipNow == FLT_MAX )
         return FLT_MAX;

      myTryHard = false;
      sumPrecip += std::max( float(0), precipNow );
      count++;
   }

   if( count == 0 ) {
      //WEBFW_LOG_DEBUG( "LocationElem::PRECIP_MEAN: return sumPrecip: FLT_MAX" );
      return FLT_MAX;
   }

   backTime_ = savedFromTime;

   //WEBFW_LOG_DEBUG( "LocationElem::PRECIP_MEAN: return sumPrecip:" << sumPrecip );
   return std::max( float(0), sumPrecip );
}


float 
LocationElem::
PRECIP_1H( int hoursBack, boost::posix_time::ptime &backTime_, bool tryHard )const
{
	float sumPrecip=0;
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
	
	//WEBFW_LOG_DEBUG( "LocationElem::PRECIP_1H: hoursBack: " << hoursBack << " startTime: " << startTime << " stopTime: " << stopTime );
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
		
		sumPrecip += std::max( float(0), precipNow );
		count++;
	}

	if( count == 0 ) {
		//WEBFW_LOG_DEBUG( "LocationElem::PRECIP_1H: return sumPrecip: FLT_MAX" );
		return FLT_MAX;
	}

	backTime_ = savedFromTime;
	
	//WEBFW_LOG_DEBUG( "LocationElem::PRECIP_1H: return sumPrecip:" << sumPrecip );
	return std::max( float(0), sumPrecip );
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

	if( precipNow == FLT_MAX || fromTime.is_special() ) {
		//cerr << "PRECIP_N (" << hoursBack << "): " << "FLT_MAX" <<" provider <" << forecastProvider << "> tryHard: " << (tryHard?"true":"false")
		//	 <<	" fromtime: "<< fromTime << endl;
		return FLT_MAX;
	}
	//cerr << "PRECIP_N first (" << hoursBack << "): " << precipNow  <<" provider <" << forecastProvider << "> tryHard: " << (tryHard?"true":"false")
	//	 <<	" fromtime: "<< fromTime << endl;

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
		
		while( itTmp != timeSerie->begin() && itTmp->first > backTime )
		   --itTmp;

		if( itTmp == timeSerie->begin() )
		   return FLT_MAX;

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
		
		precip = std::max( float(0), precipNow ) - std::max( float(0), precipBack );
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

		precip = std::max( float(0), precipNow ) - std::max( float(0), precipBack );
	}

	backTime_ = backTime;
	
	//cerr << "PRECIP_N prec: " << precip << endl;
	return std::max( float(0), precip );
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
visibility( bool tryHard)const
{
	return getValue( &PData::visibility,
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
RH2MFromModel(bool tryHard)const
{
	return getValue( &PData::RH2M,
	                 itTimeSerie->second,
	                 const_cast<ptime&>(itTimeSerie->first),
	                 const_cast<string&>(forecastProvider), FLT_MAX, tryHard );
}

float
LocationElem::
humidity( float tempUsed, bool tryHard )const
{
	RestoreForecastProvider restoreForecastProvider( const_cast<LocationElem*>(this) );
	float rh = RH2MFromModel( tryHard );

	if( rh == FLT_MAX )
		rh = miutil::dewPointTemperatureToRelativeHumidity( tempUsed, dewPointTemperatureFromModel( tryHard ) );

	return rh;
}

float
LocationElem::
dewPointTemperatureFromModel( bool tryHard )const
{
   return getValue( &PData::dewPointTemperature,
                    itTimeSerie->second,
                    const_cast<ptime&>(itTimeSerie->first),
                    const_cast<string&>(forecastProvider), FLT_MAX, tryHard );
}

float 
LocationElem::
dewPointTemperature( float usedTemperature, bool tryHard )const
{
	RestoreForecastProvider restoreForecastProvider( const_cast<LocationElem*>(this) );
	float modelTemp;
	float dewTemp = miutil::dewPointTemperature( usedTemperature, RH2MFromModel( tryHard ) );;

	if( dewTemp != FLT_MAX )
		return dewTemp;

	dewTemp = dewPointTemperatureFromModel( tryHard );
	modelTemp = T2M( false );

	if( dewTemp == FLT_MAX || modelTemp == FLT_MAX )
		return FLT_MAX;

	//Compute relative humidity from model temperature and model dewtemperature.
	//This is the model relative humidity.
	float modelRh = miutil::dewPointTemperatureToRelativeHumidity( modelTemp, dewTemp );

	//Compute the dew temperature from height corrected temperature and
	//relative humidity from the model.
	dewTemp = miutil::dewPointTemperature( usedTemperature, modelRh );

	return dewTemp;
}

float
LocationElem::
thunderProbability( bool tryHard )const
{
	if( ! thunder.hasThunder() ) {
		return FLT_MAX;
	}

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
	string provider(symbolProbabilityProvider);

	if( provider.empty() ) {
		if( ! symbolProvider.empty() )
			provider = symbolProvider;
		else
			provider = forecastProvider;
	}

	float p = getValue( &PData::symbol_PROBABILITY,
			            itTimeSerie->second,
			            fromTime,
			            provider, FLT_MAX, tryHard );

	if( p != FLT_MAX )
		const_cast<string&>( symbolProbabilityProvider ) = provider;

	return p;
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
iceingIndex( bool tryHard, const std::string &provider )const
{

   if( provider.empty() )
      return getValue( &PData::iceingIndex,
                       itTimeSerie->second,
                       const_cast<ptime&>(itTimeSerie->first),
                       const_cast<string&>(oceanProvider_), FLT_MAX, tryHard );
   else {
      string providerTmp( provider );
      return getValue( &PData::iceingIndex,
                       itTimeSerie->second,
                       const_cast<ptime&>(itTimeSerie->first),
                       const_cast<string&>( providerTmp ), FLT_MAX, tryHard );
   }

}

float
LocationElem::
significantSwellWaveHeight( bool tryHard )const
{
   if( config && !config->outputParam("significantSwellWaveHeight") )
      return FLT_MAX;

   return getValue( &PData::significantSwellWaveHeight,
                    itTimeSerie->second,
                    const_cast<ptime&>(itTimeSerie->first),
                    const_cast<string&>(oceanProvider_), FLT_MAX, tryHard );

}

float
LocationElem::
meanSwellWavePeriode( bool tryHard )const
{
   if( config && !config->outputParam("meanSwellWavePeriode") )
      return FLT_MAX;

   return getValue( &PData::meanSwellWavePeriode,
                    itTimeSerie->second,
                    const_cast<ptime&>(itTimeSerie->first),
                    const_cast<string&>(oceanProvider_), FLT_MAX, tryHard );
}

float
LocationElem::
meanSwellWaveDirection( bool tryHard )const
{
   if( config && !config->outputParam("meanSwellWaveDirection") )
      return FLT_MAX;

   return getValue( &PData::meanSwellWaveDirection,
                    itTimeSerie->second,
                    const_cast<ptime&>(itTimeSerie->first),
                    const_cast<string&>(oceanProvider_), FLT_MAX, tryHard );
}

float
LocationElem::
peakSwellWavePeriode( bool tryHard )const
{
   if( config && !config->outputParam("peakSwellWavePeriode") )
      return FLT_MAX;

   return getValue( &PData::peakSwellWavePeriode,
                    itTimeSerie->second,
                    const_cast<ptime&>(itTimeSerie->first),
                    const_cast<string&>(oceanProvider_), FLT_MAX, tryHard );
}

float
LocationElem::
peakSwellWaveDirection( bool tryHard )const
{
   if( config && !config->outputParam("peakSwellWaveDirection") )
      return FLT_MAX;

   return getValue( &PData::peakSwellWaveDirection,
                    itTimeSerie->second,
                    const_cast<ptime&>(itTimeSerie->first),
                    const_cast<string&>(oceanProvider_), FLT_MAX, tryHard );
}

float
LocationElem::
meanTotalWavePeriode( bool tryHard )const
{
   if( config && !config->outputParam("meanTotalWavePeriode") )
      return FLT_MAX;

   return getValue( &PData::meanTotalWavePeriode,
                    itTimeSerie->second,
                    const_cast<ptime&>(itTimeSerie->first),
                    const_cast<string&>(oceanProvider_), FLT_MAX, tryHard );
}

float
LocationElem::
maximumTotalWaveHeight( bool tryHard )const
{
   if( config && !config->outputParam("maximumTotalWaveHeight") )
      return FLT_MAX;

   return getValue( &PData::maximumTotalWaveHeight,
                    itTimeSerie->second,
                    const_cast<ptime&>(itTimeSerie->first),
                    const_cast<string&>(oceanProvider_), FLT_MAX, tryHard );
}


float
LocationElem::
seaCurrentVelocityU( bool tryHard )const
{
	return getValue( &PData::seaCurrentVelocityU, 
			           itTimeSerie->second,
			           const_cast<ptime&>(itTimeSerie->first), 
			           const_cast<string&>(oceanProvider_), FLT_MAX, tryHard );
}

float 
LocationElem::
seaCurrentVelocityV( bool tryHard )const
{
	return getValue( &PData::seaCurrentVelocityV, 
			           itTimeSerie->second,
			           const_cast<ptime&>(itTimeSerie->first), 
			           const_cast<string&>(oceanProvider_), FLT_MAX, tryHard );
}

float 
LocationElem::
seaSalinity( bool tryHard )const
{
	return getValue( &PData::seaSalinity, 
			           itTimeSerie->second,
			           const_cast<ptime&>(itTimeSerie->first), 
			           const_cast<string&>(oceanProvider_), FLT_MAX, tryHard );
}

float 
LocationElem::
seaSurfaceHeight( bool tryHard )const
{
	return getValue( &PData::seaSurfaceHeight, 
			                itTimeSerie->second,
			                const_cast<ptime&>(itTimeSerie->first), 
						       const_cast<string&>(oceanProvider_), FLT_MAX, tryHard );
}

float 
LocationElem::
seaTemperature( bool tryHard )const
{
	return getValue( &PData::seaTemperature, 
			           itTimeSerie->second,
			           const_cast<ptime&>(itTimeSerie->first), 
			           const_cast<string&>(oceanProvider_), FLT_MAX, tryHard );
}
  
float 
LocationElem::
meanTotalWaveDirection( bool tryHard )const
{
	return getValue( &PData::meanTotalWaveDirection, 
			           itTimeSerie->second,
			           const_cast<ptime&>(itTimeSerie->first), 
			           const_cast<string&>(oceanProvider_), FLT_MAX, tryHard );
}

float 
LocationElem::
significantTotalWaveHeight( bool tryHard )const
{
	return getValue( &PData::significantTotalWaveHeight, 
			           itTimeSerie->second,
			           const_cast<ptime&>(itTimeSerie->first), 
			           const_cast<string&>(oceanProvider_), FLT_MAX, tryHard );
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

std::string
LocationElem::
topoProvider( const std::string &provider_, TopoProviderMap &topoProviders )
{
	string provider( provider_ );

//	WEBFW_USE_LOGGER( "decode" );

//	WEBFW_LOG_DEBUG( "modeltopographyProvider: provider: "<< provider_ );
	TopoProviderMap::const_iterator it=topoProviders.find( provider_ );

	if( it != topoProviders.end() ) {
//		WEBFW_LOG_DEBUG( "topoProvider: provider: "<< provider << " --> " << it->second );
		if( ! it->second.empty() )
			provider = *it->second.begin();
	} else {
		ProviderItem item = ProviderList::decodeItem( provider_ );

		if( ! item.placename.empty() ) {
			it=topoProviders.find( item.provider );
			if( it != topoProviders.end() ) {
//				WEBFW_LOG_DEBUG( "modeltopographyProvider: provider: "<< provider << " --> " << item.provider << " --> " << it->second );
				if( ! it->second.empty() ) {
					provider = *it->second.begin();
					topoProviders[ provider_ ].push_back( provider );
				}
			}
		}
	}

	//provider += topoPostfix;

	return provider;
}



int
LocationElem::
modeltopography( const std::string &provider_ )const
{
	WEBFW_USE_LOGGER( "decode" );

	string provider =  const_cast<LocationElem*>( this )->topoProvider( provider_,
			                                                            const_cast<LocationElem*>( this )->modelTopoProviders );

	//WEBFW_LOG_WARN( "modeltopography: providerin: " << provider_ << " resolved: " << provider );

//	{
//		ostringstream ost;
//		ost << "LocationElem::modeltopograpy: '" << provider << "(" << provider_ << ")'" << endl;
//		for( TopoProviderMap::const_iterator it = modelTopoProviders.begin();
//		     it != modelTopoProviders.end(); ++it ) {
//			ost << "  " << it->first << ":";
//			for( std::list<std::string>::const_iterator sit=it->second.begin();
//					sit != it->second.end(); ++sit ){
//				ost << " '" << *sit << "'";
//			}
//			ost << endl;
//		}
//		WEBFW_LOG_DEBUG( ost.str() );
//	}


	TimeSerie::const_iterator it1=timeSerie->find( topoTime );
	
	if( it1 == timeSerie->end() ) {
		WEBFW_LOG_WARN( "modeltopography: No topography fields loaded." );
		return INT_MIN;
	}
	
	FromTimeSerie::const_iterator it2=it1->second.find( topoTime );
	
	if( it2 == it1->second.end() ) {
		WEBFW_LOG_WARN( "modeltopography: No topography fields loaded." );
		return INT_MIN;
	}

//	{
//		ostringstream ost;
//
//		for( ProviderPDataList::const_iterator it=it2->second.begin();
//			 it != it2->second.end(); ++it	) {
//			ost << "Model Topograpy: " << it->first << " Value: " << it->second.modeltopography << endl;
//		}
//		WEBFW_LOG_DEBUG( ost.str() );
//	}

	
	ProviderPDataList::const_iterator it3=it2->second.find( provider + topoPostfix );
	
	if( it3 == it2->second.end() ) {
		ProviderItem item = ProviderList::decodeItem( provider );

		if( item.placename.empty() ) {
			ProviderItem itemIn = ProviderList::decodeItem( provider_ );

			if( ! itemIn.placename.empty() ) {
				item.placename = itemIn.placename;

				provider = item.providerWithPlacename();
				it3=it2->second.find( provider + topoPostfix );

				//WEBFW_LOG_DEBUG( "modeltopography: provider in: " << provider_ << " Use provider: " << provider );

				if( it3 == it2->second.end() ) {
					if( provider_ != provider ) {
						WEBFW_LOG_INFO( "modeltopography: No topo data for provider <" << provider_ <<"> with alias <" <<  provider << ">" );
					}else {
						WEBFW_LOG_INFO( "modeltopography: No topo data for provider <" << provider_ <<">." );
					}

					return INT_MIN;
				}

				WEBFW_LOG_INFO( "modeltopography: Adding alias <" << provider << "> for <"<< provider_ << ">." );
				const_cast<LocationElem*>(this)->modelTopoProviders[provider_].push_front( provider );
			}
		} else {
			if( provider_ != provider ) {
				WEBFW_LOG_INFO( "modeltopography: No topo data for provider <" << provider_ <<"> with alias <" <<  provider << ">" );
			}else {
				WEBFW_LOG_INFO( "modeltopography: No topo data for provider <" << provider_ <<">." );
			}

			return INT_MIN;
		}
	}
	
	if( it3->second.modeltopography == FLT_MAX ) {
		if( provider_ != provider ) {
			WEBFW_LOG_ERROR( "modeltopography: No topo data for provider <" << provider_ <<"> with alias <" <<  provider << ">, but the field is present." );
		}else {
			WEBFW_LOG_ERROR( "modeltopography: No topo data for provider <" << provider_ <<">, but the field is present." );
		}

		return INT_MIN;
	}
	
	return static_cast<int>( it3->second.modeltopography );
}

int
LocationElem::
topography( const std::string &provider_ )const
{
	WEBFW_USE_LOGGER( "decode" );

	string provider;
	string::size_type ii;

	TimeSerie::const_iterator it1=timeSerie->find( topoTime );

	if( it1 == timeSerie->end() ) {
		WEBFW_LOG_WARN( "topography: No topography fields loaded." );
		return INT_MIN;
	}

	FromTimeSerie::const_iterator it2=it1->second.find( topoTime );

	if( it2 == it1->second.end() ) {
		WEBFW_LOG_WARN( "topography: No topography fields loaded." );
		return INT_MIN;
	}

	WEBFW_LOG_DEBUG( "topography: lookingup: " << provider_ << topographyPostfix );

//	{
//		ostringstream ost;
//
//		for( ProviderPDataList::const_iterator it=it2->second.begin();
//			 it != it2->second.end(); ++it	) {
//			ost << "Topograpy: " << it->first << " Value: " << it->second.topography << endl;
//		}
//		WEBFW_LOG_DEBUG( ost.str() );
//	}

	ProviderPDataList::const_iterator it3=it2->second.find( provider_ + topographyPostfix );


	if( it3 == it2->second.end() ) {
		ProviderItem itemIn = ProviderList::decodeItem( provider_ );

		if( itemIn.placename.empty() ) {
			ProviderItem item;

			for( it3=it2->second.begin();
				 it3 != it2->second.end(); ++it3	) {
				provider = it3->first;
				ii = provider.find( topographyPostfix );

				if( ii == string::npos )
					continue;

				provider.erase( ii );
				item = ProviderList::decodeItem( provider );

				if( itemIn.provider == item.provider )
					break;
			}
		} else {
			WEBFW_LOG_INFO( "topography: No topo data for provider <" << provider_ <<">." );

			return INT_MIN;
		}
	}

	if( it3 == it2->second.end() )
		return INT_MIN;

	if( it3->second.topography == FLT_MAX ) {
		WEBFW_LOG_ERROR( "topography: No topo data for provider <" << provider_ <<">, but the field is present." );

		return INT_MIN;
	}

	//WEBFW_LOG_ERROR( "topography: value: " <<  it3->second.topography << " provider: " << it3->first );
	return static_cast<int>( it3->second.topography );

}


float
LocationElem::
landcover( const std::string &provider )const
{
   WEBFW_USE_LOGGER( "decode" );

   TimeSerie::const_iterator it1=timeSerie->find( epoch );

   if( it1 == timeSerie->end() ) {
      WEBFW_LOG_WARN( "landcover: No LANDCOVER field loaded." );
      return FLT_MIN;
   }

   FromTimeSerie::const_iterator it2=it1->second.find( epoch );

   if( it2 == it1->second.end() ) {
      WEBFW_LOG_WARN( "landcover: No LANDCOVER field loaded." );
      return FLT_MIN;
   }

   float value=FLT_MIN;

   ProviderPDataList::const_iterator it3=it2->second.find( provider );

   if( it3 != it2->second.end() )
     value = it3->second.LANDCOVER;

   if( WEBFW_GET_LOGLEVEL() >= 8 ) {
      ostringstream ost;
      ost << "landcover: lookup: '" << provider << "' Value: ";

      if( value == FLT_MIN )
         ost << "(N/A)";
      else
         ost << value;

      WEBFW_LOG_DEBUG( ost.str() );
   }

   return value;
}

}

