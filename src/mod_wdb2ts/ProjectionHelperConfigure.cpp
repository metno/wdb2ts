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


#include <float.h>
//#include <math.h>
#include <cmath>
#include <ProjectionHelperConfigure.h>
#include <boost/thread.hpp>
#include <splitstr.h>
#include <list>
#include <algorithm>
//#include <milib/milib.h>
#include <wdb2TsApp.h>
#include <DbManager.h>
#include <Logger4cpp.h>
#include <boost/regex.hpp>
#include <sstream>

using namespace std;
using namespace wdb2ts;

namespace {


bool 
loadFromDBWciProtocol_2( pqxx::connection& con, 
		                   const wdb2ts::config::ActionParam &params,
		                   ProjectionHelper &projections
		                 )
{
	WEBFW_USE_LOGGER( "handler" );
	WEBFW_LOG_ERROR("WDB versions less than 0.9.7 is NOT supported.");
	return false;
}


bool
loadFromDBWciProtocol_4( pqxx::connection& con,
		                 const wdb2ts::config::ActionParam &params,
		                 int protocol,
		                 ProjectionHelper &projections )
{
	using namespace boost;

	WEBFW_USE_LOGGER( "handler" );
	ostringstream msg;

	try {
		WEBFW_LOG_DEBUG( "ProjectionHelper::P4: SELECT * FROM wci.info( NULL, NULL::wci.inforegulargrid )" );
		string transactionid;
		string query;

		if( protocol == 4 ) {
			WEBFW_LOG_ERROR("WDB versions less than 0.9.7 is NOT supported.");
			return false;
		} else {
			transactionid = "wci.inforegulargrid";
			query = "SELECT * FROM wci.getPlaceRegularGrid( NULL )";
		}

		msg.str("Projection (P4):");

		pqxx::work work( con, transactionid );
		pqxx::result  res = work.exec( query );

		for( pqxx::result::const_iterator row=res.begin(); row != res.end(); ++row )
		{
			try {
				MiProjection projection = ProjectionHelper::createProjection( row["projdefinition"].c_str() );
				msg << endl << "       projection: " << row.at("placename").c_str() << "  " << projection;
				projections.add( row.at("placename").c_str(), projection );
			}
			catch( const std::exception &ex ) {
				WEBFW_LOG_ERROR("Projection: Not supported projection '" << row["projdefinition"].c_str() << "'.");
				continue;
			}
		}

		WEBFW_LOG_DEBUG( msg.str() );
	}
	catch( std::exception &ex ) {
		WEBFW_LOG_ERROR( "EXCEPTION: ProjectionHelper::loadFromDB (protocol 4): " << ex.what() );
		return false;
	}
	catch( ... ) {
		WEBFW_LOG_ERROR( "EXCEPTION: ProjectionHelper::loadFromDB (protocol 4): UNKNOWN reason!" );
		return false;
	}

	return true;
}

/**
 * Load the projection data in projectionMap from wdb.
 * Which projection that is associated with each providername is defined
 * by the actionparam wdb.projection in the configuration file.
 * The value of wdb.projection is a ';' separated list of providername=placename.
 *
 * Ex.
 * 	value="probability_forecast=ec ensemble;proff=proff;hirlam 10=hirlam 10"
 */
bool 
loadFromDB( pqxx::connection& con, 
		      const wdb2ts::config::ActionParam &params,
		      int wciProtocol,
		      ProjectionHelper &projections )
{
	if( wciProtocol > 1  && wciProtocol <= 3 )
		return loadFromDBWciProtocol_2( con, params, projections );
	
	if( wciProtocol == 4 )
		return loadFromDBWciProtocol_4( con, params, 4, projections  );

	if( wciProtocol >= 5 )
		return loadFromDBWciProtocol_4( con, params, 5, projections  );


	return loadFromDBWciProtocol_4( con, params, 5, projections );
}


}


namespace wdb2ts {





bool
configureWdbProjection( ProjectionHelper &projections_,
		                const wdb2ts::config::ActionParam &params,
		                int wciProtocol,
		                const std::string &wdbDB,
		                Wdb2TsApp *app )
{
	WEBFW_USE_LOGGER( "handler" );
	ProjectionHelper projections( wciProtocol );
	
	try {
		miutil::pgpool::DbConnectionPtr con = app->newConnection( wdbDB );
		
		loadFromDB( con->connection(), params, wciProtocol, projections );
		projections_ = projections;
		return true;
	}
	catch( std::exception &ex ) {
		WEBFW_LOG_ERROR( "EXCEPTION: configureWdbProjection: " << ex.what() );
	}
	catch( ... ) {
		WEBFW_LOG_ERROR( "EXCEPTION: configureWdbProjection: UNKNOWN reason.");
	}
	
	return false;
}



}





