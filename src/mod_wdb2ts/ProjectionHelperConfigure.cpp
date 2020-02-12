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
#include <transactor/WciRead.h>
#include <transactor/WciTransactor.h>

using namespace std;
using namespace wdb2ts;

namespace {


class WciReadProjection : public wdb2ts::WciReadHelper {
	ProjectionHelper *projections;
	const wdb2ts::config::ActionParam &params;
	int wciProtocol;
public:
	WciReadProjection(int wciProtocol_,
			const wdb2ts::config::ActionParam &params_):
				projections(nullptr),
				params(params_), wciProtocol(wciProtocol_){
	}
	~WciReadProjection() {
		delete projections;
	}


	void clear(){
		delete projections;
		projections = new ProjectionHelper(wciProtocol);
	}

	std::string id(){
		return "wci.getPlaceRegularGrid( NULL )";
	}

	std::string query(){
		if( wciProtocol > 1  && wciProtocol <= 4 ) {
			WEBFW_USE_LOGGER( "handler" );
			WEBFW_LOG_ERROR("WDB versions less than 0.9.7 is NOT supported.");
			throw std::logic_error("WDB versions less than 0.9.7 is NOT supported.");
		}else {
			return "SELECT * FROM wci.getPlaceRegularGrid( NULL )";
		}
	}
	void doRead( pqxx::result &res ) {
		WEBFW_USE_LOGGER( "handler" );
		ostringstream msg;
		msg << "Projection wciProtocol: "  << wciProtocol << ". ";

		for( pqxx::result::const_iterator row=res.begin(); row != res.end(); ++row ){
			std::string projdefinition=row["projdefinition"].c_str();
			std::string placename= row.at("placename").c_str();
			try {
				MiProjection projection = ProjectionHelper::createProjection( projdefinition );
				msg << endl << "       projection: " << placename << "  " << projection;
				projections->add( placename, projection );
			}
			catch( const std::exception &ex ) {
				WEBFW_LOG_ERROR("Projection: Not supported projection '" << projdefinition << "'.");
				continue;
			}
		}
		WEBFW_LOG_DEBUG( msg.str() );
	}

	ProjectionHelper &getProjections()const { return *projections; }
};



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
	
	try {
		cerr << "configureWdbProjection: db '" << wdbDB << "'\n";
		wdb2ts::WciConnectionPtr con = app->newWciConnection(wdbDB );
		WciReadProjection proj(wciProtocol, params);
		WciRead work(&proj);
		con->perform(work);
		projections_ = proj.getProjections();

		return true;
	}
	catch (const miutil::pgpool::DbNoConnectionException &ex) {
		WEBFW_LOG_ERROR( "EXCEPTION: configureWdbProjection: " << ex.what() );
		throw;
	}
	catch( std::exception &ex ) {
		WEBFW_LOG_ERROR( "EXCEPTION: configureWdbProjection: " << ex.what() );
	}
	catch( ... ) {
		WEBFW_LOG_ERROR( "EXCEPTION: configureWdbProjection: UNKNOWN reason.");
	}
	
	return false;
}


bool
configureAllWdbProjection( ProjectionHelper &projections_,
		                const wdb2ts::config::ActionParam &params,
		                int wciProtocol,
		                const std::list<std::string> &wdbDBs,
		                Wdb2TsApp *app ) {
	bool ok = true;
	ProjectionHelper res;
	for( auto &wdbDB : wdbDBs ) {
		ProjectionHelper tmpRes;
		if( configureWdbProjection( tmpRes, params, wciProtocol, wdbDB, app) )
			res.merge(tmpRes);
		else
			ok=false;
	}
	projections_=res;
	return ok;
}


#if 0
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
#endif


}





