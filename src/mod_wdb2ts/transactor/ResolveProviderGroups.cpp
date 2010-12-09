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

#include <list>
#include <transactor/ResolveProviderGroups.h>
#include <wdb2tsProfiling.h>
#include <Logger4cpp.h>

DECLARE_MI_PROFILE;
using namespace std;

namespace {
   struct Provider {
      string dataprovidertype;
      string dataprovidername;
      int dataprovidernameleftset;
      int dataprovidernamerightset;
   };

   struct ProviderGroup {
      string groupname;
      int dataprovidernameleftset;
      int dataprovidernamerightset;
      list<string> providers;

      ProviderGroup( const Provider &p )
         : groupname( p.dataprovidername ), dataprovidernameleftset( p.dataprovidernameleftset ),
           dataprovidernamerightset( p.dataprovidernamerightset ) {}
   };

   void
   addToGroup( list<ProviderGroup> &groups, const Provider &provider ) {
      for( list<ProviderGroup>::iterator it=groups.begin(); it != groups.end(); ++it ) {
         if( provider.dataprovidernameleftset >= it->dataprovidernameleftset &&
             provider.dataprovidernamerightset <= it->dataprovidernamerightset  ) {
            it->providers.push_back( provider.dataprovidername );
            return;
         }
      }
   }
}




namespace wdb2ts {

ResolveProviderGroups::
ResolveProviderGroups()
   : lookUpTable( new map<string, string>() )
{
}
	
ResolveProviderGroups::
~ResolveProviderGroups()
{
}

void
ResolveProviderGroups::
operator () ( argument_type &t )
{
   WEBFW_USE_LOGGER( "wdb" );

   ostringstream ost;
   Provider provider;
   list<Provider> myProviderList;
   list<ProviderGroup> providerGroupList;
   int num;

   ost << "select * from wci.getdataprovider(NULL)";

   USE_MI_PROFILE;
   MARK_ID_MI_PROFILE("ResolveProviderGroups (Transactor)");

   try {
      pqxx::result  res = t.exec( ost.str() );

      if( res.size() == 0 ) {
         WEBFW_LOG_ERROR("ResolveProviderGroups: No dataproviders??????");
         return;
      }

      for( pqxx::result::const_iterator it=res.begin(); it != res.end(); ++it  ) {
         provider.dataprovidertype = it.at("dataprovidertype").c_str();
         provider.dataprovidername = it.at("dataprovidername").c_str();
         provider.dataprovidernameleftset = it.at("dataprovidernameleftset").as<int>();
         provider.dataprovidernamerightset = it.at("dataprovidernamerightset").as<int>();

         num = provider.dataprovidernamerightset - provider.dataprovidernameleftset;

         if( num < 0 )
            continue;

         if( num > 1 ) {
            providerGroupList.push_back( ProviderGroup( provider ) );
         } else { //num == 1
            myProviderList.push_back( provider );
         }
      }
   }
   catch( const std::exception &ex ) {
      WEBFW_LOG_ERROR("ResolveProviderGroups: EXCEPTION: Reason: " << ex.what() );
   }
   catch( ... ) {
      WEBFW_LOG_ERROR("ResolveProviderGroups: EXCEPTION: Reason: Unkown!");
      MARK_ID_MI_PROFILE("ResolveProviderGroups (Transactor)");
      throw;
   }

   for( list<Provider>::iterator it=myProviderList.begin(); it != myProviderList.end(); ++it )
      addToGroup( providerGroupList, *it );

   ost.str("");

   for( list<ProviderGroup>::iterator it= providerGroupList.begin(); it != providerGroupList.end(); ++it ){
      if( it->providers.empty() )
         continue;

      ost << "    providergroup: " << it->groupname << " (";
      for(list<string>::iterator pit=it->providers.begin(); pit != it->providers.end(); ++pit ) {
         ost << (pit==it->providers.begin()?"":", ") << *pit;
         (*lookUpTable)[*pit] = it->groupname;
      }
      ost << ")" << endl;
   }

   WEBFW_LOG_DEBUG("ProviderGroups: " << "\n" << ost.str() );

   MARK_ID_MI_PROFILE("ProviderRefTimes (Transactor)");
}


}

