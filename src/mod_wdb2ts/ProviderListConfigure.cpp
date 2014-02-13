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


#include <algorithm>
#include <iostream>
#include <sstream>
#include <replace.h>
#include <splitstr.h>
#include <trimstr.h>
#include <transactor/ProviderRefTime.h>
#include <RequestConf.h>
#include <ProviderListConfigure.h>
#include <wdb2TsApp.h>
#include <Logger4cpp.h>


using namespace std;


namespace wdb2ts {

ProviderList
providerPrioritySetPlacename( const ProviderList &pvList, 
				              const std::string &wdbDB,
							  Wdb2TsApp *app )
{
	WciConnectionPtr wciConnection;
	ProviderList resList;
	WEBFW_USE_LOGGER( "handler" );
	
	try {
		wciConnection = app->newWciConnection( wdbDB );
	}
	catch( exception &ex ) {
		WEBFW_LOG_ERROR( "LocationForecastHandler::providerPrioritySetPlacename: NO DB CONNECTION. " << ex.what() );
		return resList;
	}
	catch( ... ) {
		WEBFW_LOG_ERROR( "LocationForecastHandler::providerPrioritySetPlacename: NO DB CONNECTION. unknown exception ");
		return resList;
	}
	
	for( ProviderList::const_iterator it=pvList.begin(); it != pvList.end(); ++it ) {
		if( ! it->placename.empty() ) {
			resList.push_back( *it );
			continue;
		}
		
		try {
			ProviderRefTimeList dummyRefTimeList;
			ProviderRefTime providerReftimeTransactor( dummyRefTimeList, 
					                                   it->provider,
					                                   "NULL" );

			wciConnection->perform( providerReftimeTransactor, 3 );
			PtrProviderRefTimes res = providerReftimeTransactor.result();
			
			if( ! res )
				continue;
			
			for( ProviderRefTimeList::iterator pit = res->begin(); 
			     pit != res->end(); 
			     ++pit ) {
				ProviderItem tmp = ProviderList::decodeItem( pit->first );

				if( ! tmp.provider.empty() )
					resList.push_back( tmp );
			}
		}
		catch( const std::ios_base::failure &ex ) {
			WEBFW_LOG_ERROR( "std::ios_base::failure: LocationForecastHandler::providerPrioritySetPlacename: " << ex.what() );
		}
		catch( const std::runtime_error &ex ) {
			WEBFW_LOG_ERROR( "std::runtime_error: LocationForecastHandler::providerPrioritySetPlacename: " << ex.what() );
		}
		catch( const std::logic_error &ex ) {
			WEBFW_LOG_ERROR( "std::logic_error: LocationForecastHandler::providerPrioritySetPlacename: " << ex.what() );
		}
		catch( ... ) {
			WEBFW_LOG_ERROR( "unknown: LocationForecastHandler::providerPrioritySetPlacename" );
		}
	}

	return resList;			
}


ProviderList
configureProviderList( const wdb2ts::config::ActionParam &params, 
		                 const std::string &wdbDB,
		                 Wdb2TsApp *app )
{
	ProviderList providerPriority = providerListFromConfig( params );
	ProviderList retProviderPriority;

	WEBFW_USE_LOGGER( "handler" );
	
	if( ! providerPriority.empty() ) {
		retProviderPriority = providerPrioritySetPlacename( providerPriority, wdbDB, app );
		
      std::ostringstream logMsg;
		logMsg << "configureProviderPriority: ProviderPriority: defined (resolved).\n";

		for( ProviderList::size_type i=0; i < retProviderPriority.size(); ++i )
			logMsg << "  " << retProviderPriority[i].providerWithPlacename() << endl;
		WEBFW_LOG_DEBUG(logMsg.str());
	}

//	cerr << "configureProviderList: #" << retProviderPriority.size() << endl;
//	for( ProviderList::iterator it = retProviderPriority.begin(); it!= retProviderPriority.end(); ++it )
//		cerr << "configureProviderList: '" << it->providerWithPlacename() << "'" << endl;

	return retProviderPriority;
}




}


