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
 * Implementation of the EncodeLocationForecast2 class
 */
#ifdef HAVE_CONFIG_H
#include <wdb2ts_config.h>
#endif
// CLASS
#include <contenthandlers/LocationForecast/EncodeLocationForecast2.h>

// PROJECT INCLUDES
#include <ptimeutil.h>
#include <WeatherdataTag.h>
#include <ProductTag.h>
#include <TimeTag.h>
#include <MomentTags.h>
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

const std::string EncodeLocationForecast2::metatemplate("<!--@@META@@-->");

EncodeLocationForecast2::
BreakTimes::
BreakTimes( const std::string &provider_)
	: provider( provider_ )
{
}

EncodeLocationForecast2::
BreakTimes::
BreakTimes( const BreakTimes &bt )
	: provider( bt.provider ), from( bt.from), to( bt.to ), prevTo( bt.prevTo )
{
}

EncodeLocationForecast2::
BreakTimes &
EncodeLocationForecast2::
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


EncodeLocationForecast2::
EncodeLocationForecast2():
	metaConf( dummyMetaConf ),
	symbolContext( false),
	providerPriority( dummyProviderPriority ), modelTopoProviders( dummyTopoProvider ),
	topographyProviders( dummyTopoList ),
	symbolConf( dummySymbolConfProvider )
{
	// NOOP
}

EncodeLocationForecast2::
EncodeLocationForecast2( LocationPointDataPtr locationPointData_,
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
     symbolContext( false),
     precipitationConfig( precipitationConfig_ ),
     providerPriority( providerPriority_ ),
     modelTopoProviders( modelTopoProviders_),
     topographyProviders( topographyProviders_ ),
     symbolConf( symbolConf_ ),
     expireRand( expire_rand )
{
}

EncodeLocationForecast2::
~EncodeLocationForecast2()
{
	// NOOP
}   


EncodeLocationForecast2::BreakTimeList::const_iterator
EncodeLocationForecast2::
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
EncodeLocationForecast2::
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


boost::shared_ptr<SymbolHolder>
EncodeLocationForecast2::
findSymbolHolder( const std::string &provider, int timespan )
{
	ProviderSymbolHolderList::iterator ii = symbols.find( provider );
	
	if( ii == symbols.end() ) 
		return boost::shared_ptr<SymbolHolder>();

	for( SymbolHolderList::iterator it = ii->second.begin();
		  it != ii->second.end();
		  ++it )
		if( (*it)->timespanInHours() == timespan )
			return *it;
	
	return boost::shared_ptr<SymbolHolder>();
	
}






void
EncodeLocationForecast2::
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
		if( ! percentileOst.str().empty() )
			ost << outOst.str();
	
	}
}

void 
EncodeLocationForecast2::
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
EncodeLocationForecast2::
updatePrecipAggregates( LocationElem &location,
		                const PrecipConfigElement &precipConfig,
		                PrecipitationAggregations &aggregates )
{
//	WEBFW_USE_LOGGER( "encode" );
	int N = precipConfig.precipHours.size();
	float precip;
	ptime fromTime = location.time();
	string provider = location.forecastprovider();

	for( int precipIndex=0; precipIndex < N; ++precipIndex ) {
		precip = location.PRECIP( precipConfig.precipHours[precipIndex], fromTime);

		if( precip != FLT_MAX ) {
//			WEBFW_LOG_DEBUG( "updatePrecipAggregates: " <<  precipConfig.precipHours[precipIndex] << " "
//					        << fromTime <<  "  " << provider << " " << precip);
			aggregates[ precipConfig.precipHours[precipIndex] ] [ fromTime ] = Precipitation( provider, precip );

			if( precipConfig.precipType == PrecipSequence )
				return;
		}
	}
}


void  
EncodeLocationForecast2::
encodeMoment( const boost::posix_time::ptime &from, 
			     std::ostream &ost, 
		        miutil::Indent &indent )
{
    WEBFW_USE_LOGGER( "encode" );
	CIProviderPDataList itPData;
	CIFromTimeSerie itFromTimeserie;
	ostringstream tmpOst;
	ostringstream momentOst;
	PrecipitationAggregations precipAggregates;
	PrecipConfigElement precip = precipitationConfig->getDefault();
	bool hasMomentData;
		
	tmpOst.flags( ost.flags() );
	tmpOst.precision( ost.precision() );
	
	momentOst.flags( ost.flags() );
	momentOst.precision( ost.precision() );
	
	if( ! locationData->init( from ) )
		return;

	curItBreakTimes = breakTimes.end();
	
	while( locationData->hasNext() ) {
		LocationElem &location = *locationData->next();
		
		if( ! momentOst.str().empty() ) {
			ost << tmpOst.str();
			momentOst.str("");
		}
			
		tmpOst.str("");
		momentOst.str("");
		{
			IndentLevel level3( indent );
			TimeTag timeTag( location.time(), location.time() );
			timeTag.output( tmpOst, level3.indent() );
	
			IndentLevel level4( indent );
			LocationTag locationTag( latitude, longitude, altitude );
			locationTag.output( tmpOst, level4.indent() );
	
			IndentLevel level5( indent );
			MomentTags momentTags( location, symbolContext, projectionHelper );
			momentTags.output( momentOst, level5.indent() );
		
			hasMomentData = false;
			if( ! momentOst.str().empty() ) {
				//symbolContext.update( symbols );
				updatePrecipAggregates( location, precip, precipAggregates );
				updateBreakTimes( location.forecastprovider(), location.time() );
				tmpOst << momentOst.str();
				hasMomentData = true;
			}
		}

		if( hasMomentData  )
			encodePeriods( location,
						   precipAggregates, symbols, tmpOst, indent );

	}

	//May have one leftover.
	if( ! momentOst.str().empty() ) 
		ost << tmpOst.str();
}




bool
EncodeLocationForecast2::
encodePeriods( LocationElem &elem,
			   const PrecipitationAggregations &aggregates,
			   const ProviderSymbolHolderList &symbols,
			   std::ostream &ost,
			   miutil::Indent &indent )
{

	boost::posix_time::ptime totime = elem.time();
	std::string forecastProvider = elem.forecastprovider();
	//WEBFW_USE_LOGGER( "encode" );
	std::set<boost::posix_time::ptime> fromtimes;
	Precipitation precip;
	SymbolHolder::Symbol symbol;
	bool hasData = false;

	aggregates.findAllFromtimes( totime, forecastProvider, fromtimes );

	symbols.findAllFromtimes( totime, forecastProvider, fromtimes );

	//WEBFW_LOG_DEBUG( "encodePeriods: " << totime << " " << forecastProvider);

	for( std::set<boost::posix_time::ptime>::const_reverse_iterator it = fromtimes.rbegin();
	     it != fromtimes.rend(); ++it )
	{
		IndentLevel level3( indent );

		TimeTag timeTag( *it, totime );
		timeTag.output( ost, level3.indent() );

		IndentLevel level4( indent );
		LocationTag locationTag( latitude, longitude, altitude );
		locationTag.output( ost, level4.indent() );

		IndentLevel level5( indent );

		if( aggregates.findPrecip( forecastProvider, *it, totime, precip ) ) {
			hasData = true;
			PrecipitationTags precipitationTag( precip.precip );
			precipitationTag.output( ost, level5.indent() );
		}

		if( symbols.findSymbol(forecastProvider, *it, totime, symbol ) ) {
			hasData = true;
			SymbolGenerator::correctSymbol( symbol, elem );
			ost << "<symbol id=\"" << symbol.idname() <<"\" number=\"" << symbol.idnumber() << "\"/>\n";
		}

		if( symbol.probability != FLT_MAX ){
			hasData = true;
			ost << "<symbolProbability unit=\"probabilitycode\" value=\"" << probabilityCode( symbol.probability ) << "\"/>\n";
		}
	}

	return hasData;
}




void 
EncodeLocationForecast2::
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
		replace( result, metatemplate, "" );
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
	
	replace( result, metatemplate, ost.str() );
}

void  
EncodeLocationForecast2::
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
	symbols = SymbolGenerator::computeSymbols( *locationData, symbolConf, true, error );
	symbolContext.setSymbols( &symbols );

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
