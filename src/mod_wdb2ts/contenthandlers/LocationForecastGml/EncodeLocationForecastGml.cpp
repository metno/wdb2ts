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
 * Implementation of the EncodeLocationForecastGml class
 */
#ifdef HAVE_CONFIG_H
#include <wdb2ts_config.h>
#endif
// CLASS


// PROJECT INCLUDES
#include <contenthandlers/LocationForecastGml/EncodeLocationForecastGml.h>
#include <ptimeutil.h>
#include <contenthandlers/LocationForecastGml/GML/WdbForecastsTag.h>
#include <contenthandlers/LocationForecastGml/GML/WdbForecastTag.h>
#include <contenthandlers/LocationForecastGml/GML/PointTag.h>
#include <contenthandlers/LocationForecastGml/GML/WdbMomentTags.h>
#include <contenthandlers/LocationForecastGml/GML/TimePeriodTag.h>
#include <contenthandlers/LocationForecastGml/GML/InstantTimeTag.h>
#include <contenthandlers/LocationForecastGml/GML/XmlTag.h>
#include <ProductTag.h>
#include <TimeTag.h>
#include <MomentTags.h>
#include <LocationTag.h>
#include <PrecipitationTags.h>
#include <PrecipitationPercentileTags.h>
#include <Indent.h>
#include <replace.h>
#include <StreamReplace.h>
#include <wdb2tsProfiling.h>
#include <ptimeutil.h>
#include <copyfile.h>
#include <probabilityCode.h>
#include <SymbolGenerator.h>
#include <Logger4cpp.h>
#include <TempFileStream.h>


// SYSTEM INCLUDES
//
#include <fstream>
#include <unistd.h>
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
	std::list<std::string>  dummyTopoList;
	wdb2ts::MetaModelConfList dummyMetaConf;
	wdb2ts::SymbolConfProvider dummySymbolConfProvider;
}

namespace wdb2ts {

const std::string EncodeLocationForecastGml::metatemplate("<!--@@META@@-->");

EncodeLocationForecastGml::
BreakTimes::
BreakTimes( const std::string &provider_)
	: provider( provider_ )
{
}

EncodeLocationForecastGml::
BreakTimes::
BreakTimes( const BreakTimes &bt )
	: provider( bt.provider ), from( bt.from), to( bt.to ), prevTo( bt.prevTo )
{
}

EncodeLocationForecastGml::
BreakTimes &
EncodeLocationForecastGml::
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


EncodeLocationForecastGml::
EncodeLocationForecastGml()
	:  gmlContext( 0, true ), metaConf( dummyMetaConf ),
      providerPriority( dummyProviderPriority ), modelTopoProviders( dummyTopoProvider ),
      topographyProviders( dummyTopoList ),
      symbolConf( dummySymbolConfProvider ), isPolygon( false )
{
}


EncodeLocationForecastGml::
EncodeLocationForecastGml( Wdb2TsApp *app_,
		                   LocationPointDataPtr locationPointData_,
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
                           const std::list<std::string>  &topographyProviders_,
                           const SymbolConfProvider &symbolConf_,
                           int expire_rand )
   : locationPointData( locationPointData_ ), gmlContext( projectionHelper_, true ),
     longitude( longitude ), latitude( latitude ),
     altitude( altitude ), tempCorrection( 0.0 ), from( from ),
     refTimes( refTimes_ ), metaConf( metaConf_ ),
     precipitationConfig( precipitationConfig_ ),
     providerPriority( providerPriority_ ),
     modelTopoProviders( modelTopoProviders_),
     topographyProviders( topographyProviders_),
     symbolConf( symbolConf_ ),
     isPolygon( false ),
     app( app_ ),
     expireRand( expire_rand )
{
	if(  locationPointData )
		isPolygon = locationPointData->size() > 1;
}

EncodeLocationForecastGml::
~EncodeLocationForecastGml()
{
	// NOOP
}   


EncodeLocationForecastGml::BreakTimeList::const_iterator
EncodeLocationForecastGml::
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
EncodeLocationForecastGml::
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
EncodeLocationForecastGml::
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
EncodeLocationForecastGml::
encodePrecipitationPercentiles( const boost::posix_time::ptime &from, 
			                       std::ostream &ost, 
			                       miutil::Indent &indent )
{
	int hours[] = { 24 };
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
EncodeLocationForecastGml::
encodePrecipitation( const boost::posix_time::ptime &from, 
					 const std::vector<int> &hours,
			         std::ostream &ost,
			         miutil::Indent &indent )
{
	//int hours[] = { 1, 3, 6 };
	int n = hours.size();
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
	   	   
	   		if( ! locationData->init( from, itbt->provider ) )
	   			continue;
	   	} else if( ! locationData->init(itbt->from, itbt->provider ) )
	   		continue;
	   		
	   	doComment = true;
	   	
	   	while( locationData->hasNext() ) {
   			location = locationData->next();
			
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


void 
EncodeLocationForecastGml::
updateBreakTimes( const std::string &provider, const boost::posix_time::ptime &time)
{
//	WEBFW_USE_LOGGER( "encode" );

//	WEBFW_LOG_DEBUG( "updateBreakTimes: called! Provider: " << provider << " breakTimeForecastProvider: " << breakTimeForecastProvider);
	if( breakTimeForecastProvider != provider ) {
	//	WEBFW_LOG_DEBUG( "updateBreakTimes: breakTimeForecastProvider: " << breakTimeForecastProvider << " != "<< provider );
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

bool
EncodeLocationForecastGml::
encodeFeatureMemberTag( const LocationElem &location,
	     					   std::ostream &ost, 
	     					   miutil::Indent &indent )
{
	ostringstream momentOst;

	momentOst.flags( ost.flags() );
	momentOst.precision( ost.precision() );
	
	indent.incrementLevel();
	indent.incrementLevel();
	WdbMomentTags wdbMomentTags( gmlContext, location );
	wdbMomentTags.output( momentOst, indent );

	indent.decrementLevel();
	indent.decrementLevel();
	
	if( momentOst.str().empty() ) 
		return false;
	
	{
		WdbForecastTag forcastTag( gmlContext );
		forcastTag.output( ost, indent );

		{
			IndentLevel level2( indent );
			XmlTag oceanForecast( gmlContext, "metno:OceanForecast", "f");
			oceanForecast.output( ost, indent );
	
			{	
				IndentLevel level3( indent );
	
				TimePeriodTag timePeriodTag( gmlContext, "mox:validTime", location.time(), location.time() );
				timePeriodTag.output( ost, indent );

				if( isPolygon ) {
					PointTag pointTag( gmlContext, "mox:forecastPoint", latitude, longitude, altitude );
					pointTag.output( ost, indent );
				}
			} //Close timePeriodTag
	
			ost << momentOst.str();
		} //Close oceanForecastTag
	} //Close forecastTag
	
	return true;
}



void  
EncodeLocationForecastGml::
encodeMoment( const boost::posix_time::ptime &from, 
			     std::ostream &ost, 
		        miutil::Indent &indent )
{
	WEBFW_USE_LOGGER( "encode" );

	//WEBFW_LOG_DEBUG( "encodeMoment: from: " << from );
	
	if( ! locationData->init( from ) ) {
		WEBFW_LOG_WARN( "encodeMoment: from: " << from << " NO data!" );
		return;
	}

	//Reset the breakTime logic.
	breakTimes.clear();
	breakTimeForecastProvider.erase();
	prevBreakTime = boost::posix_time::ptime();
	curItBreakTimes = breakTimes.end();
					
	while( locationData->hasNext() ) {
		LocationElem &location = *locationData->next();

		if( encodeFeatureMemberTag( location, ost, indent ) )
				updateBreakTimes( location.forecastprovider(), location.time() );
	}
	
	//Update symbols with symbol data from the symbolContext.
	gmlContext.symbolContext.update( symbols );
}


void
EncodeLocationForecastGml::
computePrecipitationMulti( const boost::posix_time::ptime &from,
		                  const std::vector<int> &hours,
		                  PrecipitationAggregations &aggregates
                         )const
{

	WEBFW_USE_LOGGER( "encode" );
	//int hours[] = { 1, 3, 6 };
	int N=hours.size();
	int hoursIndex;
	ptime fromTime;
	LocationElem *location=0;
	float precip;

	BreakTimeList::const_iterator itbt = findBreakTime( from );

	if( itbt == breakTimes.end() ) {
		WEBFW_LOG_WARN( "computePrecipitationMulti: No data." );
		return;
	}

	for( ;
	     itbt != breakTimes.end();
	     ++itbt )
	{
		hoursIndex = 0;

		for( hoursIndex = 0; hoursIndex < N; ++hoursIndex ) {
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

				aggregates[ hours[ hoursIndex ] ] [ fromTime ] = Precipitation( itbt->provider, precip );
			}
		}
	}
}


void
EncodeLocationForecastGml::
encodePeriods( const boost::posix_time::ptime &from,
			   std::ostream &ost,
			   miutil::Indent &indent )
{
	WEBFW_USE_LOGGER( "encode" );
	PrecipConfigElement precip = precipitationConfig->getDefault();
	PrecipitationAggregations precipAggregates;

	if( precip.precipType == PrecipMulti ) {
		computePrecipitationMulti( from, precip.precipHours, precipAggregates );
	} else if( precip.precipType == PrecipSequence ){
		WEBFW_LOG_ERROR( "Encode periods: Precip type sequence not implemented." );
		return;
	}

	if( precipAggregates.empty() ) {
		WEBFW_LOG_NOTICE( "Encode periods: No precipitations aggregates was computed. This my occur if the precipitation config is wrong." );
		return;
	}

	string prevProvider;
	ptime fromTime;
	SymbolHolder::Symbol symbol;
	for( vector<int>::size_type hourIndex = 0; hourIndex < precip.precipHours.size(); ++hourIndex ) {
		PrecipitationAggregations::const_iterator itHours = precipAggregates.find( precip.precipHours[ hourIndex ] );

		if( itHours == precipAggregates.end() )
			continue;

		prevProvider.erase();

		for( map<ptime, Precipitation>::const_iterator itPrecip = itHours->second.begin();
		     itPrecip != itHours->second.end();
		     ++itPrecip )
		{
			IndentLevel level3( indent );

			if( prevProvider != itPrecip->second.provider ) {
				ost << level3.indent() << "<!-- Precip: " << precip.precipHours[ hourIndex ] << " hours provider: "
				    << itPrecip->second.provider << " -->\n";
			}

			prevProvider = itPrecip->second.provider;
			fromTime = itPrecip->first;

			{
				WdbForecastTag forcastTag( gmlContext );
				forcastTag.output( ost, indent );

				{
					IndentLevel level4( indent );
					XmlTag oceanForecast( gmlContext, "metno:OceanForecast", "f");
					oceanForecast.output( ost, indent );

					{
						IndentLevel level5( indent );

						TimePeriodTag timePeriodTag( gmlContext, "mox:validTime",
								                     fromTime,
							                         fromTime+boost::posix_time::hours( precip.precipHours[hourIndex] ) );
						timePeriodTag.output( ost, indent );

						if( isPolygon ) {
							PointTag pointTag( gmlContext, "mox:forecastPoint", latitude, longitude, altitude );
							pointTag.output( ost, indent );
						}
					} //Close timePeriodTag

					ost << level4.indent() << "<mox:precipitation  uom=\"mm\">" << itPrecip->second.precip << "</mox:precipitation>\n";

					if( symbols.findSymbol( prevProvider, fromTime, precip.precipHours[hourIndex], symbol ) ) {
						ost << level4.indent() << "<!-- yrWeatherSymbol idname: " << symbol.idname() << " -->\n";
						ost << level4.indent() << "<mox:yrWeatherSymbol>" << symbol.idnumber() << "</mox:yrWeatherSymbol> \n";
					}

				} //Close oceanForecastTag
			} //Close forecastTag



#if 0
			TimeTag timeTag( fromTime, fromTime+boost::posix_time::hours( precip.precipHours[hourIndex] ) );
			timeTag.output( ost, level3.indent() );

			IndentLevel level4( indent );
			LocationTag locationTag( latitude, longitude, altitude );
			locationTag.output( ost, level4.indent() );

			IndentLevel level5( indent );
			PrecipitationTags precipitationTag( itPrecip->second.precip );
			precipitationTag.output( ost, level5.indent() );

			if( symbols.findSymbol( prevProvider, fromTime, precip.precipHours[hourIndex], symbol ) ) {
				ost << level5.indent() << "<symbol value=\"" << symbol.idname() << "\"/> \n";
			}
#endif
		}
	}
}

void 
EncodeLocationForecastGml::
encodeHeader( std::string &result )
{
	Indent indent;
	IndentLevel level2( indent );
	ProviderRefTimeList::iterator itRefTime;
	string name;
	boost::posix_time::ptime wdbNextrun;
	boost::posix_time::ptime nextrun;
	boost::posix_time::ptime termin;
	ostringstream ost;
	
	WEBFW_USE_LOGGER( "encode" );

	ost.setf(ios::floatfield, ios::fixed);
	ost.precision(0);

	
	if( breakTimes.empty() ) {
		WEBFW_LOG_DEBUG( "encodeHeader: No breaktimes." );
		//replace( result, metatemplate, "" );
		result = "";
		return;
	}
	
	ost << indent.spaces() << "<gml:description>Location forecast from api.met.no </gml:description>\n";
	ost << indent.spaces() << "<mox:procedure xlink:href=\"http://api.met.no/yr-procedure-desc.html\"/>\n";
	ost << indent.spaces() << "<mox:observedProperty xlink:href=\"urn:x-ogc:def:phenomenon:weather\"/>\n";

	if( !isPolygon ) {
		PointTag pointTag( gmlContext, "mox:forecastPoint", latitude, longitude, altitude );
		pointTag.output( ost, indent );
	}
	
	{
		InstantTimeTag timeTag( gmlContext, "mox:issueTime", createdTime );
		timeTag.output( ost, indent );
	}
	
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
		
		if( ! nextrun.is_special() ) {
			if( wdbNextrun.is_special() || nextrun < wdbNextrun )
				wdbNextrun = nextrun;
		}
		
		termin = boost::posix_time::ptime(); //Undef
		itRefTime = refTimes->find( it->provider );
			
		if( itRefTime != refTimes->end() ) 
			termin = itRefTime->second.refTime;
		/*
		XmlTag model( gmlContext, "wdb:model", "", true );
		model.output( ost, indent );
		ost << name ;
		*/
	}
		
	if( ! wdbNextrun.is_special() ) {
		InstantTimeTag timeTag( gmlContext, "mox:nextIssueTime", wdbNextrun );
		timeTag.output( ost, indent );
	}
		
	result = ost.str();
	//replace( result, metatemplate, ost.str(), 1 );
}



void 
EncodeLocationForecastGml::
encodeMeta( std::string &result )
{
	Indent indent;
	IndentLevel level2( indent );
	ProviderRefTimeList::iterator itRefTime;
	string name;
	boost::posix_time::ptime nextrun;
	boost::posix_time::ptime termin;
	boost::posix_time::ptime endTime;
	ostringstream ost;
	
	
	if( breakTimes.empty() ) {
		//replace( result, metatemplate, "" );
		result = "";
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
			termin = boost::posix_time::ptime(); //Undef
			endTime = boost::posix_time::ptime(); //Undef
			itRefTime = refTimes->find( it->provider );

			if( itRefTime != refTimes->end() ) {
				termin = itRefTime->second.refTime;
				endTime = itRefTime->second.updatedTime;
			}
			
			if( itMeta == metaConf.end() ) {
				name = it->provider;
				nextrun = boost::posix_time::ptime(); //Undef
			} else {
				name = itMeta->second.name();
				
				if( name.empty() )
					name = it->provider;
				
				nextrun = itMeta->second.findNextrun(endTime);
			}
			
			ost << level3.indent() << "<model name=\"" << name <<"\"";
			
			if( ! termin.is_special() )
				ost << " termin=\"" << isotimeString( termin, true, true ) << "\"";
			
			if( ! nextrun.is_special() )
				ost << " nextrun=\"" << isotimeString( nextrun, true, true ) << "\"";

			ost << " from=\"" << isotimeString( it->from, true, true ) << "\"";
			ost << " to=\"" << isotimeString( it->to, true, true ) << "\"";
			ost << " />\n";
		}
		
		ost << level2.indent() << "</meta>\n";
	}
	
	//replace( result, metatemplate, ost.str() );
	result = ost.str();
}

void  
EncodeLocationForecastGml::
encode(  webfw::Response &response )
{
	miutil::Indent indent;
	stringstream sost;
	fstream      fost;
	iostream     *tmpost;
	string        result;
	ptime validFrom;
	ptime validTo;
	ptime expire;
	LocationDataPtr myLocationData;
	string error;
	string temporaryFilenamePrefix("metno_wdb2ts");
	miutil::TempFileStream tempFile;
  	
 	USE_MI_PROFILE;
 	MARK_ID_MI_PROFILE("EncodeGML");
 	WEBFW_USE_LOGGER( "encode" );
 	

 	if( isPolygon ) {
 		try{
 			tmpost = tempFile.create( temporaryFilenamePrefix, app->getTmpDir() );
 		}
 		catch( const exception &ex ) {
 			WEBFW_LOG_DEBUG("encode: Could not create temporary file in directory <" << app->getTmpDir() << ">");
 			tmpost = &sost;
 		}
 	} else {
 		tmpost = &sost;
 	}

 	iostream &ost = *tmpost;

 	//Use one decimal precision as default.
 	ost.setf(ios::floatfield, ios::fixed);
 	ost.precision(1);
 	   
 	createdTime = second_clock::universal_time(); 
 	unsigned int seed=static_cast<unsigned int>(time(0));
 	
 	response.contentType("text/xml");

 	expire = second_clock::universal_time();
	expire += hours( 1 );
	expire = ptime( expire.date(), 
	                time_duration( expire.time_of_day().hours(), 0, 0, 0 ));
	
	if( expireRand > 0 )
		expire = expire + seconds( rand_r( &seed ) % expireRand );

	if( locationPointData->empty() ) {
		locationData.reset( new LocationData() );
	}


	{ //Create a scope for the weatherdatatag and producttag.
		WdbForecastsTag wdbForecastsTag( gmlContext, createdTime, schemaName() );
		wdbForecastsTag.output( ost, indent );
		
		//Make room for the meta tag.
		ost << metatemplate;

		for( LocationPointData::iterator it = locationPointData->begin();
				it != locationPointData->end();
				++it )
		{
			if( isPolygon ) {
				longitude = it->first.longitude();
				latitude = it->first.latitude();

				try{
					altitude = app->getHight( latitude, longitude );
				}
				catch( ... ) {
					WEBFW_LOG_WARN( "Unexpected exception: cant get the alitude for location: " << longitude << "/" << latitude << " (lon/lat)." );
					altitude = INT_MIN;
				}
			}

			locationData.reset( new LocationData( it->second, longitude, latitude, altitude,
							                      providerPriority, modelTopoProviders, topographyProviders ) );


			if( altitude == INT_MIN )
				altitude = locationData->hightFromTopography();

			if( altitude == INT_MIN )
				altitude = locationData->hightFromModelTopo();

			if( altitude != INT_MIN )
				locationData->height( altitude );

			//Compute the symbols witoutStateOfAgreate = false. ie, do the best with the teperatures
			//as it is.
			symbols = SymbolGenerator::computeSymbols( *locationData, symbolConf, false, error );
			IndentLevel level2( indent );
			encodeMoment( from, ost, indent );
			encodePeriods( from, ost, indent );

			//Delete the dataset. We don't need it anymore
			it->second.reset();
		}
	}

	//result = ost.str();
	string header;
	encodeHeader( header );
	//encodeMeta( result );
	
	MARK_ID_MI_PROFILE("EncodeGML");
	response.expire( expire );
	ost.flush();

	ost.seekg( 0, ios::end );

	int contentLength = ost.tellg();
	contentLength = contentLength - metatemplate.length() + header.length();

	WEBFW_LOG_DEBUG("ContentLength: " << contentLength );

	if( contentLength < 0  )
		contentLength = 0;



	//response.out() << result;

	if( contentLength > 0 ) {
		MARK_ID_MI_PROFILE("SendGML");
		ost.seekg( 0, ios::beg );

		string filename = tempFile.filename();
		miutil::istreamreplace toSendStream( miutil::StreamReplaceSource(ost, metatemplate, header, 1 ) );

		if( filename.empty() ) {
			response.contentLength( contentLength );
			response.sendStream(  toSendStream );
		} else {
			filename += "_toSend";

			if( ! miutil::file::copyFromStreamToFile( toSendStream, filename ) ) {
				response.status( webfw::Response::INTERNAL_ERROR );
				response.contentLength( 0 );
				return;
			}

			response.sendFile( filename, true );
		}

		MARK_ID_MI_PROFILE("SendGML");
	} else {
		response.contentLength( 0 );
	}

}

/**
 * @}
 * 
 * @}
 */

} // namespace
