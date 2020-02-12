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

#ifndef __PROVIDERGROUPS_H__
#define __PROVIDERGROUPS_H__

#include <string>
#include <map>
#include <list>

namespace wdb2ts {

class ProviderList;

class Wdb2TsApp;
/**
 * ProviderGroups is an helper class to find the groupname an
 * provider belongs to.
 *
 * When we query an wci.read with a provider groupname, as provider, the returned
 * result will have the real providername in  the resultset, not the groupname. We
 * want to find back to the group name.
 *
 * This class provide a reverse lookup table to resolve the group name from the returned
 * provider name from wci.read.
 *
 * To build the reverse lookup table we use wci.getdataprovider.
 *
 * The ProviderGroups must be resolved every time wdb2ts is updated trough an update request.
 */
class ProviderGroups {
   std::map<std::string, std::string> reverseLookupTable;

public:
   ProviderGroups();
   ProviderGroups( const ProviderGroups &pg);
   ProviderGroups operator=( const ProviderGroups &rhs );

   void clear(){ reverseLookupTable.clear(); }
   /**
    * Try to resolve the group name from dataprovider. If
    * the groupname can't be resolved return the dataprovider.
    *
    * @param providerName The provider name to resolve.
    * @return The provider group name if resolved or providerName if not.
    */
   std::string lookUpGroupName( const std::string &providerName )const;

//   /**
//    * Use wdbs wci.getdataproviderto build the reverse lookup table.
//    *
//    * @param app The wdb2ts Application class. We need this to get a db connection.
//    * @param wdbid The database to use.
//    */
//   void resolve( Wdb2TsApp &app, const std::string &wdbid, const std::list<std::string> &providerList );

#ifndef NODB
   friend ProviderGroups providerGroupsResolve( Wdb2TsApp &app, const std::string &wdbid, const std::list<std::string> &providerList );
#endif
};


}

#endif /* PROVIDERGROUPS_H_ */
