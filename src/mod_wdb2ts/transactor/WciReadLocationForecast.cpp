/*
    wdb - weather and water data storage

    Copyright (C) 2008 met.no

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

#include <transactor/WciReadLocationForecast.h>
#include <WdbQueryHelper.h>
#include <wdb2tsProfiling.h>
#include <Logger4cpp.h>

DECLARE_MI_PROFILE;

using namespace std;

namespace wdb2ts {

WciReadLocationForecast::
WciReadLocationForecast(float latitude_, float longitude_, int altitude_,
      						const ParamDefList &paramDefs_,
      						PtrProviderRefTimes refTimes_,
      						const ProviderList &providerPriority_,
      						const wdb2ts::config::Config::Query &urlQuerys_,
      						int wciProtocol_ )
	: latitude( latitude_ ), longitude( longitude_), altitude( altitude_ ),
	  paramDefs( paramDefs_ ), refTimes( refTimes_ ), providerPriority( providerPriority_ ),
	  urlQuerys( urlQuerys_ ), wciProtocol( wciProtocol_ ), timeSerie( new TimeSerie() )
{
}

WciReadLocationForecast::
~WciReadLocationForecast()
{
}


void 
WciReadLocationForecast::
on_abort( const char msg_[] )throw () 
{
	timeSerie->clear();
}
	

void 
WciReadLocationForecast::
operator () ( argument_type &t )
{
	string wciReadQuery;
	string dbProfileProvider__; //Only used when profiling
	string decodeProfileProvider__; //Only used when profiling
	bool   mustHaveData;
	
	WEBFW_USE_LOGGER( "handler" );

//	timeSerie->clear();
	
	USE_MI_PROFILE;
	MARK_ID_MI_PROFILE("requestWdb");
	
	if( urlQuerys.empty() ) {
   	MARK_ID_MI_PROFILE("requestWdb");
		throw logic_error( "EXCEPTION: No wdb querys is defined." );
	}
	
	WdbQueryHelper wdbQueryHelper( urlQuerys, wciProtocol );
	
	try {
   	MARK_ID_MI_PROFILE("WciReadLocationForecast::init");
		wdbQueryHelper.init( latitude, longitude, *refTimes, providerPriority );
		MARK_ID_MI_PROFILE("WciReadLocationForecast::init");
		
		while ( wdbQueryHelper.hasNext() ) {
			try {
				START_MARK_MI_PROFILE("WciReadLocationForecast::wdbQueryHelper::next");
				wciReadQuery = wdbQueryHelper.next( mustHaveData );
				STOP_MARK_MI_PROFILE("WciReadLocationForecast::wdbQueryHelper::next");
				
			}
			catch( const NoReftime &exNoReftime ) {
				WEBFW_LOG_WARN(exNoReftime.what());
				continue;
			}
			WEBFW_LOG_DEBUG( "SQL [" << wciReadQuery << "]" );
			
#ifdef __MI_PROFILE__
			dbProfileProvider__ = "db::("+wdbQueryHelper.dataprovider()+")";
			decodeProfileProvider__ = "decode::("+wdbQueryHelper.dataprovider()+")";
#endif
			MARK_ID_MI_PROFILE( dbProfileProvider__ );
			pqxx::result  res = t.exec( wciReadQuery );
			MARK_ID_MI_PROFILE( dbProfileProvider__ );
		   
			if( mustHaveData && res.empty() ) {
				//WEBFW_LOG_ERROR( "**** The query must have data!" );
				timeSerie->clear();
				return;
			}
			
			MARK_ID_MI_PROFILE( decodeProfileProvider__ );
			decodePData( paramDefs, providerPriority, *refTimes, wciProtocol, res, *timeSerie );
			MARK_ID_MI_PROFILE( decodeProfileProvider__ );
		}
	}
	catch( const exception &ex ){
		WEBFW_LOG_ERROR( "WciReadLocationForecast: query [" << wciReadQuery << "] reason: " << ex.what() );
		throw;
	}	
	MARK_ID_MI_PROFILE("requestWdb");
}

}

