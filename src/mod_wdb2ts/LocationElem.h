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


#ifndef LOCATIONELEM_H_
#define LOCATIONELEM_H_

#include <iostream>
#include <list>
#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/utility.hpp>
#include <trimstr.h>
#include <PointDataHelper.h>
#include <UpdateProviderReftimes.h>
#include <TopoProvider.h>
#include "configdata.h"

namespace wdb2ts {

/**
 * LocationElem is a helper class to lookup values in
 * a TimeSerie map. A TimeSerie map is defined as:
 *  map[validtotime][validfromtime][providername].
 * 
 * Search for valid values is in the same order as the
 * TimeSerie definition, ie:
 * 
 *  - validtotime
 *  - validfromtime
 *  - providername 
 */
class LocationElem {
	LocationElem( const LocationElem &);
	LocationElem& operator=( const LocationElem &);

	friend class LocationData;
	
	typedef float PData::*PM_float;
	typedef int PData::*PM_int;
	typedef std::string PData::*PM_string;
	
	class EnableThunder : boost::noncopyable {
		EnableThunder(); //Not implemented.
		mutable bool isInitialized;
		mutable bool thunder;
		LocationElem *elem;

	public:
		EnableThunder(	LocationElem *e )
			:isInitialized( false ), thunder(false), elem( e ) {}
		bool hasThunder() const;
	}thunder;

	const boost::posix_time::ptime topoTime;
	const std::string topoPostfix;
	const std::string topographyPostfix;
	const boost::posix_time::ptime seaIceTime;
	const boost::posix_time::ptime seaBottomTopographyTime;
	const boost::posix_time::ptime epoch;
	TimeSerie *timeSerie;
	ITimeSerie itTimeSerie;
	ProviderList providerPriority;
	ProviderList::const_iterator itProviderPriorityBegin;
	TopoProviderMap modelTopoProviders;
	std::list<std::string>  topographyProviders;
	std::string forecastProvider;
	std::string oceanProvider_;
	std::string percentileProvider;
	std::string modelTopoProvider;
	std::string symbolProvider;
	std::string symbolProbabilityProvider;
	std::string lastUsedProvider_;
	boost::posix_time::ptime precipRefTime;
	float       latitude_;
	float       longitude_;
	int         height_;
	int         topoHeight_;
	bool        modeltopoSearched;


	LocationElem();
	LocationElem( const ProviderList &providerPriority,
				  const TopoProviderMap &modelTopoProviders,
				  const std::list<std::string> &topographyProviders,
			      float longitude, float latitude, int hight  );
	
	void init( ITimeSerie itTimeSerie, TimeSerie *timeSerie );

	/**
	 * startAtProviderWithPlacename, set the itProviderPriorityBegin
	 * to the element in the providerPriority list where we shall
	 * start to look for data.
	 *
	 * @providerWithPlaceName empty the iterator is set to the
	 * beginning of the providerPriority list. If NOT empty search up this
	 * provider and set the iterator to this element.
	 *
	 * @return true if the provider is found and the iterator is set.
	 * Return false if the provider is NOT found, and set the iterator
	 * to the beginning.
	 */
	bool startAtProvider( const std::string &providerWithPlaceName );
	
	std::string topoProvider( const std::string &provider_, TopoProviderMap &topoProviders );


	
	/**
	 * Search for a value in the 'fromTimeSerie'. 
	 * pPM points to the value to search for in a PData struct.
	 * 
	 * fromTimeSerie is a map of fromTimeSerie[validFromtime][provider].
	 * 
	 * The value is searched for in this order:
	 * 
	 * - fromTime.
	 * - provider.
	 * 
	 * The first time this function is called with an empty provider
	 * the value is searched for in the sequence given by providerPriority.
	 *
	 * If 'fromTime' is not given, ie fromTime.is_special() return true,
	 * all fromTimes in the 'fromTimeSerie' is searched 
	 * 
	 * The first provider that gives a valid value for pPM is selceted.
	 * 
	 * If the 'provider' and/or 'fromTime' is not given it is 
	 * set on return if a valid value is found.
	 * 
	 * @param pPM a pointer to the value to search for.
	 * @param fromTimeSerie Search for a valid value in this map.
	 * @param fromTime The fromTime to search for. My not be set on
	 *         entry.
	 * @param provider Search for a value with this provider name. My
	 *        not be set at entry.
	 * @param notValidValue What identify a not valid value.
 	 * 
	 */
	template<class T>
	T getValueImpl( const T PData::* pPM,
					const FromTimeSerie &fromTimeSerie,
		            boost::posix_time::ptime &fromTime,
		            std::string &provider,
		            T notValidValue )const
	{
		using namespace std;
		CIFromTimeSerie itFromTimeSerie;
		CIProviderPDataList itProviderPDataList;
		ProviderList::const_iterator itProvider;
		//cerr << "getValue: ft: " << fromTime << " '" << provider << "' notValidValue: " << notValidValue << endl; 

		//const_cast<LocationElem*>(this)->lastUsedProvider_.erase();
		
		if( ! fromTime.is_special() ) 
			itFromTimeSerie = fromTimeSerie.find( fromTime ) ;
		else
			itFromTimeSerie = fromTimeSerie.begin() ;
		
		if( itFromTimeSerie == fromTimeSerie.end() )
			return notValidValue;
		
//		if( pPM == &PData::RH2M  && provider=="ecmwf atmospheric [ecmwf grid 3]" ) {
//			cerr << "getValueImpl: " <<  itTimeSerie->first << endl;
//			cerr << "   providerList: " << endl;
//			for( itProvider = itProviderPriorityBegin;
//			     itProvider != providerPriority.end();
//				 ++itProvider )
//				cerr << "      " << itProvider->providerWithPlacename()<< endl;
//			for( itProvider = itProviderPriorityBegin;
//					itProvider != providerPriority.end();
//					++itProvider ) {
//				itProviderPDataList = itFromTimeSerie->second.find( itProvider->providerWithPlacename() );
//
//				if( itProviderPDataList == itFromTimeSerie->second.end() )
//					continue;
//
//				cerr << "   RHM: " << itFromTimeSerie->first << " : " << itProvider->providerWithPlacename() << " = " << itProviderPDataList->second.*pPM << endl;
//			}
//		}

		if( provider.empty() ) {
			if( ! fromTime.is_special() ) {
				for( itProvider = itProviderPriorityBegin;
				     itProvider != providerPriority.end();
			        ++itProvider ) {
					itProviderPDataList = itFromTimeSerie->second.find( itProvider->providerWithPlacename() );
			
					if( itProviderPDataList == itFromTimeSerie->second.end() )
						continue;
			
					if( itProviderPDataList->second.*pPM == notValidValue )
						continue;
				
					provider = itProvider->providerWithPlacename();
					const_cast<LocationElem*>(this)->lastUsedProvider_ = provider;
					return itProviderPDataList->second.*pPM;
				}
			} else {
				for( ;itFromTimeSerie != fromTimeSerie.end(); ++itFromTimeSerie ) {
					for( itProvider = itProviderPriorityBegin;
					     itProvider != providerPriority.end();
					     ++itProvider ) {
						itProviderPDataList = itFromTimeSerie->second.find( itProvider->providerWithPlacename() );
						
						if( itProviderPDataList == itFromTimeSerie->second.end() )
							continue;
								
						if( itProviderPDataList->second.*pPM == notValidValue )
							continue;
									
						provider = itProvider->providerWithPlacename();
						const_cast<LocationElem*>(this)->lastUsedProvider_ = provider;
						fromTime = itFromTimeSerie->first;
									
						return itProviderPDataList->second.*pPM;
					}
				}
			}
		} else {
			if( ! fromTime.is_special() ) {
				itProviderPDataList = itFromTimeSerie->second.find( provider );
				
				if( itProviderPDataList == itFromTimeSerie->second.end() )
					return notValidValue;
			
				const_cast<LocationElem*>(this)->lastUsedProvider_ = provider;
				return itProviderPDataList->second.*pPM;
			} else {
				for( ; 
				     itFromTimeSerie != fromTimeSerie.end(); 
				     ++itFromTimeSerie ) {
					itProviderPDataList = itFromTimeSerie->second.find( provider );
					
					if( itProviderPDataList == itFromTimeSerie->second.end() )
						continue;
							
					if(itProviderPDataList->second.*pPM == notValidValue )
						continue;		
					
					const_cast<LocationElem*>(this)->lastUsedProvider_ = provider;
					fromTime = itFromTimeSerie->first;
					return itProviderPDataList->second.*pPM;
				}
			}
		}
		
		return notValidValue;
	}

	template<class T>
	T getValue( const T PData::* pPM,
			      const FromTimeSerie &fromTimeSerie,
			      boost::posix_time::ptime &fromTime,
			      std::string &provider,
			      T notValidValue,
			      bool myCleanProvider = false,
			      bool myCleanFromTime = false )const
   {
		T value = getValueImpl( pPM, fromTimeSerie, fromTime, provider, notValidValue );
		
		//Try to find a valid value from from the provider_priority list
		//with the same provider name as the requested provider, but where
		//we allow the placename to be different. This search is only activated
		//if we can't clean the provider. The provider on the entry on the
		//method is not set.
		if( value == notValidValue && ! provider.empty() && ! myCleanProvider) {
		   std::string::size_type i = provider.find_first_of( "[" );
		   std::string prefix;
		   std::string tmpProvider;
		   if( i != std::string::npos ) {
		      prefix = provider.substr( 0, i );
		      miutil::trimstr( prefix ); //The provider name without the gridname
		   } else {
		      prefix = provider;
		   }

		   for( ProviderList::const_iterator pit=itProviderPriorityBegin;
		        pit != providerPriority.end(); ++pit ) {
		      if( pit->provider != prefix )
		         continue;
		      tmpProvider = pit->providerWithPlacename();
		      value = getValueImpl( pPM, fromTimeSerie, fromTime, tmpProvider, notValidValue );

		      if( value != notValidValue )
		         break;
		   }
		}

		if( value == notValidValue ) {
			bool retry=false;
		
			if( ! provider.empty() && myCleanProvider ) {
				provider.erase();
				retry = true;
			}
			
			if( myCleanFromTime ) {
				fromTime = boost::posix_time::ptime();
				retry = true;
			}
			
			if( retry )
				value = getValueImpl( pPM, fromTimeSerie, fromTime, provider, notValidValue );
		}
		
		return value;
   }
	
public:
	
	~LocationElem(){};

	ConfigDataPtr config;
	
	///Return the provider that was last used to return data.
	std::string lastUsedProvider() const { return lastUsedProvider_; }
	std::string forecastprovider() const;
	std::string percentileprovider() const { return percentileProvider; }
	std::string oceanProvider() const { return oceanProvider_; }
	std::string modeltopoprovider() const { return modelTopoProvider; }
	std::string symbolprovider() const { return symbolProvider; }
	std::string symbolProbabilityprovider() const { return symbolProbabilityProvider; }
	
	void symbolprovider(const std::string &provider ){ symbolProvider = provider; }
	void symbolProbabilityprovider(const std::string &provider ){  symbolProbabilityProvider = provider; }
	void forecastprovider( const std::string &provider ) { forecastProvider=provider; }

	float computeTempCorrection( const std::string &provider, int &relTopo, int &modelTopo  )const;
	
	boost::posix_time::ptime time()const { return itTimeSerie->first; }
		
	float windV10m( bool tryHard=false )const;
   float windU10m( bool tryHard=false )const;
   float PP( bool tryHard=false )const;
   float PR( bool tryHard=false )const;
   
   ///Selects one of T2M or T2M_LAND, T2M_LAND has priority over T2M.
   float TA( bool tryHard=false )const;
   float T2M( bool tryHard=false )const;
   float T2M_LAND( bool tryHard=false )const;
   float T2M_NO_ADIABATIC_HIGHT_CORRECTION( bool tryHard=false )const;
   float maxTemperature( int hours, bool tryHard=false )const;
   float minTemperature( int hours, bool tryHard=false )const;
   float temperatureCorrected( bool tryHard = false )const;
   void  temperatureCorrected( float temperature, const std::string &provider = "", bool all=false);
   float wetBulbTemperature( bool tryHard = false )const;
   float UU( bool tryHard=false )const;
   

   bool PRECIP_MIN_MAX_MEAN( int hoursBack, boost::posix_time::ptime &backTime_,
                             float &minOut, float &maxOut, float &meanOut, float probOut,
                             bool tryHard=false )const;

   float PRECIP_MEAN( int hoursBack, boost::posix_time::ptime &backTime_, bool tryHard=false )const;
   float PRECIP_1H( int hoursBack, boost::posix_time::ptime &backTime_, bool tryHard=false )const;
   
   /**
    * PRECIP from accumulated values.  
    */
   float PRECIP_N( int horsBack, boost::posix_time::ptime &fromtime,bool tryHard=false )const;
   
   /**
    * Precip compute precip from available values. 
    * Candidates are acumulated precip or 1 hours values.
    */
   float PRECIP( int horsBack, boost::posix_time::ptime &fromtime,bool tryHard=false )const;
   float NN( bool tryHard=false )const;
   float visibility( bool tryHard=false )const;
   float fog( bool tryHard=false )const;
   float highCloud( bool tryHard=false )const;
   float mediumCloud( bool tryHard=false )const;
   float lowCloud( bool tryHard=false )const;
   float RH2MFromModel( bool tryHard=false )const;
   float humidity( float tempUsed, bool tryHard=false )const;
   float dewPointTemperatureFromModel( bool tryHard=false )const;
   float dewPointTemperature( float usedTemperature, bool tryHard=false )const;
   float thunderProbability( bool tryHard=false )const;
   float fogProbability( bool tryHard=false )const;
   float WIND_PROBABILITY( bool tryHard=false )const;
   float T2M_PROBABILITY_1( bool tryHard=false )const;
   float T2M_PROBABILITY_2( bool tryHard=false )const;
   float T2M_PROBABILITY_3( bool tryHard=false )const;
   float T2M_PROBABILITY_4( bool tryHard=false )const;
   float T2M_PERCENTILE_10( bool tryHard=false )const;
   float T2M_PERCENTILE_25( bool tryHard=false )const;
   float T2M_PERCENTILE_50( bool tryHard=false )const;
   float T2M_PERCENTILE_75( bool tryHard=false )const;
   float T2M_PERCENTILE_90( bool tryHard=false )const;
   float PRECIP_PERCENTILE_10( int horsBack, bool tryHard=false )const;
   float PRECIP_PERCENTILE_25( int horsBack, bool tryHard=false )const;
   float PRECIP_PERCENTILE_50( int horsBack, bool tryHard=false )const;
   float PRECIP_PERCENTILE_75( int horsBack, bool tryHard=false )const;
   float PRECIP_PERCENTILE_90( int horsBack, bool tryHard=false )const;
   float PRECIP_PROBABILITY_0_1_MM( int horsBack, bool tryHard=false )const;
   float PRECIP_PROBABILITY_0_2_MM( int horsBack, bool tryHard=false )const;
   float PRECIP_PROBABILITY_0_5_MM( int horsBack, bool tryHard=false )const;
   float PRECIP_PROBABILITY_1_0_MM( int horsBack, bool tryHard=false )const;
   float PRECIP_PROBABILITY_2_0_MM( int horsBack, bool tryHard=false )const;
   float PRECIP_PROBABILITY_5_0_MM( int horsBack, bool tryHard=false )const;
   float symbol_PROBABILITY( boost::posix_time::ptime &fromTime, bool tryHard=false )const;
   /**
    * Return the symbol. 
    * 
    * The symbol is a parameter that is valid for a period in taime, ie 
    * validfrom and validto should be different. 
    * 
    * In many cases, parameters that is valid for a period, is loaded with 
    * validform equal to validto and we have to guess the validperiod.
    * 
    * @param[out] fromTime On output this is set to the validFrom.
    */
   int   symbol( boost::posix_time::ptime &fromTime, bool tryHard=false )const;
      
   
   float iceingIndex( bool tryHard=false, const std::string &provider=""  )const;

   /*ocean parameters.*/
   float significantSwellWaveHeight( bool tryHard=false )const;
   float meanSwellWavePeriode( bool tryHard=false )const;
   float meanSwellWaveDirection( bool tryHard=false )const;
   float peakSwellWavePeriode( bool tryHard=false )const;
   float peakSwellWaveDirection( bool tryHard=false )const;
   float meanTotalWavePeriode( bool tryHard=false )const;
   float maximumTotalWaveHeight( bool tryHard=false )const;

   float seaCurrentVelocityU( bool tryHard=false )const; 
   float seaCurrentVelocityV( bool tryHard=false )const;
   float seaSalinity( bool tryHard=false )const;
   float seaSurfaceHeight( bool tryHard=false )const;
   float seaTemperature( bool tryHard=false )const;
   float meanTotalWaveDirection( bool tryHard=false )const;
   float significantTotalWaveHeight( bool tryHard=false )const;
   int   seaIcePresence( std::string &usedProvider, const std::string &useProvider="" )const;
   int   seaBottomTopography( std::string &usedProvider, const std::string &useProvider="" )const;
      
	/**
	 * Return INT_MIN if no value is found.
	 */
   int modeltopography( const std::string &provider )const;

	/**
	 * Return INT_MIN if no value is found.
	 */
   int topography( const std::string &provider )const;
   
   /**
    *
    * @param provider
    * @return FLT_MIN if no value is found.
    */
   float landcover( const std::string &provider )const;

   float latitude() const { return latitude_; }
   float longitude() const { return longitude_; }
	
   /**
    * Return INT_MIN if no value is given.
    */
   int height() const { return height_; }
   void height( int h )  { height_ = h; }
	
};

/**
 * PartialData holds data that is needed to correct symbols.
 */
struct PartialData {
   float totalCloud;
   float lowCloud;
   float mediumCloud;
   float highCloud;
   float fog;
   float temperatureCorrected;
   float wetBulbTemperature;
   float latitude;
   float longitude;
   ConfigDataPtr config;
   boost::posix_time::ptime time;

   PartialData()
      : totalCloud( FLT_MAX ),
        lowCloud( FLT_MAX ),
        mediumCloud( FLT_MAX ),
        highCloud( FLT_MAX ),
        fog( FLT_MAX ),
        temperatureCorrected( FLT_MAX ),
        wetBulbTemperature( FLT_MAX ),
        latitude( FLT_MAX ),
        longitude( FLT_MAX )
   {}

   PartialData( const LocationElem &elem ) {
      totalCloud = elem.NN();
      lowCloud = elem.lowCloud();
      mediumCloud = elem.mediumCloud();
      highCloud = elem.highCloud();
      fog = elem.fog();
      temperatureCorrected = elem.temperatureCorrected();
      wetBulbTemperature = elem.wetBulbTemperature();
      latitude = elem.latitude();
      longitude = elem.longitude();
      config = elem.config;
      time = elem.time();

   }
   PartialData( const PartialData &pd )
      : totalCloud( pd.totalCloud ),
        lowCloud( pd.lowCloud ),
        mediumCloud( pd.mediumCloud ),
        highCloud( pd.highCloud ),
        fog( pd.fog ),
        temperatureCorrected( pd.temperatureCorrected ),
        wetBulbTemperature( pd.wetBulbTemperature ),
        latitude( pd.latitude ),
        longitude( pd.longitude ),
        config( pd.config ),
        time( pd.time )
   {}

   std::string url() const{
      if( config ) return config->url;
      else return "";
   }
};



}

#endif /*LOCATIONELEM_H_*/
