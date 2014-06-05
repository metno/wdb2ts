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
#include <milib/milib.h>
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

	try {

		pqxx::work work( con, "WciPlaceSpecification");
		pqxx::result  res = work.exec( "SELECT * FROM wci.placespecification()" );
		
		ostringstream msg;
		MiProjection::GridSpec gs;
		MiProjection::ProjectionType pType;
		int nCols;
		int nRows;

		msg << "Projection (P2): ";

		for( pqxx::result::const_iterator row=res.begin(); row != res.end(); ++row )
		{
			nCols = row["inumber"].as<int>();
			nRows = row["jnumber"].as<int>();
			gs[0] = row["startlongitude"].as<float>();
			gs[1] = row["startlatitude"].as<float>();
			gs[2] = row["jincrement"].as<float>();
			gs[3] = row["iincrement"].as<float>();
			
			// The SRIDs are hardcoded into the code, instead of trying to decode
			// the PROJ string.
			switch ( row["originalsrid"].as<int>() ) {
			case 50000:
			case 50007:
				//From comments in milib shall gs allways be set to
				//this value for geographic projection.
				gs[0] = 1.0;
				gs[1] = 1.0;
				gs[2] = 1.0;
				gs[3] = 1.0;
				gs[4] = 0.0;
				gs[5] = 0.0;
				pType = MiProjection::geographic;
				break;
			case 50001: // Hirlam 10     
				gs[4] = -40.0;
				gs[5] = 68.0;
				pType = MiProjection::spherical_rotated;
				break;
			case 50002: // Hirlam 20
				gs[4] = 0.0;
				gs[5] = 65.0;
				pType = MiProjection::spherical_rotated;
				break;
			case 50003: // PROFET
				gs[4] = -24.0;
				gs[5] = 66.5;
				pType = MiProjection::spherical_rotated;
				break;
			case 50004: //Sea and wave models
				gs[0] = (0-gs[0])/gs[3] ; //poleGridX = (0 - startX) / iIncrement
				gs[1] = (0-gs[1])/gs[2];  //poleGridY = (0 - startY) / jIncrement
				gs[3] = 58;
				gs[4] = 60;
				gs[5] = 0.0;
				pType = MiProjection::polarstereographic;
				break;
			case 50005: //Sea and wave models
				gs[0] = (0-gs[0])/gs[3] ; //poleGridX = (0 - startX) / iIncrement
				gs[1] = (0-gs[1])/gs[2];  //poleGridY = (0 - startY) / jIncrement
				gs[3] = 24;
				gs[4] = 60;
				gs[5] = 0.0;
				pType = MiProjection::polarstereographic;
				break;
			default:
				pType =  MiProjection::undefined_projection;
				/*
				gs[4] = 0.0;
				gs[5] = 0.0;
				pType = MiProjection::spherical_rotated;
				*/
				break;
			}
		/*
		WEBFW_LOG_DEBUG( row.at("placename").c_str() << "i#: " << row.at("inumber").as<int>()
			     << " j#: " << row.at("jnumber").as<int>() 
			     << " iincr: " << row.at("iincrement").as<float>()
			     << " jincr: " << row.at("jincrement").as<float>()
			     << " startlon: " << row.at("startlongitude").as<float>()
			     << " startlat: " << row.at("startlatitude").as<float>()	
			     << " srid: " << row.at("originalsrid").as<int>()
			     << " proj: " << row.at("projdefinition").c_str() );
		*/

			projections.add( row.at("placename").c_str(), MiProjection( gs, pType ) );
			msg << endl << "       projection: " <<  row.at("placename").c_str() << "  " << MiProjection( gs, pType );
		}

		WEBFW_LOG_DEBUG( msg.str() );
	}
	catch( std::exception &ex ) {
		WEBFW_LOG_ERROR( "EXCEPTION: ProjectionHelper::loadFromDB: " << ex.what() );
		return false;
	}
	catch( ... ) {
		WEBFW_LOG_ERROR( "EXCEPTION: ProjectionHelper::loadFromDB: UNKNOWN reason!" );
		return false;
	}
	
	return true;
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
			transactionid = "wci.inforegulargrid";
			query = "SELECT * FROM wci.info( NULL, NULL::wci.inforegulargrid )";
		} else {
			transactionid = "wci.inforegulargrid";
			query = "SELECT * FROM wci.getPlaceRegularGrid( NULL )";
		}

		msg.str("Projection (P4):");

		pqxx::work work( con, transactionid );
		pqxx::result  res = work.exec( query );

		for( pqxx::result::const_iterator row=res.begin(); row != res.end(); ++row )
		{

			MiProjection projection = ProjectionHelper::createProjection( row["startx"].as<float>(), row["starty"].as<float>(),
														row["incrementx"].as<float>(), row["incrementy"].as<float>(),
														row["projdefinition"].c_str() );
			if( projection.getProjectionType() ==  MiProjection::undefined_projection )
				continue;

			projections.add( row.at("placename").c_str(), projection );
			msg << endl << "       projection: " << row.at("placename").c_str() << "  " << projection;
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





