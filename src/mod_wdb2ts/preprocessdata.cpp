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

#include <set>
#include <float.h>
#include <preprocessdata.h>

namespace {

using namespace std;

void computePrecipN( wdb2ts::TimeSerie &timeSerie, 
                     wdb2ts::ITimeSerie itNow,
                     const std::string &providerName,
		               const boost::posix_time::ptime &refTime,
		               int hoursBack );
}

namespace wdb2ts {


/* Preprocessdata goes through all data and computes 
 * 1 houre (PRECIP_1), 3 houre (PRECIP_3), 6 houre (PRECIP_6),
 * 12 hour (PRECIP_12) and 24 houre (PRECIP_24). 
 * 
 * The values is computed from by PRECIP[T]-PRECIP[T-n], where n
 * denotes a number of times back from T. It is importent that 
 * the fromtimes is the same for all PRECIP values that is used in 
 * computations. 
 * 
 * It also computes a teperature value TA that is the value 
 * of T2M or T2M_land, where T2M_land have priority over T2M. 
 * 
 * timeSerie is composed of timeSerie[validto][validfrom][providername] = PData
 *   
 * 
 */

void 
preprocessdata( TimeSerie &timeSerie )
{
	
	//TODO:Reprogram this to only generate the symbols.
	//For now just return.
	
	return;
	
	using namespace boost::posix_time;
	
	ITimeSerie itProviderTimeSerie;
	IFromTimeSerie it;
	IFromTimeSerie itBack;
	IProviderPDataList itFrom;
	ptime refFromTime;
	ptime fromTime;
	set<string> providers;
	set<string>::const_iterator itProvider;
	
	for( itProviderTimeSerie=timeSerie.begin();
			  itProviderTimeSerie != timeSerie.end();
			  ++itProviderTimeSerie ) {
		for( it=itProviderTimeSerie->second.begin(); it!=itProviderTimeSerie->second.end(); ++it ) {
			for( CIProviderPDataList itPData=it->second.begin(); itPData != it->second.end(); ++itPData ) {
				providers.insert( itPData->first );
			}
		}
	}
	
	for( itProvider = providers.begin(); itProvider != providers.end(); ++itProvider ) {
		for( itProviderTimeSerie=timeSerie.begin();
		  	  itProviderTimeSerie != timeSerie.end();
		     ++itProviderTimeSerie	) { //first->ptime second->map<ptime, map<string,PData> >
			for( it=itProviderTimeSerie->second.begin(); it!=itProviderTimeSerie->second.end(); ++it ) {
				//Give priority to T2M_LAND
				//Find the fromtime that is equal to totime
				itFrom = it->second.find( *itProvider ); 
			
				if( itFrom != it->second.end() ) {
					//Give priority to T2M_LAND
			
					if( itFrom->second.T2M_LAND != FLT_MAX )
						itFrom->second.TA=itFrom->second.T2M_LAND;
					else
						itFrom->second.TA=itFrom->second.T2M;
				}
			
				//We use the fromtime from the first record that has a valid PRECIP value
				//as the reference time for the PRECIP computations.
				if( refFromTime.is_special() ) {
					IProviderPDataList tmpItData;
				
					for( IFromTimeSerie tmpIt=itProviderTimeSerie->second.begin();
				     	  tmpIt != itProviderTimeSerie->second.end();
				     	  ++tmpIt ) {
						
						tmpItData = tmpIt->second.find( *itProvider );
						
						if( tmpItData == tmpIt->second.end() )
							continue;
						
						if( tmpItData->second.PRECIP != FLT_MAX ) {
							refFromTime = tmpIt->first;
							break;
						}
					}
				
					if( refFromTime.is_special() )
						continue;
				}

				computePrecipN( timeSerie, itProviderTimeSerie, *itProvider, refFromTime,  1 );
				computePrecipN( timeSerie, itProviderTimeSerie, *itProvider, refFromTime,  3 );
				computePrecipN( timeSerie, itProviderTimeSerie, *itProvider, refFromTime,  6 );
				computePrecipN( timeSerie, itProviderTimeSerie, *itProvider, refFromTime, 12 );
				computePrecipN( timeSerie, itProviderTimeSerie, *itProvider, refFromTime, 24 );
			}
		}
	}
	
}

}

namespace {
void
computePrecipN( wdb2ts::TimeSerie &timeSerie, 
		          wdb2ts::ITimeSerie itNow,
		          const std::string &providerName,
		          const boost::posix_time::ptime &refTime,
		          int hoursBack )
{
	using namespace boost::posix_time;
 		
   wdb2ts::ITimeSerie itBackFrom;
   wdb2ts::IFromTimeSerie itBackRefTime;
   wdb2ts::IProviderPDataList itBackFromPData;
   wdb2ts::IFromTimeSerie itFrom;
	wdb2ts::IProviderPDataList itFromPData;
	float diff;
	
	//Find the record PRECIP[time - hoursback]
	ptime fromTime = itNow->first - hours( hoursBack );
	
	itBackFrom = timeSerie.find( fromTime );
	
	if( itBackFrom == timeSerie.end() )
		return;
		
	itBackRefTime = itBackFrom->second.find( refTime );
	
	if( itBackRefTime == itBackFrom->second.end() )
		return;
	
	itBackFromPData = itBackRefTime->second.find( providerName );
	
	if( itBackFromPData == itBackRefTime->second.end() )
		return;
	
	if( itBackFromPData->second.PRECIP == FLT_MAX )
		return;
	
	
	//Find the record PRECIP[time]
	itFrom = itNow->second.find( refTime );
		
	if( itFrom == itNow->second.end() )
		return;
	
	itFromPData = itFrom->second.find( providerName );
	
	if( itFromPData == itFrom->second.end() )
		return;
	
	if( itFromPData->second.PRECIP == FLT_MAX )
		return;
	
	
	diff = itFromPData->second.PRECIP - itBackFromPData->second.PRECIP; 
	
	if( diff < 0 )
		diff = 0.0f;
	
	switch( hoursBack ) {
	case  1: itNow->second[itNow->first][providerName].PRECIP_1T = diff; break;
	case  3: itNow->second[itNow->first][providerName].PRECIP_3T = diff; break;
	case  6: itNow->second[itNow->first][providerName].PRECIP_6T = diff; break;
	case 12: itNow->second[itNow->first][providerName].PRECIP_12T = diff; break;
	case 24: itNow->second[itNow->first][providerName].PRECIP_24T = diff; break;
	default:
		break;
	}
	
}

}

