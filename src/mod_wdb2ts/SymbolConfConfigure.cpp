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

#include <ctype.h>
#include <stdio.h>
#include <SymbolConfConfigure.h>
#include <trimstr.h>
#include <splitstr.h>
#include <ProviderList.h>
#include <UpdateProviderReftimes.h>
#include <transactor/ProviderRefTime.h>
#include <wdb2TsApp.h>
#include <Logger4cpp.h>

namespace wdb2ts {

using namespace std;

SymbolConfProvider
symbolConfProviderSetPlacename( const SymbolConfProvider &symbolConfProvider, 
		                          WciConnectionPtr wciConnection )
{
	WEBFW_USE_LOGGER( "handler" );

	SymbolConfProvider resList;
		
	for( SymbolConfProvider::const_iterator it=symbolConfProvider.begin(); 
	     it != symbolConfProvider.end(); 
	     ++it ) 
	{
		ProviderItem pi = ProviderList::decodeItem( it->first );
		
		if( ! pi.placename.empty() ) {
			resList.add( pi.providerWithPlacename(), it->second );
			continue;
		}
		
		try {
			ProviderRefTimeList dummyRefTimeList;
			ProviderRefTime providerReftimeTransactor( dummyRefTimeList, 
					                                     pi.provider, 
					                                     "NULL" );

			wciConnection->perform( providerReftimeTransactor, 3 );
			PtrProviderRefTimes res = providerReftimeTransactor.result();
			
			if( ! res )
				continue;
			
			for( ProviderRefTimeList::iterator pit = res->begin(); 
			     pit != res->end(); 
			     ++pit ) {
				ProviderList tmp = ProviderList::decode( pit->first, false );
				
				for( ProviderList::size_type i=0; i < tmp.size(); ++i )
					resList.add( tmp[i].providerWithPlacename(), it->second );
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

SymbolConfProvider
symbolConfProviderWithPlacename( const wdb2ts::config::ActionParam &params, 
											const std::string &wdbDB,                           
											Wdb2TsApp *app
                               )
{
	WciConnectionPtr wciConnection;
	SymbolConfProvider tmpList;

	WEBFW_USE_LOGGER( "handler" );
		
	try {
		wciConnection = app->newWciConnection( wdbDB );
	}
	catch( exception &ex ) {
		WEBFW_LOG_ERROR( "symbolConfProviderWithPlacename: NO DB CONNECTION. " << ex.what() );
		return tmpList;
	}
	catch( ... ) {
		WEBFW_LOG_ERROR( "symbolConfProviderWithPlacename: NO DB CONNECTION. unknown exception " );
		return tmpList;
	}

	configureSymbolconf( params, tmpList );

	return symbolConfProviderSetPlacename( tmpList, wciConnection );
}


SymbolConfProvider
symbolConfProviderWithPlacename( const wdb2ts::config::ActionParam &params,
		                         const std::list<std::string> &wdbDBs,
								 Wdb2TsApp *app
                               )
{
	WEBFW_USE_LOGGER( "handler" );

	SymbolConfProvider res;
	SymbolConfProvider tmpList;
	configureSymbolconf( params, tmpList );

	for( auto &wdbDB : wdbDBs ){
		try {
			WciConnectionPtr wciConnection=app->newWciConnection( wdbDB );
			SymbolConfProvider tmpRes=symbolConfProviderSetPlacename( tmpList, wciConnection );

			if( tmpRes.empty())
				continue;
			res.merge(tmpRes);
		}
		catch( exception &ex ) {
			WEBFW_LOG_ERROR( "symbolConfProviderWithPlacename: NO DB CONNECTION. " << ex.what() );

		}
		catch( ... ) {
			WEBFW_LOG_ERROR( "symbolConfProviderWithPlacename: NO DB CONNECTION. unknown exception " );
		}
	}

	return tmpList;
}


}

