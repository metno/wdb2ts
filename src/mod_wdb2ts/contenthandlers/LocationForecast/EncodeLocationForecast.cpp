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
 * Implementation of the EncodeLocationForecast class
 */
#ifdef HAVE_CONFIG_H
#include <wdb2ts_config.h>
#endif
// CLASS
#include <contenthandlers/LocationForecast/EncodeLocationForecast.h>

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
#include <Logger4cpp.h>

// SYSTEM INCLUDES
//
#include <stdlib.h>
#include <time.h>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace std;

using namespace miutil;
using namespace boost::posix_time;

DEFINE_MI_PROFILE;

namespace {
	wdb2ts::ProviderList dummyProviderPriority;
	wdb2ts::TopoProviderMap dummyTopoProvider;
	wdb2ts::LocationData dummy(wdb2ts::TimeSeriePtr(), FLT_MIN, FLT_MAX, INT_MIN, dummyProviderPriority,dummyTopoProvider );
	wdb2ts::MetaModelConfList dummyMetaConf;
}

namespace wdb2ts {

const std::string EncodeLocationForecast::metatemplate("<!--@@META@@-->");

EncodeLocationForecast::
BreakTimes::
BreakTimes( const std::string &provider_)
	: provider( provider_ )
{
}

EncodeLocationForecast::
BreakTimes::
BreakTimes( const BreakTimes &bt )
	: provider( bt.provider ), from( bt.from), to( bt.to ), prevTo( bt.prevTo )
{
}

EncodeLocationForecast::
BreakTimes &
EncodeLocationForecast::
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


EncodeLocationForecast::
EncodeLocationForecast(): 
	locationData( dummy ), metaConf( dummyMetaConf )
{
	// NOOP
}

EncodeLocationForecast::
EncodeLocationForecast( LocationData &locationData_,
                        const ProjectionHelper *projectionHelper_,
		                  float longitude,
								float latitude,
								int   altitude,
								const boost::posix_time::ptime &from,
								const ProviderSymbolHolderList &symbols_,
								PtrProviderRefTimes refTimes_,
								MetaModelConfList &metaConf_,
								int expire_rand )
   : locationData( locationData_ ), projectionHelper( projectionHelper_ ),
     longitude( longitude ), latitude( latitude ),
     altitude( altitude ), tempCorrection( 0.0 ), from( from ),
     symbols( symbols_ ), refTimes( refTimes_ ), metaConf( metaConf_ ),
     expireRand( expire_rand )
{
}

EncodeLocationForecast::
~EncodeLocationForecast()
{
	// NOOP
}   


EncodeLocationForecast::BreakTimeList::const_iterator
EncodeLocationForecast::
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

boost::shared_ptr<SymbolHolder>
EncodeLocationForecast::
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




//Encode the symbols into the xml output.
//We only code symbols that match the timespans in the first
//first dataprovider we have data for in the provider priority list.
//This is identified by the first provider in the breakTimes.
void 
EncodeLocationForecast::
encodeSymbols( std::ostream &out, 
 	            miutil::Indent &indent )
{
	ptime from;
	ptime to;
	ptime time;
	int symbolid;
	string name;
	string idname;
	float prob;
	int oldPrec = out.precision();
	
	if( breakTimes.empty() )
		return;
	
	ProviderSymbolHolderList::iterator ii = symbols.find( breakTimes.begin()->provider );
	
	if( ii == symbols.end() )
		return;
	
	out.precision( 0 );
	
	for( SymbolHolderList::iterator it = ii->second.begin();
	     it != ii->second.end();
	     ++it )
	{
		for( BreakTimeList::iterator itbt = breakTimes.begin(); 
	     	  itbt != breakTimes.end();
	     	  ++ itbt ) 
		{
			boost::shared_ptr<SymbolHolder> ptrSh=findSymbolHolder( itbt->provider,
					                                                  (*it)->timespanInHours() );
		
			if( ! ptrSh ) 
				continue;
			
			ptime startAt;
			if( itbt->prevTo.is_special() ) 
				startAt = itbt->from;
			else
				startAt = itbt->prevTo;
			
			out << indent.spaces() << "<!-- Symbols (BreakTimes) timespan: " << (*it)->timespanInHours()  
				 << " (" << itbt->provider << ")  " << startAt << " - " << itbt->to << "  -->" << endl;

			SymbolHolder *sh = ptrSh.get();
			
			sh->initIndex( startAt );
			while ( sh->next(symbolid, name, idname, time, from, to, prob) && to <= itbt->to ) {
				IndentLevel level3( indent );
				TimeTag timeTag( 	from, to );
				timeTag.output( out, level3.indent() );
												
				IndentLevel level4( indent );
				LocationTag locationTag( latitude, longitude, altitude );
				locationTag.output( out, level4.indent() );
										
				IndentLevel level5( indent );
				out << level5.indent() 
				    << "<symbol id=\"" << idname <<"\" number=\"" << symbolid << "\"/>\n";
				
				if( prob != FLT_MAX )
					out << level5.indent() 
					    << "<symbolProbability unit=\"probabilitycode\" value=\"" << probabilityCode( prob ) << "\"/>\n";
				
			}
		}
	}
	
	out.precision( oldPrec );
}


void
EncodeLocationForecast::
encodePrecipitationPercentiles( const boost::posix_time::ptime &from, 
			                       std::ostream &ost, 
			                       miutil::Indent &indent )
{
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
		if( ! locationData.init( from ) )
			return;
			
		prevProvider.erase();	
		first = true;
		percentileOst.str("");
		
		while( locationData.hasNext() ) {
			LocationElem& location = *locationData.next();
			
			if( ! percentileOst.str().empty() ) {
				ost << outOst.str();
				percentileOst.str("");
			}
		
			outOst.str("");
			
			IndentLevel level3( indent );
			provider = location.forecastprovider();
				
			if( prevProvider != provider )
				ost << level3.indent() << "<!-- Dataprovider: " << provider << " -->\n";
				
			prevProvider = provider;
				
			if( first ) {
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
EncodeLocationForecast::
encodePrecipitation( const boost::posix_time::ptime &from, 
			            std::ostream &ost, 
			            miutil::Indent &indent )
{
	WEBFW_USE_LOGGER( "encode" );

	int hours[] = { 1, 3, 6, 12, 24 };
	int N=sizeof( hours )/sizeof(hours[0]);
	int precipCount[N];
	float precip;
	ptime fromTime;
	ptime prevFromTime;
	bool first;
	bool doComment;
	string prevProvider;
	string provider;
	LocationElem *location=0;
	int precipIndex=0; 
	
	for( int i=0; i<N; ++i ) 
		precipCount[i]=0;
	
	first = true;
	BreakTimeList::const_iterator itbt = findBreakTime( from );

	if( itbt == breakTimes.end() ) {
		WEBFW_LOG_WARN( "EncodeLocationForecast::encodePrecipitation: No data." );
		return;
	}
	
	prevFromTime = itbt->from;
	//WEBFW_LOG_DEBUG( "Precip: pr: " << itbt->provider << " from: " << itbt->from <<" to: " << itbt->to << " prevFromTime: " << prevFromTime << " req from: " << from );
	   
	for( ; 
	     itbt != breakTimes.end();
	     ++itbt ) 
	{
		precipIndex=-1;
		
		for( int i=0; precipCount[i]<N; ++i )
			if( precipCount[i] >0 )
				precipIndex = i;
		
		precipIndex++;
		
		while( precipIndex < N ) {
			if( first ) {
				first = false;
	   	   
				if( ! locationData.init( from, itbt->provider ) )
					continue;
			
			} else if( ! locationData.init(prevFromTime, itbt->provider ) ) {
				break;
			}
		
			doComment = true;
	   	   
			while( locationData.hasNext() ) {
				location = locationData.next();
			
				if(  location->time() > itbt->to )  
					break;
	   			
				precip = location->PRECIP( hours[precipIndex], fromTime );
	   		
				//WEBFW_LOG_DEBUG( "fromTime: " << fromTime << " prevFromTime: "<< prevFromTime << " precipCount[" << precipIndex << "]: " << precipCount[precipIndex] << " precip: " << precip );
			
				if( precip == FLT_MAX || fromTime<prevFromTime ) {
					continue;
				}
				
				++ precipCount[precipIndex];
			
				prevFromTime = location->time();
				
				IndentLevel level3( indent );
			
				if( doComment ) {
					doComment = false;
					ost << level3.indent() << "<!-- Precip: " << hours[precipIndex] << " hours provider: "
					    << itbt->provider << " -->\n";
				}
				
				TimeTag timeTag( fromTime, location->time() );
				timeTag.output( ost, level3.indent() );
							
				IndentLevel level4( indent );
				LocationTag locationTag( latitude, longitude, altitude );
				locationTag.output( ost, level4.indent() );
					
				IndentLevel level5( indent );
				PrecipitationTags precipitationTag( precip );
				precipitationTag.output( ost, level5.indent() );
			}
			
			precipIndex++;
		}
	}
}



#if 0
void
EncodeLocationForecast::
encodePrecipitation( const boost::posix_time::ptime &from, 
			            std::ostream &ost, 
			            miutil::Indent &indent )
{
	int hours[] = { 1, 3, 6 };
	int n=sizeof( hours )/sizeof(hours[0]);
	float precip;
	ptime fromTime;
	ptime firstFromTime;
	bool first;
	bool doComment;
	string prevProvider;
	string provider;
	LocationElem *location=0;
	
	for( int i=0; i<n; ++i ) {
	   first = true;
	   BreakTimeList::const_iterator itbt = findBreakTime( from );
	   
	   if( itbt == breakTimes.end() )
	   	continue;
	   
	   firstFromTime = itbt->from;
	   //WEBFW_LOG_DEBUG( "Precip: pr: " << itbt->provider << " from: " << itbt->from <<" to: " << itbt->to );
	   
	   for( ; 
	        itbt != breakTimes.end();
	   	  ++itbt ) 
	   {
	   	if( first ) {
	   		first = false;
	   	   
	   		if( ! locationData.init( from, itbt->provider ) )
	   			continue;
	   	} else if( ! locationData.init(itbt->from, itbt->provider ) )
	   		continue;
	   		
	   	doComment = true;
	   	
	   	while( locationData.hasNext() ) {
   			location = locationData.next();
			
	   		if(  location->time() > itbt->to )  
	   			break;
	   			
	   		precip = location->PRECIP( hours[i], fromTime );
	   		
	   		if( fromTime<firstFromTime || precip == FLT_MAX ) 
	   			continue;
			
	   		IndentLevel level3( indent );
			
	   		if( doComment ) {
	   			doComment = false;
	   			ost << level3.indent() << "<!-- Precip: " << hours[i] << " hours provider: "
	   			    << itbt->provider << " -->\n";
	   		}
				
	   		TimeTag timeTag( fromTime, location->time() );
	   		timeTag.output( ost, level3.indent() );
							
	   		IndentLevel level4( indent );
	   		LocationTag locationTag( latitude, longitude, altitude );
	   		locationTag.output( ost, level4.indent() );
					
	   		IndentLevel level5( indent );
	   		PrecipitationTags precipitationTag( precip );
	   		precipitationTag.output( ost, level5.indent() );
	   	}
	   }
	}
}
#endif

void 
EncodeLocationForecast::
updateBreakTimes( const std::string &provider, const boost::posix_time::ptime &time)
{
	if( breakTimeForecastProvider != provider ) {
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
	}
	
	curItBreakTimes->to = time;
	prevBreakTime = time;
}

void  
EncodeLocationForecast::
encodeMoment( const boost::posix_time::ptime &from, 
			     std::ostream &ost, 
		        miutil::Indent &indent )
{
	CIProviderPDataList itPData;
	CIFromTimeSerie itFromTimeserie;
	ostringstream tmpOst;
	ostringstream momentOst;
		
	tmpOst.flags( ost.flags() );
	tmpOst.precision( ost.precision() );
	
	momentOst.flags( ost.flags() );
	momentOst.precision( ost.precision() );
	
	if( ! locationData.init( from ) )
		return;
	
	curItBreakTimes = breakTimes.end();
	
	while( locationData.hasNext() ) {
		LocationElem &location = *locationData.next();
		
		if( ! momentOst.str().empty() ) {
			ost << tmpOst.str();
			momentOst.str("");
		}
			
		tmpOst.str("");
		momentOst.str("");
		
		IndentLevel level3( indent );
		TimeTag timeTag( location.time(), location.time() );
		timeTag.output( tmpOst, level3.indent() );
	
		IndentLevel level4( indent );
		LocationTag locationTag( latitude, longitude, altitude );
		locationTag.output( tmpOst, level4.indent() );
	
		IndentLevel level5( indent );
		MomentTags momentTags( location, symbolContext, projectionHelper );
		momentTags.output( momentOst, level5.indent() );
		
		if( ! momentOst.str().empty() ) {
			updateBreakTimes( location.forecastprovider(), location.time() );
			tmpOst << momentOst.str();
		}
	}

	//May have one leftover.
	if( ! momentOst.str().empty() ) 
		ost << tmpOst.str();
	
	//Update symbols with symbol data from the symbolContext.
	symbolContext.update( symbols );
}

void 
EncodeLocationForecast::
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
EncodeLocationForecast::
encode(  webfw::Response &response )
{
	miutil::Indent indent;
	ostringstream ost;
	string        result;
	ptime validFrom;
	ptime validTo;
	ptime expire;
  	
 	USE_MI_PROFILE;
 	MARK_ID_MI_PROFILE("EncodeXML");
 	
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
	
	
	{ //Create a scope for the weatherdatatag and producttag.
		WeatherdataTag weatherdataTag(second_clock::universal_time(), schemaName() );
		weatherdataTag.output( ost, indent.spaces() );
		
		//Make room for the meta tag.
		ost << metatemplate;

		IndentLevel level2( indent );
		ProductTag productTag;
		productTag.output( ost, level2.indent() );
		
		encodeMoment( from, ost, indent );	
		encodePrecipitation( from, ost, indent );
		encodeSymbols( ost, indent );
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
