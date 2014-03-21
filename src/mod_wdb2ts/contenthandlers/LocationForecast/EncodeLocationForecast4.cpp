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

/**
 * @addtogroup wdb2ts 
 * @{
 * @addtogroup mod_wdb2ts 
 * @{
 */
/** @file
 * Implementation of the EncodeLocationForecast3 class
 */
#ifdef HAVE_CONFIG_H
#include <wdb2ts_config.h>
#endif
// CLASS
#include <contenthandlers/LocationForecast/EncodeLocationForecast4.h>

// PROJECT INCLUDES
#include <ptimeutil.h>
#include <WeatherdataTag.h>
#include <ProductTag.h>
#include <TimeTag.h>
#include <MomentTags1.h>
#include <LocationTag.h>
#include <PrecipitationTags.h>
#include <PrecipitationPercentileTags.h>
#include <Indent.h>
#include <replace.h>
#include <wdb2tsProfiling.h>
#include <ptimeutil.h>
#include <probabilityCode.h>
#include <ProviderList.h>
#include <SymbolGenerator.h>
#include <Logger4cpp.h>

// SYSTEM INCLUDES
//
#include <stdlib.h>
#include <time.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <map>
#include <iterator>

using namespace std;

using namespace miutil;
using namespace boost::posix_time;

DEFINE_MI_PROFILE;

namespace {
	wdb2ts::ProviderList dummyProviderPriority;
	wdb2ts::TopoProviderMap dummyTopoProvider;
	//wdb2ts::LocationData dummy(wdb2ts::TimeSeriePtr(), FLT_MIN, FLT_MAX, INT_MIN, dummyProviderPriority,dummyTopoProvider );
	wdb2ts::MetaModelConfList dummyMetaConf;
	std::list<std::string> dummyTopoList;
	wdb2ts::SymbolConfProvider dummySymbolConfProvider;
}

namespace wdb2ts {

const std::string EncodeLocationForecast4::metatemplate("<!--@@META@@-->");

EncodeLocationForecast4::
BreakTimes::
BreakTimes( const std::string &provider_)
	: provider( provider_ )
{
}

EncodeLocationForecast4::
BreakTimes::
BreakTimes( const BreakTimes &bt )
	: provider( bt.provider ), from( bt.from), to( bt.to ), prevTo( bt.prevTo )
{
}

EncodeLocationForecast4::
BreakTimes &
EncodeLocationForecast4::
BreakTimes::
operator=(const BreakTimes &rhs )
{
	if( this != &rhs ) {
		provider = rhs.provider;
		from = rhs.from;
		to = rhs.to;
		prevTo = rhs.prevTo;
	}
	
	return *this;
}


EncodeLocationForecast4::
EncodeLocationForecast4():
	metaConf( dummyMetaConf ),
	providerPriority( dummyProviderPriority ), modelTopoProviders( dummyTopoProvider ),
	topographyProviders( dummyTopoList ),
	symbolConf( dummySymbolConfProvider ),
	nElements( 0 )
{
	// NOOP
}

EncodeLocationForecast4::
EncodeLocationForecast4( LocationPointDataPtr locationPointData_,
                        const ProjectionHelper *projectionHelper_,
                        float longitude,
                        float latitude,
                        int   altitude,
                        const boost::posix_time::ptime &from,
                        PtrProviderRefTimes refTimes_,
                        MetaModelConfList &metaConf_,
                        ProviderPrecipitationConfig *precipitationConfig_,
                        const ProviderList &providerPriority_,
                        const TopoProviderMap &modelTopoProviders_,
                        const std::list<std::string> &topographyProviders_,
                        const SymbolConfProvider &symbolConf_,
                        int expire_rand )
   : locationPointData( locationPointData_ ), projectionHelper( projectionHelper_ ),
     longitude( longitude ), latitude( latitude ),
     altitude( altitude ), tempCorrection( 0.0 ), from( from ),
     refTimes( refTimes_ ), metaConf( metaConf_ ),
     precipitationConfig( precipitationConfig_ ),
     providerPriority( providerPriority_ ),
     modelTopoProviders( modelTopoProviders_),
     topographyProviders( topographyProviders_ ),
     symbolConf( symbolConf_ ),
     expireRand( expire_rand ),
     nElements( 0 )
{
}

EncodeLocationForecast4::
~EncodeLocationForecast4()
{
	// NOOP
}   


EncodeLocationForecast4::BreakTimeList::const_iterator
EncodeLocationForecast4::
findBreakTime( const boost::posix_time::ptime &time )const
{
	if( time.is_special() )
		return breakTimes.begin();
	
	BreakTimeList::const_iterator it;
	
	for( it=breakTimes.begin(); it != breakTimes.end(); ++it )
	{
		if( time >= it->from && time <= it->to )
			return it;
	}
	
	it = breakTimes.begin(); 
	
	if( it != breakTimes.end() && time < it->from )
		return it;
	
	return breakTimes.end();
}


void
EncodeLocationForecast4::
compactBreakTimes()
{
	BreakTimeList::iterator it=breakTimes.begin();
	BreakTimeList::iterator itFirst=breakTimes.begin();
	BreakTimeList::iterator itPrev=it;
	ProviderItem piFirst;
	ProviderItem pi;

	if( it == breakTimes.end() )
		return;

	piFirst = ProviderList::decodeItem( itFirst->provider );

	for( ++it ; it != breakTimes.end(); ++it )
	{
		pi = ProviderList::decodeItem( it->provider );

		if(  pi.provider != piFirst.provider ) {
			if( std::distance( itFirst, itPrev ) > 0 ) {
				itFirst->to = itPrev->to;

				for( ++itFirst; itFirst != itPrev; ++itFirst )
					breakTimes.erase( itFirst );

				breakTimes.erase( itPrev );
			}

			itFirst = it;
			piFirst = ProviderList::decodeItem( itFirst->provider );
		}

		itPrev = it;
	}
}


//boost::shared_ptr<SymbolHolder>
//EncodeLocationForecast4::
//findSymbolHolder( const std::string &provider, int timespan )
//{
//	ProviderSymbolHolderList::iterator ii = symbols.find( provider );
//
//	if( ii == symbols.end() )
//		return boost::shared_ptr<SymbolHolder>();
//
//	for( SymbolHolderList::iterator it = ii->second.begin();
//		  it != ii->second.end();
//		  ++it )
//		if( (*it)->timespanInHours() == timespan )
//			return *it;
//
//	return boost::shared_ptr<SymbolHolder>();
//
//}






void
EncodeLocationForecast4::
encodePrecipitationPercentiles( const boost::posix_time::ptime &from, 
			                       std::ostream &ost, 
			                       miutil::Indent &indent )
{
	WEBFW_USE_LOGGER( "encode" );
	log4cpp::Priority::Value loglevel = WEBFW_GET_LOGLEVEL();

	int hours[] = { 6, 12, 24 };
	int n=sizeof( hours )/sizeof(hours[0]);
	ptime fromTime;
	ptime toTime;
	bool first;
	string prevProvider;
	string provider;
	ostringstream percentileOst;
	ostringstream outOst;

	percentileOst.flags( ost.flags() );
	percentileOst.precision( ost.precision() );
	
	outOst.flags( ost.flags() );
	outOst.precision( ost.precision() );
	
	for( int i=0; i<n; ++i ) {
		if( ! locationData->init( from ) )
			return;
			
		prevProvider.erase();	
		first = true;
		percentileOst.str("");
		
		while( locationData->hasNext() ) {
			LocationElem& location = *locationData->next();
			
			if( ! percentileOst.str().empty() ) {
			   ++nElements;
				ost << outOst.str();
				percentileOst.str("");
			}
		
			outOst.str("");
			
			IndentLevel level3( indent );
			provider = location.forecastprovider();
				
			if( prevProvider != provider && loglevel >= log4cpp::Priority::DEBUG )
				ost << level3.indent() << "<!-- Dataprovider: " << provider << " -->\n";
				
			prevProvider = provider;
				
			if( first ) {
				if( loglevel >= log4cpp::Priority::DEBUG )
					ost << level3.indent() << "<!-- PrecipPercentiles: " << hours[i] << " hours -->\n";

				first = false;
			}
			
			toTime = location.time();
			fromTime = toTime - boost::posix_time::hours( hours[i] );
			
			TimeTag timeTag( 	fromTime, toTime );
			timeTag.output( outOst, level3.indent() );
								
			IndentLevel level4( indent );
			LocationTag locationTag( latitude, longitude, altitude );
			locationTag.output( outOst, level4.indent() );
						
			IndentLevel level5( indent );
						
			PrecipitationPercentileTags precipPercentileTag( location, hours[i] );
			precipPercentileTag.output( percentileOst, level5.indent() );
			
			if( ! percentileOst.str().empty() )  
				outOst << percentileOst.str();
				
		}

		//My have one left over.
		if( ! percentileOst.str().empty() ) {
		   ++nElements;
			ost << outOst.str();
		}
	
	}
}

void 
EncodeLocationForecast4::
updateBreakTimes( const std::string &provider, const boost::posix_time::ptime &time)
{
	WEBFW_USE_LOGGER( "encode" );

	if(	breakTimeForecastProvider != provider ) {
		if( ! provider.empty() ) {
			WEBFW_LOG_DEBUG( "encode::updateBreakTimes: provider: '" << provider << "' "
					         << "breakTimeProvider: '" << breakTimeForecastProvider << "'");
			breakTimeForecastProvider = provider;
			BreakTimeList::iterator itBreakTimes = curItBreakTimes;
			curItBreakTimes = breakTimes.insert( breakTimes.end(),
						                         BreakTimes( breakTimeForecastProvider )
		                                   ); 
			curItBreakTimes->from = time;
			if( ! prevBreakTime.is_special() )
				curItBreakTimes->prevTo = prevBreakTime;
		
			if( itBreakTimes != breakTimes.end() )
				itBreakTimes->to = prevBreakTime;
		} else {
			WEBFW_LOG_DEBUG( "encode::updateBreakTimes: EMPTY provider.");
		}
	}
	
	curItBreakTimes->to = time;
	prevBreakTime = time;
}



void  
EncodeLocationForecast4::
encodeMoment( const boost::posix_time::ptime &from, 
			     std::ostream &ost, 
		        miutil::Indent &indent )
{
    WEBFW_USE_LOGGER( "encode" );
    log4cpp::Priority::Value loglevel = WEBFW_GET_LOGLEVEL();

    int countProviderChange=0; //used to stop endless loops.
	CIProviderPDataList itPData;
	CIFromTimeSerie itFromTimeserie;
	ostringstream tmpOst;
	ostringstream momentOst;
	WeatherSymbolDataBuffer symbolDataBuffer;
	PrecipitationAggregations precipAggregates;
	PrecipConfigElement precip;
	SymbolConfList symbolConfList;
	boost::posix_time::ptime dataFrom;
	boost::posix_time::ptime currentTime;
	boost::posix_time::ptime currentFrom( from );
	string prevForecastprovider;
	bool hasMomentData;
		
	tmpOst.flags( ost.flags() );
	tmpOst.precision( ost.precision() );
	
	momentOst.flags( ost.flags() );
	momentOst.precision( ost.precision() );
	
	dataFrom = from - boost::posix_time::hours( symbolConf.maxHours() );

	currentTime = dataFrom;
	//We must collect data from 'from' - time
	//to fill up symbolDataBuffer with data so
	//we can generate Symbols from the first
	//dataset in the output, but we shall not
	//deliver the data between 'dataFrom' to 'from'
	//in the output.
	if( ! locationData->init( dataFrom ) )
		return;

	curItBreakTimes = breakTimes.end();
	
	while( locationData->hasNext() ) {
		SymbolDataElement symbolData;
		LocationElem &location = *locationData->next();
		location.config = config_;
		
		if( currentTime >= currentFrom && ! momentOst.str().empty() ) {
		   ++nElements;
			ost << tmpOst.str();
			momentOst.str("");
		}
			
		currentTime = location.time();
		tmpOst.str("");
		momentOst.str("");

		{ //Begin Block
			IndentLevel level3( indent );
			TimeTag timeTag( location.time(), location.time() );
			timeTag.output( tmpOst, level3.indent() );
	
			IndentLevel level4( indent );
			LocationTag locationTag( latitude, longitude, altitude );
			locationTag.output( tmpOst, level4.indent() );
	
			IndentLevel level5( indent );
			MomentTags1 momentTags( location, symbolData, projectionHelper );
			momentTags.output( momentOst, level5.indent() );
		
			hasMomentData = false;
			if( ! momentOst.str().empty() ) {
				hasMomentData = true;

				if( currentTime >= currentFrom )
					updateBreakTimes( location.forecastprovider(), location.time() );

				if( prevForecastprovider.empty() )
					prevForecastprovider = location.forecastprovider();
				else if( prevForecastprovider != location.forecastprovider()
						 && countProviderChange < 10) {
					++countProviderChange;
					if( loglevel >= log4cpp::Priority::DEBUG ) {
						ost << "<!-- Change provider: " <<  prevForecastprovider<< " -> " << location.forecastprovider()
							<< " (" << location.time() << ") -->\n";
					}
					prevForecastprovider = location.forecastprovider();
					currentFrom =  location.time();
					dataFrom =  currentFrom - boost::posix_time::hours( symbolConf.maxHours() );
					locationData->init( dataFrom, location.forecastprovider() );
					hasMomentData = false;
					momentOst.str("");
					symbolDataBuffer.clear();
				}

				if( hasMomentData ) {
//					ost << "<!-- Add symboldata: " << location.time() << " ("  << location.forecastprovider()
//					       << ") -->\n";
					symbolDataBuffer.add( location.time(), symbolData );
					tmpOst << momentOst.str();
				}

			}
		} //End Block

		if( hasMomentData && currentTime >= currentFrom   ) {
			symbolConfList = symbolConf.get( location.forecastprovider() );

			if( loglevel >= log4cpp::Priority::DEBUG ) {
				tmpOst << "<!-- provider: " <<  location.forecastprovider() << "\n";

				for( int i=0; i < symbolConfList.size(); ++i ) {
					tmpOst << "  symbolConf [" << i << "]: " << symbolConfList[i] << "\n";
				}
				tmpOst << " -->\n";
			}

			encodePeriods( location, symbolDataBuffer,
						   symbolConfList , tmpOst, indent );
		}

	}

	//May have one leftover.
	if( ! momentOst.str().empty() && currentTime >= currentFrom) {
	   ++nElements;
		ost << tmpOst.str();
	}
}



bool
EncodeLocationForecast4::
encodePeriods( LocationElem &location,
	    	   const WeatherSymbolDataBuffer &weatherData,
  		      // const PrecipConfigElement &precipConfig,
  		       const SymbolConfList &symbolConf,
		       std::ostream &ost,
		       miutil::Indent &indent )
{
	WEBFW_USE_LOGGER( "encode" );
	int N = symbolConf.size();
	float precip;
	float precipMin;
	float precipMax;
	float precipProb;
	SymbolDataElement symbolData;
	ptime fromTime;
	ptime toTime = location.time();
	string provider = location.forecastprovider();
	string oldSymbolProvider = location.symbolprovider();
	int symbolCode;
	float symbolProbability;
	bool hasData = false;

	if( weatherData.begin() == weatherData.end() )
		return hasData;

	location.symbolprovider( provider );
	for( int symbolIndex=0; symbolIndex < N; ++symbolIndex ) {
	   precip = FLT_MAX;
	   precipMin = FLT_MAX;
	   precipMax = FLT_MAX;
	   precipProb = FLT_MAX;
	   symbolCode = weather_symbol::Error;

	   if( ! location.PRECIP_MIN_MAX_MEAN( symbolConf[symbolIndex].precipHours(), fromTime, precipMin, precipMax, precip, precipProb ) )
	      precip = location.PRECIP( symbolConf[symbolIndex].precipHours(), fromTime);


	   symbolCode = location.symbol( fromTime );
	   symbolProbability = location.symbol_PROBABILITY( fromTime );

	   if( precip == FLT_MAX && symbolCode == INT_MAX )
		   continue;

	   if( symbolCode != INT_MAX )
		   symbolData = computeWeatherSymbol(
				   weatherData, symbolConf[symbolIndex].precipHours(),
				   weather_symbol::Code( symbolCode )
		   	   );
	   else
		   symbolData = computeWeatherSymbol( weatherData, symbolConf[symbolIndex].precipHours(),
			                              precip, precipMin, precipMax );

	   if( symbolData.weatherCode == weather_symbol::Error ) {
		   	WEBFW_LOG_DEBUG( "encodePeriods: ERROR_SYMBOL (" <<  symbolConf[symbolIndex].precipHours() << " "
		   			       << " ) " << fromTime <<  " - " << location.time() << " '" <<
		   			       provider << "' " <<  symbolData
		   			       );
		   	continue;

	   }

	   hasData = true;
	   IndentLevel level3( indent );
	   TimeTag timeTag( symbolData.from, toTime );
	   timeTag.output( ost, level3.indent() );

	   IndentLevel level4( indent );
	   LocationTag locationTag( latitude, longitude, altitude );
	   locationTag.output( ost, level4.indent() );

	   IndentLevel level5( indent );
	   if( precip != FLT_MAX) {
		   PrecipitationTags precipitationTag( precip, precipMin, precipMax, FLT_MAX );
		   precipitationTag.output( ost, level5.indent() );
	   }
	   ost << "<symbol id=\"" << weather_symbol::name( symbolData.weatherCode ) <<"\" number=\"" << symbolData.weatherCode << "\"/>\n";

	   if( symbolProbability != FLT_MAX )
		   ost << "<symbolProbability unit=\"probabilitycode\" value=\"" << probabilityCode( symbolProbability ) << "\"/>\n";

	}
	location.symbolprovider( oldSymbolProvider );
	return hasData;
}






void 
EncodeLocationForecast4::
encodeMeta( std::string &result )
{
	Indent indent;
	IndentLevel level2( indent );
	ProviderRefTimeList::iterator itRefTime;
	string name;
	boost::posix_time::ptime nextrun;
	boost::posix_time::ptime runEnded;
	boost::posix_time::ptime termin;
	ostringstream ost;
	
	if( breakTimes.empty() ) {
		replaceString( result, metatemplate, "" );
		return;
	}
	
	compactBreakTimes();

	ost << level2.indent() << "<meta>\n";
	
	{
		IndentLevel level3( indent );
		MetaModelConfList::iterator itMeta;
		BreakTimeList::iterator it;
		
		for( BreakTimeList::iterator it = breakTimes.begin(); 
		     it != breakTimes.end();
		     ++it )
		{
			itMeta = metaConf.find( it->provider );
			
			//Try to find the provider part of the provider. Remember
			//a provider string may be on the form 'provider [placename]'.
			if( itMeta == metaConf.end() ) {
				ProviderItem pi = ProviderList::decodeItem( it->provider );
				itMeta = metaConf.find( pi.provider );
			}
			
			if( itMeta == metaConf.end() ) {
				name = it->provider;
				nextrun = boost::posix_time::ptime(); //Undef
			} else {
				name = itMeta->second.name();
				
				if( name.empty() )
					name = it->provider;
				
				nextrun = itMeta->second.findNextrun();
			}
			
			termin = boost::posix_time::ptime(); //Undef
			runEnded = boost::posix_time::ptime(); //Undef
			itRefTime = refTimes->find( it->provider );
			
			if( itRefTime != refTimes->end() ) { 
				termin = itRefTime->second.refTime;
				runEnded = itRefTime->second.updatedTime;
			}
			
			ost << level3.indent() << "<model name=\"" << name <<"\"";
			
			if( ! termin.is_special() )
				ost << " termin=\"" << isotimeString( termin, true, true ) << "\"";
			
			if( ! runEnded.is_special() )
				ost << " runended=\"" << isotimeString( runEnded, true, true ) << "\"";
			
			
			if( ! nextrun.is_special() )
				ost << " nextrun=\"" << isotimeString( nextrun, true, true ) << "\"";
				
			ost << " from=\"" << isotimeString( it->from, true, true ) << "\"";
			ost << " to=\"" << isotimeString( it->to, true, true ) << "\"";
			ost << " />\n";
		}
		
		ost << level2.indent() << "</meta>\n";
	}
	
	replaceString( result, metatemplate, ost.str() );
}

void  
EncodeLocationForecast4::
encode(  webfw::Response &response )
{
	miutil::Indent indent;
	ostringstream ost;
	string        result;
	ptime validFrom;
	ptime validTo;
	ptime expire;
	string error;

 	USE_MI_PROFILE;
 	MARK_ID_MI_PROFILE("EncodeXML");
 	WEBFW_USE_LOGGER( "encode" );
 	
 	nElements = 0;

 	//Use one decimal precision as default.
 	ost.setf(ios::floatfield, ios::fixed);
 	ost.precision(1);
 	   
 	unsigned int seed=static_cast<unsigned int>(time(0));
 	
 	response.contentType("text/xml");
 	response.directOutput( true );
   
 	expire = second_clock::universal_time();
	expire += hours( 1 );
	expire = ptime( expire.date(), 
	                time_duration( expire.time_of_day().hours(), 0, 0, 0 ));
   
	if( expireRand > 0)
		expire = expire + seconds( rand_r( &seed ) % expireRand );
	
	
	WEBFW_LOG_DEBUG( "encode: Number of locations: " << locationPointData->size() );

	if( locationPointData->empty() ) {
		locationData.reset( new LocationData() );
	} else if ( locationPointData->size() == 1 ){
		locationData.reset( new LocationData( locationPointData->begin()->second, longitude, latitude, altitude,
					                               providerPriority, modelTopoProviders, topographyProviders ) );

		if( altitude == INT_MIN ) {

			altitude = locationData->hightFromTopography();
			if( altitude != INT_MIN )  {
				WEBFW_LOG_DEBUG("encode: Height from topograpy: " << altitude )
			}

		}

		if( altitude == INT_MIN ) {
			altitude = locationData->hightFromModelTopo();
			if( altitude != INT_MIN )  {
				WEBFW_LOG_DEBUG("encode: Height from model topograpy: " << altitude )
			}
		}

		if( altitude != INT_MIN ) {
			locationData->height( altitude );
		}

	} else {
		WEBFW_LOG_ERROR( "Polygon not supported.");
		locationData.reset( new LocationData() );
	}

	//Compute the symbols withoutStateOfAgregate. We correct the symbols later
	//when the temperature is known.
	//symbols = SymbolGenerator::computeSymbols( *locationData, symbolConf, true, error );

	{ //Create a scope for the weatherdatatag and producttag.
		WeatherdataTag weatherdataTag(second_clock::universal_time(), schemaName() );
		weatherdataTag.output( ost, indent.spaces() );
		
		//Make room for the meta tag.
		ost << metatemplate;

		IndentLevel level2( indent );
		ProductTag productTag;
		productTag.output( ost, level2.indent() );
		
		encodeMoment(  from, ost, indent );
		encodePrecipitationPercentiles( from, ost, indent );

	}

	if( nElements == 0 ) {
	   throw NoData();
	}

	result = ost.str();
	
	encodeMeta( result );
	
	MARK_ID_MI_PROFILE("EncodeXML");
	response.expire( expire );
	response.contentLength( result.size() );
	
	MARK_ID_MI_PROFILE("SendXML");
	response.out() << result;
	MARK_ID_MI_PROFILE("SendXML");
}

/**
 * @}
 * 
 * @}
 */

} // namespace
