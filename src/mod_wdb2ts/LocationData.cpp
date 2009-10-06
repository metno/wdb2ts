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

namespace wdb2ts {

using namespace std;
using namespace boost::posix_time;

LocationData::
LocationData( wdb2ts::TimeSeriePtr timeSerie_, 
				  float longitude, float latitude, int hight,
		        const ProviderList &providerPriority,
		        const TopoProviderMap &modelTopoProviders,
		        const TopoProviderMap &topographyProviders )
 	: providerPriority_( providerPriority ),
	  modelTopoProviders_( modelTopoProviders ),
	  topographyProviders_( topographyProviders ),
 	  timeSeriePtr( timeSerie_ ),
 	  timeSerie( timeSeriePtr.get() ),
 	  locationElem( providerPriority, modelTopoProviders, topographyProviders, longitude, latitude, hight )
{
	if( timeSerie )
		itTimeSerie = timeSerie->begin();
	
	
}
	
LocationData::
~LocationData()
{
	//delete timeSerie;
}
	

int
LocationData::
hightFromModelTopo()
{
	int alt;
	init( boost::posix_time::ptime() );

	if( ! hasNext() )
		return INT_MIN;

	LocationElem &elem= *next();

	ProviderList::const_iterator it;

	for( it=providerPriority_.begin(); it != providerPriority_.end(); ++it ){
		alt = elem.modeltopography( it->providerWithPlacename() );

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


CITimeSerie
LocationData::
findProviderData( const std::string &provider, 
		            const boost::posix_time::ptime &startAtToTime )const
{
	CIFromTimeSerie itFromTimeSerie;
	CIProviderPDataList itProviderPDataList;
	CITimeSerie it;
	
	it = timeSerie->begin();
			
	if( it != timeSerie->end() )
		++it;   //Allways skip the first dataset. This is the initial condition.
	
	if( startAtToTime.is_special() ) {
		for( ; it != timeSerie->end(); ++it ) {
			
			itFromTimeSerie = it->second.find( it->first );
			
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
		
		itFromTimeSerie = it->second.find( it->first );
		
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
init( const boost::posix_time::ptime &startAtToTime,  const std::string &provider_  )
{
	if( ! timeSerie )
		return false;
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
			itTimeSerie = findProviderData( it->providerWithPlacename(), startAtToTime );
			
			if( itTimeSerie == timeSerie->end() )
				continue;
			
			locationElem.forecastProvider = it->providerWithPlacename();
			
			return true;
		}	
			
		return false;	
	}
	
	
	itTimeSerie = findProviderData( provider_, startAtToTime );
		
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
	CITimeSerie itTmpTimeSerie=itTimeSerie;
	++itTimeSerie;
	
	locationElem.init( itTmpTimeSerie, timeSerie );
	
	return &locationElem;
}

}



