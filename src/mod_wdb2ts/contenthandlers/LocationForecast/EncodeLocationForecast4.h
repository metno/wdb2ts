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
#ifndef __ENCODE_LOCATIONFORECAST4_H__
#define __ENCODE_LOCATIONFORECAST4_H__

#include <boost/date_time/posix_time/posix_time.hpp>
#include <string>
#include <PointDataHelper.h>
#include <Indent.h>
#include <LocationData.h>
#include <UpdateProviderReftimes.h>
#include <Encode.h>
#include <SymbolHolder.h>
#include <MetaModelConf.h>
#include <SymbolContext.h>
#include <ProjectionHelper.h>
#include <PrecipitationConfig.h>
#include <Precipitation.h>
#include <SymbolConf.h>
#include <WeatherSymbol.h>

/**
 * @addtogroup wdb2ts
 * @{
 * @addtogroup mod_wdb2ts 
 * @{
 */
/** @file
 * Definition of the EncodeCSV class
 */
namespace wdb2ts {





/**
 * Encode the result as XML, locationforecast schema.
 */
class EncodeLocationForecast4 : public Encode
{
	struct BreakTimes {
		std::string provider;
		boost::posix_time::ptime from;
		boost::posix_time::ptime to;
		boost::posix_time::ptime prevTo;


		BreakTimes( const std::string &provider_);
		BreakTimes( const BreakTimes &bt);

		BreakTimes& operator=(const BreakTimes &rhs );
	};

	typedef std::list<BreakTimes> BreakTimeList;

	static const std::string metatemplate;
	LocationPointDataPtr locationPointData;
	LocationDataPtr locationData;
	const ProjectionHelper *projectionHelper;
	float longitude;
	float latitude;
	int   altitude;
	float tempCorrection;
	boost::posix_time::ptime from;
	ProviderSymbolHolderList symbols;

	
	//The following variables is used to maintain a list
	//of BreakTimes. The list and variables is manipulated in
	//the function updateBreakTimes.
	BreakTimeList breakTimes;
	BreakTimeList::iterator curItBreakTimes;
	std::string breakTimeForecastProvider;
	boost::posix_time::ptime prevBreakTime;
	PtrProviderRefTimesByDbId refTimes;

	//Metadata configuration.
	MetaModelConfList &metaConf;
	
	//The precipitation configuration.
	ProviderPrecipitationConfig *precipitationConfig;

	const ProviderList &providerPriority;
	const TopoProviderMap &modelTopoProviders;
	const std::list<std::string>  &topographyProviders;
	const SymbolConfProvider &symbolConf;


	int nElements;

	void encodePrecipitationPercentiles( const boost::posix_time::ptime &from, 
			                             std::ostream &ost,
					                     miutil::Indent &indent );

	
	void encodeMoment( const boost::posix_time::ptime &from, 
				       std::ostream &out,
			           miutil::Indent &indent );



	bool encodePeriods( LocationElem &elem,
			             WeatherSymbolDataBuffer &weatherData,
			            const SymbolConfList &symbolConf,
				        //const PrecipConfigElement &precipConfig,
				        std::ostream &ost,
				        miutil::Indent &indent,
						  bool doOutputXml=true);

	int getSymbol( LocationElem &location, const boost::posix_time::ptime &fromTime )const;
	float getSymbolProbability( LocationElem &location,
							    const boost::posix_time::ptime &fromTime ) const;

	void encodeMeta( std::string &result );
	
	void 	updateBreakTimes( const std::string &provider,
			                  const boost::posix_time::ptime &time );
	
	/**
	 * comapactBreakTimes scans the breaktimes list to find subsequent
	 * providers with the same name. This providers is collapsed to one.
	 */
	void 	compactBreakTimes();

	BreakTimeList::const_iterator findBreakTime( const boost::posix_time::ptime &time )const;
		
	//boost::shared_ptr<SymbolHolder> findSymbolHolder( const std::string &provider, int timespan );

	
public:
	/// Default Constructor
	EncodeLocationForecast4( );
	/**
	 * Constructor given result set and geographical position
	 * The EncodeLocationForcast take over the responsibility 
	 * of the timeSerie pointer an delete at the end.
	 * 
	 * @param	timeSerie	The timeSerie.
	 */
	EncodeLocationForecast4( LocationPointDataPtr locationPointData,
	                         const ProjectionHelper *projectionHelper,
	                         float longitude,
	                         float latitude,
	                         int   altitude,
	                         const boost::posix_time::ptime &from,
	                         PtrProviderRefTimesByDbId refTimes,
	                         MetaModelConfList &metaConf,
	                         ProviderPrecipitationConfig *precipitationConfig,
	                         const ProviderList &providerPriority,
	                         const TopoProviderMap &modelTopoProviders,
	                         const std::list<std::string> &topographyProviders,
	                         const SymbolConfProvider &symbolConf
		                  );

	
	/// Destructor
	virtual ~EncodeLocationForecast4();
    
		
	/**
	 * Which xml schema that is used.
	 */
	std::string schemaName()const {
         if( schema_.empty() )
	         return "http://api.met.no/weatherapi/locationforecast/1.5/schema";
         return schema_;
	}
	/**
	 * Encode the Result Set as XML.
	 * @param	out		Stream on which the encoded result is returned
	 * @exception	logic_error
	 */
    virtual void encode(  webfw::Response &response );
};

} // namespace

/**
 *  @} 
 *  @} 
 */

#endif 
