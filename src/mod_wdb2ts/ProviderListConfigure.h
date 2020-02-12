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


#ifndef __PROVIDER_LIST_CONFIGURE_H__
#define __PROVIDER_LIST_CONFIGURE_H__

#include <ProviderList.h>
#include <map>

namespace wdb2ts {

class Wdb2TsApp;

/**
 * For providers that is not defined with a placename, search the database and 
 * set the placename. The priority for the placename for these providers is
 * unpredictable.
 */
ProviderList
providerPrioritySetPlacename( const ProviderList &pvList, 
										const std::string &wdbDB,
										Wdb2TsApp *app );

ProviderList
providerPrioritySetPlacename( const ProviderItem &pvItem,
				              const std::string &wdbDB,
							  Wdb2TsApp *app );


ProviderList
configureProviderList( const wdb2ts::config::ActionParam &params, 
		                 const std::string &wdbDB,
		                 Wdb2TsApp *app );

ProviderList
configureProviderList( const wdb2ts::config::ActionParam &params,
		                 const std::list<std::string> &wdbDBs,
		                 Wdb2TsApp *app );

}
#endif 
