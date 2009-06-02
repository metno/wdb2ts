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

#ifndef __WCI_READ_H__
#define __WCI_READ_H__


#include <boost/date_time/posix_time/posix_time.hpp>
#include <pqxx/transactor>
#include <string>
#include <Config.h>

namespace wdb2ts {


class WciReadHelper
{
public:
	WciReadHelper();
	virtual ~WciReadHelper();
	
	///id return an id to use in logging.
	virtual std::string id();
	virtual std::string query()=0; 
	virtual void clear();
	virtual void doRead( pqxx::result &result )=0;
	virtual void on_abort( const char msg_[] )throw ();
};


/**
 * Transactor class to fetch data from an wdb database.
 * The data is returned in an TimeSeriePtr.
 */
class WciRead 
	: public pqxx::transactor<>
{
public:
	WciRead( WciReadHelper *helper );
	
	~WciRead();
	void operator () ( argument_type &t );
	void on_abort( const char msg_[] )throw (); 
	
private:
	WciReadHelper *helper;
};

}

#endif 
