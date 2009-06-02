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


#ifndef __WCI_TRANSACTOR_H__
#define __WCI_TRANSACTOR_H__


#include <iostream>
#include <boost/shared_ptr.hpp>
#include <pqxx/transactor>
#include <string>

namespace wdb2ts {

/**
 * WciTransactor is a helper class. It is used to detect
 * missing call to wci.begin. If it detect that wci.begin
 * has not been run on the connection the wci.begin is run 
 * and the transaction is run again.
 * 
 * example.
 * 
 * <pre>
   pqxx::connection con( ..... );
   WciRead wciRead; //This is a normal pqxx transactor object.
  
   con.perform( WciTransaction<WciRead>( wciRead, wciuser) );
   </pre>
 */
template<typename Transactor >
class WciTransactor : public pqxx::transactor<typename Transactor::argument_type>
{
	Transactor transactor;
	
public:
	WciTransactor( const Transactor &t,  std::string &user ) 
		: transactor( t ), user( user ), doWciBegin( new bool(false) ) {	}

	WciTransactor( const Transactor &t,  const char *user ) 
			: transactor( t ), user( user ), doWciBegin( new bool(false) ) {	}

	~WciTransactor() {};
	
	void operator () (typename Transactor::argument_type & t)
	{
		
		if( *doWciBegin ) {
			std::cerr << "WciTransactor: SELECT wci.begin('" << user <<"')" << std::endl;
			*doWciBegin = false;
			t.exec( "SELECT wci.begin('" + user + "')" );
		}
		
		transactor( t );
	}

	void on_abort( const char msg_[] )throw () 
	{
		std::string msg( msg_ );
		
		if( msg.find("wci has not been initialized.") != std::string::npos ) {
			*doWciBegin = true;
		} else {
			transactor.on_abort( msg_ );
		}
	}
	
	void on_commit() 
	{
		transactor.on_commit();
	}		

	void on_doubt() throw () 
	{
		transactor.on_doubt();
	}
		
private:
	const std::string user;
	boost::shared_ptr<bool> doWciBegin;
};

}


#endif 
