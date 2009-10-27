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
#include <sstream>
#include <stlContainerUtil.h>
#include <transactor/ProviderRefTime.h>
#include <UpdateProviderReftimes.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <ptimeutil.h>
#include <UpdateProviderReftimes.h>
#include <Logger4cpp.h>

using namespace std;
using namespace boost::posix_time; //ptime, second_clock
using namespace miutil; //isotimeString, ptimeFromIsoString 

namespace {
bool getProviderReftimes( wdb2ts::WciConnectionPtr wciConnection,
		                    wdb2ts::ProviderRefTimeList &refTimes, 
		                    const string &provider,
		                    const string &reftimespec );

}


namespace wdb2ts {

int 
ProviderRefTimeList::
getDataversion( const std::string &providerWithPlacename ) const
{
	const_iterator it = find( providerWithPlacename );
	
	if( it == end() )
		return -1;
	
	return it->second.dataversion;
}

bool
ProviderRefTimeList::
providerReftime( const std::string &provider,
		         boost::posix_time::ptime &refTime ) const
{
	ProviderItem pvItemIn = ProviderList::decodeItem( provider );
	ProviderItem pvItem;

	for( ProviderRefTimeList::const_iterator it = begin();
			it != end(); ++it ) {

		pvItem = ProviderList::decodeItem( it->first );

		if( ( !pvItemIn.placename.empty() && pvItemIn == pvItem ) ||
			 ( pvItemIn.placename.empty() && pvItemIn.provider == pvItem.provider ) )
		{
			refTime = it->second.refTime;
			return true;
		}
	}

	return false;
}

bool
ProviderRefTimeList::
disabled( const std::string &provider, bool &disabled_ ) const
{
	ProviderItem pvItemIn = ProviderList::decodeItem( provider );
	ProviderItem pvItem;

	for( ProviderRefTimeList::const_iterator it = begin();
		it != end(); ++it ) {

		pvItem = ProviderList::decodeItem( it->first );

		if( ( !pvItemIn.placename.empty() && pvItemIn == pvItem ) ||
			( pvItemIn.placename.empty() && pvItemIn.provider == pvItem.provider ) )
		{
					disabled_ = it->second.disabled;
					return true;
		}
	}

	return false;
}




bool
updateProviderRefTimes( WciConnectionPtr wciConnection, 
						const ProviderRefTimeList &requestedUpdates,
		                ProviderRefTimeList &refTimes,
		                int wciProtocol  )
{
	ProviderRefTimeList resRefTimes;
	ptime now( second_clock::universal_time() );
	ptime back(now);
	ptime endTime( now );
	string reftime;
	ostringstream ost;
	string provider;
	bool changed=false;
	ProviderRefTimeList::const_iterator itReftime;
	
	back -= hours( 8640 ); //8640 hours back from now
	endTime += hours( 24 ); //1 day in the future.
	
	for( ProviderRefTimeList::const_iterator it = requestedUpdates.begin();
	     it != requestedUpdates.end();
	     ++it )
	{
		ost.str("");
		
		//ost <<  "ARRAY['" << it->first << "']";
		provider = it->first;
		
		ost.str("");
		
		if( it->second.refTime.is_special() ) {
			if( wciProtocol == 1 )
				ost << "('" << isotimeString( back, false, true ) << "','" << isotimeString( endTime, false, true ) << "','inside')";
			else
				ost << "'inside " << isotimeString( back, false, true ) << " TO " << isotimeString( endTime, false, true ) << "'";
		} else { 
			if( wciProtocol == 1 )
				ost << "('" << isotimeString( it->second.refTime, false, true ) << "','" << isotimeString( it->second.refTime, false, true ) << "','exact')";
			else
				ost << "'exact " << isotimeString( it->second.refTime, false, true ) << "'";
		}		

		reftime = ost.str();
		resRefTimes.clear();
		
		if( getProviderReftimes( wciConnection, resRefTimes, provider, reftime ) ) {
			
			ProviderItem pi=ProviderList::decodeItem( provider );

			//It should be only one result.
			for( ProviderRefTimeList::iterator rit = resRefTimes.begin();
			     rit != resRefTimes.end();
			     ++rit ) 
			{
				ProviderRefTimeList::iterator refTimeIt = refTimes.find( rit->first );

				//Do not update the reftime if we allready have a record for 
				//this reftime. This is neccesary since the updatedTime is used
				//in the meta data returned, runEnded or something other that tells
				//when we started to serve data from this provider/reftime combination.
				if( refTimeIt != refTimes.end() && refTimeIt->second.refTime == rit->second.refTime ) {
					if( ! pi.placename.empty() ) { 
						if( refTimeIt->first == pi.providerWithPlacename() ) {
							if( refTimeIt->second.dataversion != it->second.dataversion ||
								refTimeIt->second.disabled != it->second.disabled)
								changed = true;

							refTimeIt->second.dataversion = it->second.dataversion;
							refTimeIt->second.disabled = it->second.disabled;
						}
					} else {
						if( refTimeIt->second.dataversion != it->second.dataversion ||
							refTimeIt->second.disabled != it->second.disabled)
							changed = true;
						refTimeIt->second.dataversion = it->second.dataversion;
						refTimeIt->second.disabled = it->second.disabled;
					}
					continue;
				}
				
				if( ! pi.placename.empty() ) {
					if( rit->first == pi.providerWithPlacename() ) {
						rit->second.dataversion = it->second.dataversion;
						rit->second.disabled = it->second.disabled;
						refTimes[rit->first] = rit->second;
						changed = true;
					}
				} else {
					rit->second.dataversion = it->second.dataversion;
					rit->second.disabled = it->second.disabled;
					refTimes[rit->first] = rit->second;
					changed = true;
				}
			}
		} 
	}
	
	return changed;
}


bool
updateProviderRefTimes( WciConnectionPtr wciConnection, 
		                ProviderRefTimeList &refTimes,
		                const ProviderList &providers,
		                int wciProtocol  )
{
	ostringstream ost;
	ProviderRefTimeList tmpRefTimes;
	ptime now( second_clock::universal_time() );
	ptime back(now);
	ptime  endTime( now );
	string reftimeSpec;
	list<string> providerListWithoutplacename=providers.providerWithoutPlacename();
	
	back -= hours( 8640 ); //48 hours back from now
	endTime += hours( 24 ); //1 day in the future.
	
	if( providers.empty() )
		return true;
	
	refTimes.clear();
	
	if( wciProtocol == 1 )
		ost << "('" << isotimeString( back, false, true ) << "','" << isotimeString( endTime, false, true ) << "','inside')";
	else
		ost << "'inside " << isotimeString( back, false, true ) << " TO " << isotimeString( endTime, false, true ) << "'";
	
	reftimeSpec = ost.str();
	
	for( list<string>::iterator it = providerListWithoutplacename.begin() ; 
		  it != providerListWithoutplacename.end(); 
		  ++ it )
	{
		tmpRefTimes.clear();
		
		//Search in the period [back, endTime]
		if( getProviderReftimes( wciConnection, tmpRefTimes, *it, reftimeSpec ) ) {
			for( ProviderRefTimeList::iterator pit =tmpRefTimes.begin();
			     pit != tmpRefTimes.end();
			     ++pit ) 
			{
				refTimes[pit->first] = pit->second;
			}
			continue;
		}
	
		//Try to serach the entire database.
		if( getProviderReftimes( wciConnection, tmpRefTimes, *it, "NULL" ) ) {
			for( ProviderRefTimeList::iterator pit =tmpRefTimes.begin();
				  pit != tmpRefTimes.end();
				  ++pit ) 
				refTimes[pit->first] = pit->second;
		}
	}

	return ! refTimes.empty();
}


void
removeDisabledProviders( ProviderList &providers, const ProviderRefTimeList &reftimes )
{
	ProviderRefTimeList::const_iterator rit;
	ProviderList::iterator it = providers.begin();

	WEBFW_USE_LOGGER( "handler" );

	while( it != providers.end() ) {
		rit = reftimes.find( it->providerWithPlacename() );

		if( rit != reftimes.end() && rit->second.disabled ) {
			WEBFW_LOG_DEBUG("removeDisabledProviders: removing provider: " << it->providerWithPlacename() );
			it = providers.erase( it );
		} else {
			++ it;
		}
	}
}

}


namespace {
bool 
getProviderReftimes( wdb2ts::WciConnectionPtr wciConnection,
		               wdb2ts::ProviderRefTimeList &refTimes, 
                     const string &provider,
	                  const string &reftimespec )
{

	wdb2ts::ProviderItem pi=wdb2ts::ProviderList::decodeItem( provider );
	wdb2ts::ProviderRefTime providerReftimeTransactor( refTimes, pi.provider, reftimespec );
	wdb2ts::PtrProviderRefTimes result;
	
	
	try {
		wciConnection->perform( providerReftimeTransactor, 3 );
		result = providerReftimeTransactor.result();
		
		refTimes = *result;
	}
	catch( const std::ios_base::failure &ex ) {
		throw logic_error( ex.what() );
	}
	catch( const std::runtime_error &ex ) {
		throw logic_error( ex.what() );
	}
	catch( const std::logic_error &ex ) {
		throw;
	}
	catch( ... ) {
		throw logic_error( "Unknown error while checking for reference times." );
	}

	return ! refTimes.empty();
}


}
