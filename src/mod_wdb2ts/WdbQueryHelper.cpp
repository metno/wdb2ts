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


#include <sstream>
#include <algorithm>
#include <set>
#include <WdbQueryHelper.h>
#include <ptimeutil.h>
#include <wdb2tsProfiling.h>
#include <ProviderList.h>

DEFINE_MI_PROFILE;

namespace {
	wdb2ts::config::Config::Query dummyQuerys;
	
	std::string returnColoumns="value, dataprovidername, placename, referencetime, validfrom, validto, " 
   									"valueparametername, valueparameterunit, levelparametername, " 
   									"levelunitname, levelfrom, levelto, dataversion";
	
	class myProviderList {
		wdb2ts::ProviderList *list;
		
	public:
		myProviderList(  wdb2ts::ProviderList *list_ )
			:list( list_ )
			{}
		
		void push_back( const std::string &elem ) {
			list->push_back( wdb2ts::ProviderItem( elem ) );
		}
	};
	
}

namespace wdb2ts {

using namespace boost::posix_time;

using namespace std;

WdbQueryHelper::
WdbQueryHelper()
	: urlQuerys( dummyQuerys ), first( true )
{
	itNext = urlQuerys.end();
}
	
WdbQueryHelper::
WdbQueryHelper( const wdb2ts::config::Config::Query &urlQuerys, int wciProtocol )
	: urlQuerys( urlQuerys ), first( true ), webQuery( wciProtocol, returnColoumns )
{
	itNext = urlQuerys.end();
}	
	

std::string 
WdbQueryHelper::
dataprovider()const
{
	
	if( ! dataProviders.empty() )
		return dataProviders;
	
	ostringstream ost;
	int i=0;
	std::list<std::string>::const_iterator it;
	
	for( it=webQuery.dataprovider.valueList.begin();
		  it != webQuery.dataprovider.valueList.end();
		  ++it, ++i) {
		if( ! i ) 
			ost << *it;
		else
			ost << "," << *it;
	}
	
	const_cast<WdbQueryHelper*>(this)->dataProviders = ost.str();
	return dataProviders;
}

void
WdbQueryHelper::
doGetProviderReftime( const std::string &provider, 
		                boost::posix_time::ptime &refTimeFrom,  
		                boost::posix_time::ptime &refTimeTo ) const
{
	ProviderItem pvItemIn = ProviderList::decodeItem( provider );
	ProviderItem pvItem;
	
	cerr << "doGetProviderReftime: providerIn: " << provider 
	     << " '" << pvItemIn.providerWithPlacename() << "'" << endl;
	
	for( ProviderRefTimeList::const_iterator it = reftimes.begin();
			it != reftimes.end(); ++it ) {
		cerr << "    " << it->first << " -> " << it->second.refTime << endl;
		pvItem = ProviderList::decodeItem( it->first );
		
		if( ( !pvItemIn.placename.empty() && pvItemIn == pvItem ) ||
			 ( pvItemIn.placename.empty() && pvItemIn.provider == pvItem.provider ) ) 
		{
			if( refTimeFrom.is_special() ) {
				refTimeFrom = it->second.refTime;
				refTimeTo = refTimeFrom;
			}else if( refTimeTo < it->second.refTime ) {
				refTimeTo = it->second.refTime;
			}else if( refTimeFrom > it->second.refTime ) {
				refTimeFrom = it->second.refTime;
			}
			
			if( ! pvItemIn.placename.empty() )
				return;
		}
	}
}



bool
WdbQueryHelper::
getProviderReftime( const std::string &provider, 
		              boost::posix_time::ptime &refTimeFrom,  
		              boost::posix_time::ptime &refTimeTo ) 
{
	ProviderItem pvItemIn = ProviderList::decodeItem( provider );
	
	refTimeFrom = boost::posix_time::ptime(); //undef
	cerr << "getProviderReftime: " << provider << endl;

	doGetProviderReftime( provider, refTimeFrom, refTimeTo ); 
		
	if( refTimeFrom.is_special() ) {
		cerr << " --- Return: getProviderReftime: No reftime found. " << endl;
		return false;
	}
 
	cerr << " --- Return: getProviderReftime:" << pvItemIn.providerWithPlacename() 
	     << "  " << refTimeFrom << " - " << refTimeTo << endl;
	
	if( refTimeTo == refTimeFrom )
		refTimeFrom_IsEqualTo_ReftTimeTo = true;
	else
		refTimeFrom_IsEqualTo_ReftTimeTo = false;
	
	return true;
}

bool
WdbQueryHelper::
getProviderReftime( const std::list<std::string> &providerList, 
		              boost::posix_time::ptime &refTimeFrom,  
		              boost::posix_time::ptime &refTimeTo ) 
{
	string log;

	refTimeFrom = boost::posix_time::ptime(); //undef
	
	for( std::list<std::string>::const_iterator it=providerList.begin();
	     it != providerList.end();
	     ++ it )
	{
		doGetProviderReftime( *it, refTimeFrom, refTimeTo );
		if( it != providerList.end() )
			log += ", ";
		
		log += *it;
	}
		
	if( refTimeFrom.is_special() ) {
		cerr << " --- Return: getProviderReftime: No reftime found. [" << log << "]" << endl;
		return false;
	}
 
	cerr << " --- Return: getProviderReftime: [" << log << "]"
	     << "  " << refTimeFrom << " - " << refTimeTo << endl;
	
	if( refTimeTo == refTimeFrom )
		refTimeFrom_IsEqualTo_ReftTimeTo = true;
	else
		refTimeFrom_IsEqualTo_ReftTimeTo = false;
	
	return true;
}


std::string
WdbQueryHelper::
getDataversionString( const std::list<std::string> &dataproviderList ) const
{
	ProviderItem pvItemIn;
	ProviderItem pvItem;
	std::set<int> dvSet;
	
	for( std::list<std::string>::const_iterator it = dataproviderList.begin();
	     it != dataproviderList.end();
	     ++it )
	{
		pvItemIn = ProviderList::decodeItem( *it );

		for( ProviderRefTimeList::const_iterator rit = reftimes.begin();
				rit != reftimes.end(); ++rit ) 
		{
			pvItem = ProviderList::decodeItem( rit->first );
				
			if( ( !pvItemIn.placename.empty() && pvItemIn == pvItem ) ||
				 ( pvItemIn.placename.empty() && pvItemIn.provider == pvItem.provider ) ) 
			{
				dvSet.insert( rit->second.dataversion );
					
				if( ! pvItemIn.placename.empty() )
					break;
			}
		}
	}
	
	if( dvSet.empty() )
		return "-1";
	
	ostringstream ost;
	std::set<int>::iterator it = dvSet.begin();
	
	ost << *it;
	for( ++ it ; it != dvSet.end(); ++it )
		ost << "," << *it; 
	
	return ost.str();
}


void 
WdbQueryHelper::
init( float lat, float lon, 
		const ProviderRefTimeList &reftimes_,
		const ProviderList &providerPriority_,
		const std::string &extraParams )
{
	ostringstream ost;
	first=true;
	
	reftimes = reftimes_;
	providerPriority = providerPriority_;

	itCurProviderPriority = curProviderPriority.end();
	
	ost << "lat=" << lat << ";lon=" << lon << ";";
	
	if( ! extraParams.empty() ) {
		if( extraParams[0] == ';')
			ost << extraParams.substr( 1 );
		else
			ost << extraParams;
		
		if( extraParams[extraParams.length()-1]!=';' )
			ost << ";";
	}
	
	position = ost.str();
}
	
bool 
WdbQueryHelper::
hasNext( )
{
	USE_MI_PROFILE;
	START_MARK_MI_PROFILE("WdbQueryHelper::hasNext");
	
	ptime refTimeFrom;
	ptime refTimeTo;
	dataProviders.erase();
	
	if( first ) {
		itNext = urlQuerys.begin();
	}
				
	
	if( itCurProviderPriority != curProviderPriority.end() ) {
		for( itCurProviderPriority++
			 ; itCurProviderPriority != curProviderPriority.end() && 
		      ! getProviderReftime( (reftimeProvider.empty()?itCurProviderPriority->provider:reftimeProvider), 
		      		                 refTimeFrom, refTimeTo )
		    ; ++itCurProviderPriority);
	}
	
	
	if( itCurProviderPriority != curProviderPriority.end() ) {
		webQuery.dataprovider.decode( itCurProviderPriority->provider );
		webQuery.reftime.decode( miutil::isotimeString( refTimeFrom, true, true ) + "," +
				                   miutil::isotimeString( refTimeTo, true, true ) );
		webQuery.dataversion.decode( getDataversionString( webQuery.dataprovider.valueList ) );
		
		STOP_MARK_MI_PROFILE("WdbQueryHelper::hasNext");
		return true;
	}
	
	while( itNext != urlQuerys.end() ) {
		if( ! first )  
			++itNext;
		else	
			first = false;
	
		if( itNext == urlQuerys.end() )
			return false;

		reftimeProvider.erase();
		string q( position );
		
		q = position + itNext->query();
		queryMustHaveData = itNext->probe();
		
		webQuery.decode( q );
	
		curProviderPriority.clear();
		itCurProviderPriority = curProviderPriority.end();
		
		if( webQuery.reftime.fromTime.is_special() && ! webQuery.reftime.isDecoded ) {
			if( webQuery.dataprovider.valueList.empty() ) {
				if( providerPriority.empty() ) {
					STOP_MARK_MI_PROFILE("WdbQueryHelper::hasNext");
					throw logic_error( "EXCEPTION: missing provider in the query." );
				}
	
				curProviderPriority = providerPriority;
			} else {
				for( UrlParamDataProvider::ValueList::const_iterator itValueList = webQuery.dataprovider.valueList.begin();
				     itValueList != webQuery.dataprovider.valueList.end();
				     ++itValueList ) {
					curProviderPriority.push_back( ProviderItem( *itValueList ) );
				}
				/*
				copy( webQuery.dataprovider.valueList.begin(),
					   webQuery.dataprovider.valueList.end(),
					   back_inserter( myProviderList( &curProviderPriority) ) ); */  	
			}
		
			for( itCurProviderPriority = curProviderPriority.begin();
			     itCurProviderPriority != curProviderPriority.end() ;
			     ++itCurProviderPriority ) 
			{
				cerr << "getProviderRefTime: " << itCurProviderPriority->provider << " " 
				     << refTimeFrom << " - " << refTimeTo << endl;
				if( ! getProviderReftime( itCurProviderPriority->provider, refTimeFrom, refTimeTo ) ) {
					cerr << " --- Not found" << endl;
					continue;
				}
				cerr << " --- found" << endl;
				break;
			}
		
			if( itCurProviderPriority != curProviderPriority.end() ) {
				webQuery.dataprovider.decode( itCurProviderPriority->provider );
				webQuery.reftime.decode( miutil::isotimeString( refTimeFrom, true, true ) + "," +
							                miutil::isotimeString( refTimeTo, true, true ) );
				webQuery.dataversion.decode( getDataversionString( webQuery.dataprovider.valueList ) );
				STOP_MARK_MI_PROFILE("WdbQueryHelper::hasNext");
				return true;
			}
			
			continue;
		} else {
			if( webQuery.dataprovider.valueList.empty() ) {
				if( providerPriority.empty() ) {
					STOP_MARK_MI_PROFILE("WdbQueryHelper::hasNext");
					throw logic_error( "EXCEPTION: missing provider in the query." );
				}
				
				webQuery.dataprovider.setValueList( providerPriority.providerWithoutPlacename() );
			} 
			
			if( webQuery.reftime.sameTimespecAs( reftimeProvider ) ) {
				if( ! getProviderReftime( reftimeProvider, refTimeFrom, refTimeTo ) ) 
					continue;
				
				webQuery.reftime.decode( miutil::isotimeString( refTimeFrom, true, true ) + "," +
						                   miutil::isotimeString( refTimeTo, true, true ) );
			} else if( webQuery.reftime.useProviderList( ) ) {
				if( ! getProviderReftime( webQuery.dataprovider.valueList, refTimeFrom, refTimeTo ) ) 
					continue;
				
				webQuery.reftime.decode( miutil::isotimeString( refTimeFrom, true, true ) + "," +
						                   miutil::isotimeString( refTimeTo, true, true ) );
			}
			
			webQuery.dataversion.decode( getDataversionString( webQuery.dataprovider.valueList ) );
			STOP_MARK_MI_PROFILE("WdbQueryHelper::hasNext");
			return true;
		}
		
		webQuery.dataversion.decode( getDataversionString( webQuery.dataprovider.valueList ) );
		
		STOP_MARK_MI_PROFILE("WdbQueryHelper::hasNext");
		return true;
	}

	STOP_MARK_MI_PROFILE("WdbQueryHelper::hasNext");
	return false;
}
	



std::string 
WdbQueryHelper::
next(  bool &mustHaveData )
{
	mustHaveData = queryMustHaveData;
	
	if( refTimeFrom_IsEqualTo_ReftTimeTo )
		return webQuery.wciReadQuery();
	
	return webQuery.wciReadQuery() + " ORDER BY referencetime";
}

}
