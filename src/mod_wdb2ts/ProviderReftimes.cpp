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
#include <ProviderReftimes.h>
#include <Logger4cpp.h>

using namespace std;
using namespace boost::posix_time; //ptime, second_clock
using namespace miutil; //isotimeString, ptimeFromIsoString 


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
ProviderRefTimeList::
dataversion( const std::string &provider, int &dataversion_ ) const
{
   ProviderItem pvItemIn = ProviderList::decodeItem( provider );
   ProviderItem pvItem;

   for( ProviderRefTimeList::const_iterator it = begin();
         it != end(); ++it ) {

      pvItem = ProviderList::decodeItem( it->first );

      if( ( !pvItemIn.placename.empty() && pvItemIn == pvItem ) ||
            ( pvItemIn.placename.empty() && pvItemIn.provider == pvItem.provider ) )
         {
         dataversion_ = it->second.dataversion;
         return true;
         }
   }

   return false;
}

bool
ProviderRefTimeList::
providerReftimeDisabledAndDataversion( const std::string &provider,
                                       boost::posix_time::ptime &refTime,
                                       bool &disabled,
                                       int &dataversion ) const
{

   ProviderItem pvItemIn = ProviderList::decodeItem( provider );
   ProviderItem pvItem;

   for( ProviderRefTimeList::const_iterator it = begin();
         it != end(); ++it ) {

      pvItem = ProviderList::decodeItem( it->first );

      if( ( !pvItemIn.placename.empty() && pvItemIn == pvItem ) ||
            ( pvItemIn.placename.empty() && pvItemIn.provider == pvItem.provider ) ) {
         refTime = it->second.refTime;
         disabled = it->second.disabled;
         dataversion = it->second.dataversion;
         return true;
      }
   }

   return false;

}

int
ProviderRefTimeList::
updateDisableStatus( const std::string &provider, bool disable )
{
   ProviderItem pvItemIn = ProviderList::decodeItem( provider );
   ProviderItem pvItem;
   int count=0;

   for( ProviderRefTimeList::iterator it = begin();
         it != end(); ++it ) {

      pvItem = ProviderList::decodeItem( it->first );

      if( ( !pvItemIn.placename.empty() && pvItemIn == pvItem ) ||
          ( pvItemIn.placename.empty() && pvItemIn.provider == pvItem.provider ) ) {
         if( it->second.disabled != disable ) {
            it->second.disabled = disable;
            ++count;
         }
      }
   }

   return count;
}

int
ProviderRefTimeList::
updateDataversion( const std::string &provider, int dataversion )
{
   ProviderItem pvItemIn = ProviderList::decodeItem( provider );
   ProviderItem pvItem;
   int count=0;

   for( ProviderRefTimeList::iterator it = begin();
         it != end(); ++it ) {

      pvItem = ProviderList::decodeItem( it->first );

      if( ( !pvItemIn.placename.empty() && pvItemIn == pvItem ) ||
          ( pvItemIn.placename.empty() && pvItemIn.provider == pvItem.provider ) ) {
         if( it->second.dataversion != dataversion ) {
            it->second.dataversion = dataversion;
            ++count;
         }
      }
   }

   return count;
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


