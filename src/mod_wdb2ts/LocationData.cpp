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

#include <iostream>
#include <float.h>
#include <limits.h>
#include <LocationData.h>
#include <Logger4cpp.h>

namespace {
boost::posix_time::ptime topoTimeSpecialTime( boost::gregorian::date(1970, 1, 1),
											  boost::posix_time::time_duration( 0, 0, 0 ) );
}

namespace wdb2ts {

using namespace std;
using namespace boost::posix_time;

LocationData::
LocationData( wdb2ts::TimeSeriePtr timeSerie_, 
				  float longitude, float latitude, int hight,
		        const ProviderList &providerPriority,
		        const TopoProviderMap &modelTopoProviders,
		        const std::list<std::string>  &topographyProviders )
 	: providerPriority_( providerPriority ),
	  modelTopoProviders_( modelTopoProviders ),
	  topographyProviders_( topographyProviders ),
 	  timeSeriePtr( timeSerie_ ),
 	  timeSerie( timeSeriePtr.get() ),
 	  locationElem( providerPriority, modelTopoProviders, topographyProviders, longitude, latitude, hight )
{
	cleanup();

	if( timeSerie )
		itTimeSerie = timeSerie->begin();
	
	
}
	
LocationData::
~LocationData()
{
	//delete timeSerie;
}
	
void
LocationData::
cleanup()
{
	WEBFW_USE_LOGGER( "encode" );

	log4cpp::Priority::Value loglevel = WEBFW_GET_LOGLEVEL();

	boost::posix_time::ptime fromThisTime;
	string providerWithFromThisTime;
	ProviderPDataList::iterator itProviderData;

	for( ProviderList::iterator itProvider = providerPriority_.begin();
		     itProvider != providerPriority_.end();
		     ++itProvider )
	{
		TimeSerie::iterator tit = timeSeriePtr->begin();

		for( TimeSerie::iterator tit = timeSeriePtr->begin();
			 tit != timeSeriePtr->end() && fromThisTime.is_special();
			 ++tit )
		{
			if( tit->first.is_special() || tit->first == topoTimeSpecialTime )
				continue;

			for( FromTimeSerie::iterator fit = tit->second.begin();
				 fit != tit->second.end();
				 ++fit )
			{

			    if( fit->first.is_special() || fit->first == topoTimeSpecialTime )
			    	continue;

				itProviderData = fit->second.find( itProvider->providerWithPlacename() );

				if( itProviderData == fit->second.end() )
					continue;

				WEBFW_LOG_DEBUG("LocationData::cleanup: '" << itProviderData->first
						         << "' count: " << itProviderData->second.count() );

				if( itProviderData->second.count() > 1 ) {
					fromThisTime = tit->first;
					providerWithFromThisTime = itProviderData->first;
					break;
				}
			}
		}
	}

	if( fromThisTime.is_special() ) {
		WEBFW_LOG_DEBUG("LocationData::cleanup: Found no datasets with a count > 2.");
		return;
	}

	WEBFW_LOG_DEBUG("LocationData::cleanup: Use only data after or equal totime: " << fromThisTime << " ("
			        << providerWithFromThisTime << ")");



	TimeSerie::iterator titTemp;
	TimeSerie::iterator tit = timeSeriePtr->begin();

	for( TimeSerie::iterator tit = timeSeriePtr->begin();
		tit != timeSeriePtr->end(); /*NOOP*/  )
	{
		if( tit->first.is_special() || tit->first == topoTimeSpecialTime ) {
			++tit;
			continue;
		}

		if( tit->first < fromThisTime ) {
			if( loglevel >= log4cpp::Priority::DEBUG ) {
				for( FromTimeSerie::iterator fit = tit->second.begin();
					 fit != tit->second.end();
					 ++fit )
				{
					for( ProviderPDataList::iterator pit=fit->second.begin();
						 pit!=fit->second.end();
						 ++pit )
					{
						WEBFW_LOG_DEBUG("LocationData::cleanup: Removing '" << pit->first <<"' "
								         << fit->first << " - " << tit->first << " count: " << pit->second.count());
					}
				}
			}

			titTemp = tit;
			++tit;
			timeSeriePtr->erase( titTemp );
		} else {
			break;
		}
	}
}


int
LocationData::
hightFromModelTopo()
{
	int alt;
	init( boost::posix_time::ptime() );

	LocationElem *elem= peek();

	if( ! elem )
		return INT_MIN;

	ProviderList::const_iterator it;

	for( it=providerPriority_.begin(); it != providerPriority_.end(); ++it ){
		alt = elem->modeltopography( it->providerWithPlacename() );

		if( alt != INT_MIN )
			return alt;
	}

	return INT_MIN;

}

int
LocationData::
hightFromTopography()
{
	int alt=INT_MIN;
	init( boost::posix_time::ptime() );


	LocationElem *elem= peek();

	if( ! elem )
		return INT_MIN;

	std::list<std::string>::const_iterator it;

	for( it=topographyProviders_.begin(); it != topographyProviders_.end(); ++it ){
		alt = elem->topography( *it );

		if( alt != INT_MIN )
			return alt;
	}

	return INT_MIN;

}


bool
LocationData::
hasDataForTime( const boost::posix_time::ptime &fromtime, 
			    const boost::posix_time::ptime &totime,
			    const std::string &provider ) const
{

	if( ! timeSerie )
		return false;
	
	CITimeSerie itts = timeSerie->find( totime );
	
	if( itts == timeSerie->end() )
		return false;
	
	CIFromTimeSerie itfts = itts->second.find( fromtime );
	
	if( itfts == itts->second.end() )
		return false;
	
	CIProviderPDataList itpdl = itfts->second.find( provider );
	
	if( itpdl == itfts->second.end() )
		return false;

	return true;
}	


ITimeSerie
LocationData::
findFirstDataSet( const std::string &provider, 
		            const boost::posix_time::ptime &startAtToTime )const
{
	IFromTimeSerie itFromTimeSerie;
	IProviderPDataList itProviderPDataList;
	ITimeSerie it;
	boost::posix_time::time_duration h;
	boost::posix_time::ptime fromTime;
	
	it = timeSerie->begin();
			
	if( it != timeSerie->end() )
		++it;   //Allways skip the first dataset. This is the initial condition.
	
	if( startAtToTime.is_special() ) {
		for( ; it != timeSerie->end(); ++it ) {
			
		   fromTime = it->first - boost::posix_time::hours( timespan_ );

			itFromTimeSerie = it->second.find( fromTime );
			
			if( itFromTimeSerie == it->second.end() )
				continue;

			itProviderPDataList = itFromTimeSerie->second.find( provider );
			
			if( itProviderPDataList == itFromTimeSerie->second.end() )
				continue;
			
			return it;
		}
		
		return timeSerie->end();
	} 
	
	for( ; it != timeSerie->end(); ++it ) {
		if( it->first < startAtToTime )
			continue;
		
      fromTime = it->first - boost::posix_time::hours( timespan_ );

		itFromTimeSerie = it->second.find( fromTime );
		
		if( itFromTimeSerie == it->second.end() )
			continue;
		
		itProviderPDataList = itFromTimeSerie->second.find( provider );
				
		if( itProviderPDataList == itFromTimeSerie->second.end() )
			continue;
				
		return it;
	}
	
	return timeSerie->end();
}


bool 
LocationData::
init( const boost::posix_time::ptime &startAtToTime,  const std::string &provider,
      int timespan)
{
	if( ! timeSerie )
		return false;

	provider_ = provider;
	timespan_ = timespan;
	/*
	if( startAtToTime.is_special() ) {
		itTimeSerie = timeSerie->begin();
	} else {
		for( itTimeSerie = timeSerie->begin(); 
			  itTimeSerie != timeSerie->end(); 
		     ++itTimeSerie ) 
			if( itTimeSerie->first >= startAtToTime )
				break;
	}
	
	locationElem.forecastProvider = provider_;
	
	return itTimeSerie != timeSerie->end();
	*/
	
	
	if( provider_.empty() ) {
		for( ProviderList::iterator it = providerPriority_.begin();
		     it != providerPriority_.end();
		     ++it ) {
			itTimeSerie = findFirstDataSet( it->providerWithPlacename(), startAtToTime );
			
			if( itTimeSerie == timeSerie->end() )
				continue;
			
			locationElem.forecastProvider = it->providerWithPlacename();
			provider_ = it->providerWithPlacename();
			return true;
		}	
			
		return false;	
	}
	
	
	itTimeSerie = findFirstDataSet( provider_, startAtToTime );
		
	if( itTimeSerie == timeSerie->end() )
		return false;
		
	locationElem.forecastProvider = provider_;
		
	return true;
}

bool 
LocationData::
hasNext()
{
	if( ! timeSerie )
		return false;
	
	if( itTimeSerie != timeSerie->end() )
		return true;
	
	return false;
}

LocationElem* 
LocationData::
next()
{
	ITimeSerie itTmpTimeSerie=itTimeSerie;
	++itTimeSerie;
	
	locationElem.init( itTmpTimeSerie, timeSerie );

	if( locationElem.symbolprovider() == "pgen_probability [norway 025]" ) {
	   cerr << "LocationData::next: first: " << itTmpTimeSerie->first;
	   FromTimeSerie::iterator fit = itTmpTimeSerie->second.begin();
	   if( fit != itTmpTimeSerie->second.end() ) {
	      cerr << " - " << fit->first;
	      ProviderPDataList::iterator pid = fit->second.begin() ;
	      if( pid != fit->second.end() ) {
	         cerr << " '" << pid->first << "' # " << pid->second.count() << endl << "   ";
	         pid->second.print( cerr );
	      }
	   }
	   cerr << endl;
	}
	return &locationElem;
}

LocationElem*
LocationData::
peek()
{
	if( ! hasNext() )
		return 0;

	locationElem.init( itTimeSerie, timeSerie );

	return &locationElem;
}


}



