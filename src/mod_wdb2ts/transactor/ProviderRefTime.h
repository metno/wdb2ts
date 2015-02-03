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

#ifndef __PROVIDER_REF_TIME_H__
#define __PROVIDER_REF_TIME_H__


#include <pqxx/transactor>
#include <string>
#include <UpdateProviderReftimes.h>

namespace wdb2ts {


/**
 * Transactor class to fetch provider data from an wdb database.
 * The data is returned in as an PtrProviderRefTimes.
 * 
 */
class  ProviderRefTime
	: public pqxx::transactor<>
{
public:
	ProviderRefTime( const ProviderRefTimeList &refTimes, 
         			  const std::string &provider,
         			  const std::string &reftimespec,
					  int dataversion=-1);
	
	~ProviderRefTime();

	void operator () ( argument_type &t );

	PtrProviderRefTimes result() const { return refTimesResult; }

private:
	const ProviderRefTimeList refTimes;
	const std::string provider;
	const std::string reftimespec;
	int dataversion;
	PtrProviderRefTimes refTimesResult;
};

}

#endif 
