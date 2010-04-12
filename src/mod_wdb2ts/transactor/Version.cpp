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

#include <boost/date_time/posix_time/posix_time.hpp>
#include <wdb2tsProfiling.h>
#include <transactor/Version.h>
#include <Logger4cpp.h>

DECLARE_MI_PROFILE;

using namespace std;

namespace wdb2ts {

Version::
Version( )
	: version( new std::string() )
{
}

Version::
~Version()
{
}

void 
Version::
operator () ( argument_type &t )
{
	version->erase();

	string q("SELECT * FROM wci.version()");

	WEBFW_USE_LOGGER( "wdb" );

	WEBFW_LOG_DEBUG( "WCI version (Transactor): query: " << q );
	
	USE_MI_PROFILE;
	MARK_ID_MI_PROFILE("wci.version (Transactor)");
	
	try {
		pqxx::result  res = t.exec( q );
	      
		if( res.size() == 0 )
			return;
	        
		for( pqxx::result::const_iterator it=res.begin(); it != res.end(); ++it  ) { 
			if( ! it.at("version").is_null() ) {
				(*version) = it.at("version").c_str();
				break;
			}
		}
	}
	catch( ... ) {
		MARK_ID_MI_PROFILE("wci.version (Transactor)");
		throw;
	}
	
	MARK_ID_MI_PROFILE("wci.version (Transactor)");
}

}

