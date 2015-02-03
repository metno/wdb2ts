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


#ifndef __UPDATEPROVIDERREFTIMES_H__
#define __UPDATEPROVIDERREFTIMES_H__


#include <ProviderReftimes.h>

#include <DbManager.h>

namespace wdb2ts {

typedef boost::shared_ptr<ProviderRefTimeList> PtrProviderRefTimes;


/**
 * @exception std::logic_error on db failure.
 */

bool
updateProviderRefTimes( WciConnectionPtr wciConnection, 
		                ProviderRefTimeList &refTimes,
		                const ProviderList &providers,
		                int wciProtocol );

/**
 * Checks that we have data in wdb for the the providers
 * and requested reference times.
 *
 * @throws std::logical_error if the reftimes, dataversion or reftimes
 * do not exist in the database.
 */
bool
updateProviderRefTimes( WciConnectionPtr wciConnection, 
						const ProviderRefTimeList &requestedUpdates,
		                ProviderRefTimeList &refTimes,
		                int wciProtocol );

}


#endif 
