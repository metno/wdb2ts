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
#ifndef __ENCODE_LOCATIONFORECAST3_GML_H__
#define __ENCODE_LOCATIONFORECAST3_GML_H__

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
#include <ProjectionHelperConfigure.h>
#include <contenthandlers/LocationForecastGml/GMLContext.h>
#include <PrecipitationConfig.h>
#include <SymbolConfConfigure.h>
#include <wdb2TsApp.h>
#include <Precipitation.h>
#include <RequestIterator.h>

/**
 * @addtogroup wdb2ts
 * @{
 * @addtogroup mod_wdb2ts 
 * @{
 */
/** @file
 */
namespace wdb2ts {





/**
 * Encode the result as XML, locationforecast schema.
 */
class EncodeLocationForecastGml3 : public Encode
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
	RequestIterator &reqit;
	LocationPointDataPtr locationPointData;
	LocationDataPtr      locationData;
	GMLContext   gmlContext;

	float tempCorrection;
	boost::posix_time::ptime from;
	boost::posix_time::ptime createdTime; 
	ProviderSymbolHolderList symbols;
	
	//The following variables is used to maintain a list
	//of BreakTimes. The list and variables is manipulated in
	//the function updateBreakTimes.
	BreakTimeList breakTimes;
	BreakTimeList::iterator curItBreakTimes;
	std::string breakTimeForecastProvider;
	boost::posix_time::ptime prevBreakTime;

	//Metadata configuration.
	MetaModelConfList &metaConf;
	
	//The precipitation configuration.
	ProviderPrecipitationConfig *precipitationConfig;

	const TopoProviderMap &modelTopoProviders;
	const std::list<std::string>  &topographyProviders;
	const SymbolConfProvider &symbolConf;

	//Expire randomization.
	int expireRand;
	
	float longitude, latitude;
	int altitude;

	int nElements;

	void encodeSymbols( std::ostream &out, 
					        miutil::Indent &indent );

	void encodePrecipitation( const boost::posix_time::ptime &from, 
				              const std::vector<int> &precipHours,
				              std::ostream &out,
				              miutil::Indent &indent );

	void encodePrecipitationPercentiles( const boost::posix_time::ptime &from, 
			                             std::ostream &ost,
					                     miutil::Indent &indent );

	bool encodeFeatureMemberTag( const LocationElem &location,
		         					  std::ostream &ost, miutil::Indent &indent );
	
	void encodeMoment( const boost::posix_time::ptime &from, 
   			           std::ostream &out,
			           miutil::Indent &indent );

	void encodePeriods( const boost::posix_time::ptime &from,
			            std::ostream &ost,
			            miutil::Indent &indent );

	
	void encodeHeader( std::string &result );

	void encodeMeta( std::string &result );
	
	void updateBreakTimes( const std::string &provider,
		                   const boost::posix_time::ptime &time );
	
	void computePrecipitationMulti( const boost::posix_time::ptime &from,
                                   const std::vector<int> &hours,
			                       PrecipitationAggregations &aggregates
	                             )const;


	BreakTimeList::const_iterator findBreakTime( const boost::posix_time::ptime &time )const;
		
	boost::shared_ptr<SymbolHolder> findSymbolHolder( const std::string &provider, int timespan ); 

	
public:
	/// Default Constructor
	EncodeLocationForecastGml3( );
	/**
	 * Constructor given result set and geographical position
	 * The EncodeLocationForcast take over the responsibility 
	 * of the timeSerie pointer an delete at the end.
	 * 
	 * @param	timeSerie	The timeSerie.
	 */
	EncodeLocationForecastGml3( RequestIterator &reqit,
                               const ProjectionHelper *projectionHelper,
			                      const boost::posix_time::ptime &from,
			                      MetaModelConfList &metaConf,
			                      ProviderPrecipitationConfig *precipitationConfig,
			                      const TopoProviderMap &modelTopoProviders,
			                      const std::list<std::string>  &topographyProviders,
			                      const SymbolConfProvider &symbolConf,
			                      int expire_rand
			         		   );

	
	/// Destructor
	virtual ~EncodeLocationForecastGml3();
    
		
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
