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

#ifndef __PROVIDERGROUPSRESOLVE_H__
#define __PROVIDERGROUPSRESOLVE_H__

#include <string>
#include <map>
#include <list>
#include "ProviderGroups.h"

namespace wdb2ts {

class Wdb2TsApp;
class ParamDefList;

/**
 * Use wdbs wci.getdataproviderto build the reverse lookup table.
 *
 * @param app The wdb2ts Application class. We need this to get a db connection.
 * @param wdbid The database to use.
 */

ProviderGroups
providerGroupsResolve( Wdb2TsApp &app, const std::string &wdbid, const std::list<std::string> &providerList );

void
paramDefListRresolveProviderGroups( Wdb2TsApp &app, ParamDefList &paramDefList, const std::string &wdbid );
}

#endif
