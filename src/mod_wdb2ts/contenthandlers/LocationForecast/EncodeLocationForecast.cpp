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
#include <SymbolGenerator.h>
#include <Logger4cpp.h>

// SYSTEM INCLUDES
//
#include <stdlib.h>
#include <time.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <map>
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

	//Used by encodePrecipitation to compute the precipitation agregations.
	struct Precip { string provider;
	                float  precip;

	                Precip():precip(FLT_MAX) {}
	                Precip( const Precip &p ) : provider( p.provider ), precip( p.precip ) {}
	                Precip( const string &provider_, float precip_ )
						: provider( provider_ ), precip( precip_ ) {}

	                Precip& operator=( const Precip &rhs ) {
	                	if( this != &rhs ) {
	                		provider = rhs.provider;
	                		precip = rhs.precip;
	                	}

	                	return *this;
	                }

				  };

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
	metaConf( dummyMetaConf ),
	symbolContext( true ),
	providerPriority( dummyProviderPriority ), modelTopoProviders( dummyTopoProvider ),
	topographyProviders( dummyTopoList ),
	symbolConf( dummySymbolConfProvider )
{
	// NOOP
}

EncodeLocationForecast::
EncodeLocationForecast( LocationPointDataPtr locationPointData_,
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
     symbolContext( true ),
     precipitationConfig( precipitationConfig_ ),
     providerPriority( providerPriority_ ),
     modelTopoProviders( modelTopoProviders_),
     topographyProviders( topographyProviders_ ),
     symbolConf( symbolConf_ ),
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

void
EncodeLocationForecast::
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
	WEBFW_USE_LOGGER( "encode" );
	log4cpp::Priority::Value loglevel = WEBFW_GET_LOGLEVEL();

	SymbolHolder::Symbol symbol;
	PartialData partialData;
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
			
			SymbolHolder *sh = ptrSh.get();
			sh->initIndex( startAt );

			if( loglevel >= log4cpp::Priority::DEBUG )
				out << indent.spaces() << "<!-- Symbols (BreakTimes) timespan: " << (*it)->timespanInHours()
				    << " (" << itbt->provider << ")  " << startAt << " - " << itbt->to << " -->" << endl;

			//while ( sh->next(symbolid, name, idname, time, from, to, prob) && to <= itbt->to ) {
			while ( sh->next( symbol ) && symbol.toAsPtime() <= itbt->to ) {
			   if( symbols.getPartialData( symbol.fromAsPtime(), partialData ) )
			      SymbolGenerator::correctSymbol( symbol, partialData );

			   ++nElements;
			   IndentLevel level3( indent );
				TimeTag timeTag( 	symbol.fromAsPtime(), symbol.toAsPtime() );
				timeTag.output( out, level3.indent() );
												
				IndentLevel level4( indent );
				LocationTag locationTag( latitude, longitude, altitude );
				locationTag.output( out, level4.indent() );
										
				IndentLevel level5( indent );
				out << level5.indent() 
				    << "<symbol id=\"" << symbol.idname() <<"\" number=\"" << symbol.idnumber() << "\"/>\n";
				
				if( symbol.probability != FLT_MAX )
					out << level5.indent() 
					    << "<symbolProbability unit=\"probabilitycode\" value=\"" << probabilityCode( symbol.probability ) << "\"/>\n";
				
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
				
			if( prevProvider != provider &&  loglevel >= log4cpp::Priority::DEBUG )
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
EncodeLocationForecast::
encodePrecipitation( const boost::posix_time::ptime &from,
		             std::vector<int> &hours,
			         std::ostream &ost,
			         miutil::Indent &indent )
{
	WEBFW_USE_LOGGER( "encode" );
	log4cpp::Priority::Value loglevel = WEBFW_GET_LOGLEVEL();
//	int hours[] = { 1, 3, 6, 12, 24 };
	int N = hours.size();
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
	bool tryHard;
	
	for( int i=0; i<N; ++i ) 
		precipCount[i]=0;
	
	first = true;
	BreakTimeList::const_iterator itbt = findBreakTime( from );

	if( itbt == breakTimes.end() ) {
		WEBFW_LOG_WARN( "encodePrecipitation: No data." );
		return;
	}
	
	prevFromTime = itbt->from;
	WEBFW_LOG_DEBUG( "encodePrecipitation: Precip: provider: " << itbt->provider << " from: " << itbt->from <<" to: " << itbt->to << " prevFromTime: " << prevFromTime << " req from: " << from );
	   
	for( ; 
	     itbt != breakTimes.end();
	     ++itbt ) 
	{
		tryHard = true;
		precipIndex=-1;
		
		for( int i=0; i<N; ++i ) {
			if( precipCount[i] >0 ) {
				precipIndex = i;
				break;
			}
		}
		
		precipIndex++;
		
		while( precipIndex < N ) {
			if( first ) {
				first = false;
	   	   
				if( ! locationData->init( from, itbt->provider ) )
					continue;
			
			} else if( ! locationData->init(prevFromTime, itbt->provider ) ) {
				break;
			}
		
			doComment = true;
	   	   
			while( locationData->hasNext() ) {
				location = locationData->next();
			
				if(  location->time() > itbt->to )  
					break;
	   			
				precip = location->PRECIP( hours[precipIndex], fromTime, tryHard );
	   		
				//WEBFW_LOG_DEBUG( "encodePrecipitation: hours: " << hours[precipIndex] << " fromTime: " << fromTime << " prevFromTime: "<< prevFromTime << " precipCount[" << precipIndex << "]: " << precipCount[precipIndex] << " precip: " << precip );
			
				if( precip == FLT_MAX || fromTime<prevFromTime ) {
					continue;
				}
				++nElements;
				tryHard=false;
				++ precipCount[precipIndex];
			
				prevFromTime = location->time();
				
				IndentLevel level3( indent );
			
				if( doComment && loglevel >= log4cpp::Priority::DEBUG ) {
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

void
EncodeLocationForecast::
encodePrecipitationMulti( const boost::posix_time::ptime &from,
		                  std::vector<int> &hours,
			              std::ostream &ost,
			              miutil::Indent &indent )
{

	WEBFW_USE_LOGGER( "encode" );
 	log4cpp::Priority::Value loglevel = WEBFW_GET_LOGLEVEL();
	//int hours[] = { 1, 3, 6 };
	int N=hours.size();
	int hoursIndex;
	map< int, map<ptime, Precip>  > precipHours;

	ptime fromTime;
	ptime timeBack;
	string prevProvider;
	string provider;
	LocationElem *location=0;
	float precip;

	BreakTimeList::const_iterator itbt = findBreakTime( from );

	if( itbt == breakTimes.end() ) {
		WEBFW_LOG_WARN( "encodePrecipitationMulti: No data." );
		return;
	}

	for( ;
	     itbt != breakTimes.end();
	     ++itbt )
	{
		hoursIndex = 0;

		for( hoursIndex = 0; hoursIndex < N; ++hoursIndex ) {

			//WEBFW_LOG_DEBUG( "encodePrecipitationMulti: " << itbt->provider << "  " << itbt->from  );

			if( ! locationData->init( itbt->from, itbt->provider ) )
				continue;

			while( locationData->hasNext() ) {
				location = locationData->next();


				if(  location->time() > itbt->to )
					break;

				precip = location->PRECIP( hours[hoursIndex], fromTime );


				//COMMENT: To be compatible with ts2xml do not send out data before the request time.
				if( fromTime < from )
					continue;


				//WEBFW_LOG_DEBUG( "encodePrecipitationMulti: hours: " << hours[hoursIndex] << " fromTime: " << fromTime << " precip: " << precip );

				if( precip == FLT_MAX )
					continue;

				precipHours[ hours[ hoursIndex ] ] [ fromTime ] = Precip( itbt->provider, precip );
			}
		}
	}

	for( hoursIndex = 0; hoursIndex < N; ++hoursIndex ) {
		map< int, map<ptime, Precip>  >::const_iterator itHours = precipHours.find( hours[ hoursIndex ] );

		if( itHours == precipHours.end() )
			continue;

		prevProvider.erase();

		for( map<ptime, Precip>::const_iterator itPrecip = itHours->second.begin();
		     itPrecip != itHours->second.end();
		     ++itPrecip )
		{
			IndentLevel level3( indent );

			if( prevProvider != itPrecip->second.provider &&
				loglevel >= log4cpp::Priority::DEBUG)
			{
				ost << level3.indent() << "<!-- Precip: " << hours[ hoursIndex ] << " hours provider: "
					    << itPrecip->second.provider << " -->\n";
			}

			++nElements;
			prevProvider = itPrecip->second.provider;
			fromTime = itPrecip->first;

			TimeTag timeTag( fromTime, fromTime+boost::posix_time::hours( hours[hoursIndex] ) );
			timeTag.output( ost, level3.indent() );

			IndentLevel level4( indent );
			LocationTag locationTag( latitude, longitude, altitude );
			locationTag.output( ost, level4.indent() );

			IndentLevel level5( indent );
			PrecipitationTags precipitationTag( itPrecip->second.precip );
			precipitationTag.output( ost, level5.indent() );
		}
	}
}



void 
EncodeLocationForecast::
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
	
	if( ! locationData->init( from ) )
		return;
	
	curItBreakTimes = breakTimes.end();
	
	while( locationData->hasNext() ) {
		LocationElem &location = *locationData->next();
		
		if( ! momentOst.str().empty() ) {
		   ++nElements;
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
		   symbols.addPartialData( location );
			updateBreakTimes( location.forecastprovider(), location.time() );
			tmpOst << momentOst.str();
		}
	}

	//May have one leftover.
	if( ! momentOst.str().empty() ) {
	   ++nElements;
		ost << tmpOst.str();
	}
	
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
		replaceString( result, metatemplate, "" );
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
	
	replaceString( result, metatemplate, ost.str() );
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

	//Compute the symbols witoutStateOfAgreate = false. ie, do the best with the teperatures
	//as it is.
	symbols = SymbolGenerator::computeSymbols( *locationData, symbolConf, false, error );

	{ //Create a scope for the weatherdatatag and producttag.
		WeatherdataTag weatherdataTag(second_clock::universal_time(), schemaName() );
		weatherdataTag.output( ost, indent.spaces() );
		
		//Make room for the meta tag.
		ost << metatemplate;

		IndentLevel level2( indent );
		ProductTag productTag;
		productTag.output( ost, level2.indent() );
		
		encodeMoment( from, ost, indent );	

		PrecipConfigElement precip = precipitationConfig->getDefault();

		if( precip.precipType == PrecipSequence )
			encodePrecipitation( from, precip.precipHours, ost, indent );
		else  // precip.precipType == PrecipMulti
			encodePrecipitationMulti( from, precip.precipHours, ost, indent );

		encodeSymbols( ost, indent );
		encodePrecipitationPercentiles( from, ost, indent );
	}

	if( nElements <= 0 ) {
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
