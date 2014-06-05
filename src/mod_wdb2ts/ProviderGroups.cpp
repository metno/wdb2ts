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


#include <list>
#include <ProviderGroups.h>
#include <ProviderList.h>
#include <transactor/ResolveProviderGroups.h>
#include <Logger4cpp.h>


namespace wdb2ts {

ProviderGroups::
ProviderGroups()
{
}

ProviderGroups::
ProviderGroups( const ProviderGroups &pg)
   : reverseLookupTable( pg.reverseLookupTable )
{
}

ProviderGroups
ProviderGroups::
operator=( const ProviderGroups &rhs )
{
   if( this != &rhs ) {
      reverseLookupTable = rhs.reverseLookupTable;
   }

   return *this;
}

std::string
ProviderGroups::
lookUpGroupName( const std::string &providerName )const
{
   std::map<std::string, std::string>::const_iterator it=reverseLookupTable.find( providerName );

   if( it != reverseLookupTable.end() )
      return it->second;
   else
      return providerName;
}

#if 0
void
ProviderGroups::
resolve( Wdb2TsApp &app, const std::string &wdbid, const std::list<std::string> &providerList )
{
   WciConnectionPtr wciConnection;

   WEBFW_USE_LOGGER( "handler" );

   try {
      wciConnection = app.newWciConnection( wdbid );
   }
   catch( const std::exception &ex ) {
      WEBFW_LOG_ERROR( "ResolveProviderGroups: NO DB CONNECTION. " << ex.what() );
      return;
   }
   catch( ... ) {
      WEBFW_LOG_ERROR( "ResolveProviderGroups: NO DB CONNECTION. unknown exception ");
      return;
   }

   try {
      ResolveProviderGroups resolveGroups;
      wciConnection->perform( resolveGroups, 3 );
      reverseLookupTable  = *resolveGroups.result();
   }
   catch( const std::ios_base::failure &ex ) {
      WEBFW_LOG_ERROR( "std::ios_base::failure: ResolveProviderGroups: " << ex.what() );
   }
   catch( const std::runtime_error &ex ) {
      WEBFW_LOG_ERROR( "std::runtime_error: ResolveProviderGroups:: " << ex.what() );
   }
   catch( const std::logic_error &ex ) {
      WEBFW_LOG_ERROR( "std::logic_error: ResolveProviderGroups:: " << ex.what() );
   }
   catch( ... ) {
      WEBFW_LOG_ERROR( "EXCEPTION: unknown: ResolveProviderGroups." );
   }

   /*
    * Filter the providers so that provider names that is not a groupname
    * resolves to the specific provider in the config file, even if the provider is
    * part of a group.
    */
   std::map<std::string, std::string>::iterator itrl;

   for( std::list<std::string>::const_iterator it=providerList.begin(); it != providerList.end(); ++it ) {
      itrl = reverseLookupTable.find( *it );

      if( itrl == reverseLookupTable.end() )
         continue;

      itrl->second = *it;
   }


}
#endif
}

