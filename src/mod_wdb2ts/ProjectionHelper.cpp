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
#include <math.h>
#include <ProjectionHelper.h>
#include <splitstr.h>
#include <list>
#include <milib/milib.h>
#include <wdb2TsApp.h>
#include <DbManager.h>

namespace wdb2ts {

using namespace std;


MiProjection::
MiProjection()
	:gridType( undefined_projection )
{
}

MiProjection::
MiProjection( GridSpec gridSpec, ProjectionType pt )
{
	set( gridSpec, pt );
}

MiProjection::
MiProjection( const MiProjection &proj )
{
	for( int i = 0; i < 6; ++i )
		gridSpec[i] = proj.gridSpec[i];
	
	gridType = proj.gridType;
}

MiProjection&
MiProjection::
operator=( const MiProjection &rhs )
{
	if( this != &rhs ) {
		for( int i = 0; i < 6; ++i )
			gridSpec[i] = rhs.gridSpec[i];
		
		gridType = rhs.gridType;
	}
	
	return *this;
}

void 
MiProjection::
makeGeographic()
{
	GridSpec gs = { 1.0, 1.0, 1.0, 1.0, 0.0, 0.0 };
	
	set( gs, geographic );
}

/*
 * The initialization is a copy from the function
 * Projection::set_mi_gridspec(const int gt, const float gs[speclen]) 
 * in the file diField/diProjection.cc
 */
void 
MiProjection::
set( GridSpec gridSpec_, ProjectionType pt  )
{
	gridType = pt;
	GridSpec gridspecstd;
	/*  
	  Fix gridspec from fortran indexing (from 1) to C/C++ indexing (from 0).
	  To be used in fortran routines xyconvert and uvconvert,
	  gridspecstd must be used in fortran routine mapfield
	  and when writing a field to a field file.
	 */
	for( int i=0; i<6; i++ ) {
		gridspecstd[i]= gridSpec_[i];
		gridSpec[i] =   gridSpec_[i];
	}

	/*
	    projection type 1 : polarstereographic at 60 degrees north !!!
	    (mapareas defined in setup without setting true latitude to 60.,
	    no problem for fields when using gridpar)
	 */
	if( gridType == polarstereographic_60 ) {
		gridspecstd[4]= 60.;
		gridSpec[4]=    60.;
	}

	/*
	    call movegrid on grid specification if not undefined projection:
	    Offsets "gridspecstd" with (1,1) and output result in "gridspec"
	 */
	if( gridType != undefined_projection ) {
		int ierror= 1;
		::movegrid( gridType, gridspecstd, 1.0, 1.0, gridSpec, &ierror);
	    
		if (ierror){
	      cerr << "movegrid error : " << ierror
	           << " (in MiProjection::set)" << endl;
		}
	}
}

bool 
MiProjection::
xyconvert( const MiProjection &proj, float &latitude, float &longitude )const
{
	int error;

	if( gridType == undefined_projection || proj.gridType == undefined_projection ) {
		cerr << "MiProjection::xyconvert: undefined projection." << endl;
		return false;
	}
	/*
	 extern void xyconvert(int npos, float *x, float *y,
				int igtypa, float *ga,
				int igtypr, float *gr, int *ierror);
	 */
	::xyconvert( 1, &longitude, &latitude, 
				  static_cast<int>(proj.gridType), const_cast<float*>(proj.gridSpec), 
				  static_cast<int>(gridType), const_cast<float*>(gridSpec), &error);
	
	if( error != 0 ) {
		cerr << "MiProjection::xyconvert: Conversion error. Error code: " << error << "." << endl;
		return false;
	}
	
	return true;
}

bool 
MiProjection::
uvconvert( const MiProjection &proj, float latitude, float longitude, float &u, float &v )const
{
	int error;

	if( gridType == undefined_projection || proj.gridType == undefined_projection ) {
		cerr << "MiProjection::uvconvert: undefined projection." << endl;
		return false;
	}
	
	/*
	    extern void uvconvert(int npos, float *xr, float *yr,
			float *u, float *v,
			int igtypa, float *ga,
			int igtypr, float *gr, float udef, int *ierror);
	 */ 
	
	::uvconvert( 1, &longitude, &latitude, &u, &v,
			       static_cast<int>(proj.gridType), const_cast<float*>(proj.gridSpec),
			       static_cast<int>(gridType), const_cast<float*>(gridSpec), FLT_MAX, &error);
	
	if( error != 0 ) {
		cerr << "MiProjection::uvconvert: Conversion error. Error code: " << error << "." << endl;
		return false;
	}
		
	return true;
	
}


ProjectionHelper::
ProjectionHelper()
	: isInitialized(false), wciProtocol( 1 )
{
	geographic.makeGeographic();
}

bool 
ProjectionHelper::
loadFromDBWciProtocol_1( pqxx::connection& con, 
		                   const wdb2ts::config::ActionParam &params 
		                )
{
	map<string, list<string> > places;
	map<string, list<string> >::iterator itPlace;
	list<string>::iterator itProvider;

	if( isInitialized )
		return true;
	
	isInitialized = true;
	
	wdb2ts::config::ActionParam::const_iterator it=params.find("wdb.projection");
		
	if( it == params.end() ) {
		cerr << "WARNING: No wdb.projection specification!" << endl;
		return true;
	}
	
	vector<string> vec = miutil::splitstr(it->second.asString(), ';');
	
	if( vec.empty() ) {
		cerr << "WARNING: wdb.projection specification. No valid values!" << endl;
		return false;
	}
	
	for( vector<string>::size_type i=0; i<vec.size(); ++i ) {
		vector<string> vals = miutil::splitstr( vec[i], '=');
		
		if( vals.size() != 2 ) {
			cerr << "WARNING: wdb.projection specification. value <" << vec[i] << "> wrong format!"<< endl;
			continue;
		}
		
		places[ vals[1] ].push_back( vals[0] );
	}

	try {
		pqxx::work work( con, "WciPlaceSpecification");
		pqxx::result  res = work.exec( "SELECT * FROM wci.placespecification()" );
		
		MiProjection::GridSpec gs;
		MiProjection::ProjectionType pType;
		int nCols;
		int nRows;

		for( pqxx::result::const_iterator row=res.begin(); row != res.end(); ++row ){
			itPlace = places.find( row.at("placename").c_str() );
			
			if( itPlace == places.end() || itPlace->second.empty() )
				continue;
			
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
			case 50004:
				//TODO: This is only valid for Nordic4km?
				gs[0] = (0-gs[0])/gs[3] ; //poleGridX = (0 - startX) / iIncrement
				gs[1] = (0-gs[1])/gs[2];  //poleGridY = (0 - startY) / jIncrement
				gs[3] = 58;
				gs[4] = 60;
				gs[5] = 0.0;
				pType = MiProjection::polarstereographic;
				break;
			default:
				gs[4] = 0.0;
				gs[5] = 0.0;
				pType = MiProjection::spherical_rotated;
				break;
			}
		/*
		cerr << row.at("placename").c_str() << "i#: " << row.at("inumber").as<int>() 
			     << " j#: " << row.at("jnumber").as<int>() 
			     << " iincr: " << row.at("iincrement").as<float>()
			     << " jincr: " << row.at("jincrement").as<float>()
			     << " startlon: " << row.at("startlongitude").as<float>()
			     << " startlat: " << row.at("startlatitude").as<float>()	
			     << " srid: " << row.at("originalsrid").as<int>()
			     << " proj: " << row.at("projdefinition").c_str() << endl; 
		*/
			
			for( itProvider = itPlace->second.begin(); itProvider != itPlace->second.end(); ++itProvider ) {
				projectionMap[ *itProvider ] = MiProjection( gs, pType );
				cerr << "INFO: wdb.projection added: " << *itProvider  << ": "
				     << projectionMap[ *itProvider ]  << endl;
			}	
			
		}
	}
	catch( std::exception &ex ) {
		cerr << "EXCEPTION: ProjectionHelper::loadFromDB: " << ex.what() << endl;
		return false;
	}
	catch( ... ) {
		cerr << "EXCEPTION: ProjectionHelper::loadFromDB: UNKNOWN reason!" << endl;
		return false;
	}
	
	return true;
}

bool 
ProjectionHelper::
loadFromDBWciProtocol_2( pqxx::connection& con, 
		                   const wdb2ts::config::ActionParam &params
		                 )
{
	try {
		pqxx::work work( con, "WciPlaceSpecification");
		pqxx::result  res = work.exec( "SELECT * FROM wci.placespecification()" );
	
		placenameMap.clear();
		
		MiProjection::GridSpec gs;
		MiProjection::ProjectionType pType;
		int nCols;
		int nRows;

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
		cerr << row.at("placename").c_str() << "i#: " << row.at("inumber").as<int>() 
			     << " j#: " << row.at("jnumber").as<int>() 
			     << " iincr: " << row.at("iincrement").as<float>()
			     << " jincr: " << row.at("jincrement").as<float>()
			     << " startlon: " << row.at("startlongitude").as<float>()
			     << " startlat: " << row.at("startlatitude").as<float>()	
			     << " srid: " << row.at("originalsrid").as<int>()
			     << " proj: " << row.at("projdefinition").c_str() << endl; 
		*/

			placenameMap[row.at("placename").c_str()] = MiProjection( gs, pType );
		}
	}
	catch( std::exception &ex ) {
		cerr << "EXCEPTION: ProjectionHelper::loadFromDB: " << ex.what() << endl;
		return false;
	}
	catch( ... ) {
		cerr << "EXCEPTION: ProjectionHelper::loadFromDB: UNKNOWN reason!" << endl;
		return false;
	}
	
	return true;
}


bool 
ProjectionHelper::
loadFromDB( pqxx::connection& con, 
		      const wdb2ts::config::ActionParam &params,
		      int wciProtocol_ )
{
	boost::mutex::scoped_lock lock( mutex );
	
	wciProtocol = wciProtocol_;
	
	if( wciProtocol > 1 )
		return loadFromDBWciProtocol_2( con, params );
	
	return loadFromDBWciProtocol_1( con, params );
}


ProjectionHelper::ProjectionMap::const_iterator
ProjectionHelper::
findProjection( const std::string &provider ) 
{
	ProjectionMap::const_iterator it;

#if 0
	cerr << "ProjectionHelper::findProjection: protocol: " << wciProtocol << endl;
	cerr << "ProjectionHelper::findProjection: provider: " << provider << endl;
	for( it = projectionMap.begin(); it != projectionMap.end(); ++it ) 
		cerr << "ProjectionHelper::findProjection: " << it->first << ": " << it->second << endl;
#endif
	
	it = projectionMap.find( provider );
	
	if( it != projectionMap.end() )
		return it;
	
	if( wciProtocol == 1 )
		return projectionMap.end();
	
	
#if 0	
	for( it = placenameMap.begin(); it != placenameMap.end(); ++it ) 
		cerr << "ProjectionHelper::findProjection: placenameMap: " << it->first << ": " << it->second << endl;
#endif		
	
	//The provider may be on the form 'provider [placename]'. 
	//Search the placename map for the place name and add it to the 
	//projectionMap.
	ProviderItem pi=ProviderList::decodeItem( provider );

#if 0
	cerr << "ProjectionHelper::findProjection: placenameMap: pi.placename: " << pi.placename << endl;
	cerr << "ProjectionHelper::findProjection: placenameMap: pi.provider:  " << pi.provider << endl;
	cerr << "ProjectionHelper::findProjection: placenameMap: pi.providerWithPlacename:  " << pi.providerWithPlacename() << endl;
#endif
	
	if( pi.placename.empty() )
		return projectionMap.end();
	
	it = placenameMap.find( pi.placename );
	
	if( it == placenameMap.end() )
		return projectionMap.end();
	
	cerr << "ProjectionHelper: Added provider <" << pi.providerWithPlacename() << "> to the projection cache! " << it->second << endl; 
	
	projectionMap[ pi.providerWithPlacename() ] = it->second;
	
	return projectionMap.find( provider );
}
	



bool 
ProjectionHelper::
convertToDirectionAndLength( const std::string &provider, 
		                       float latitude, float longitude, 
								     float u, float v, 
								     float &direction, float &length, bool turn )const
{
	if( u == FLT_MAX || v == FLT_MAX )
		return false;
	
	{ //Mutex protected scope.
		boost::mutex::scoped_lock lock( const_cast<ProjectionHelper*>(this)->mutex );
	
		ProjectionMap::const_iterator it = 
				const_cast<ProjectionHelper*>(this)->findProjection( provider );
		
		if( it != projectionMap.end() ) 
		{
			if( it->second.getProjectionType() == MiProjection::undefined_projection ) {
				cerr << "ProjectionHelper::convertToDirectionAndLength: WARNING Projection definition for provider <" << provider << "> is 'undefined_projection'!" << endl;
				return false;
			}
			
			if( ! geographic.uvconvert( it->second, latitude, longitude, u, v ) ){
				cerr << "ProjectionHelper::convertToDirectionAndLength: failed!" << endl;
				return false;
			}
		
			//cerr << "ProjectionHelper::convertToDDandFF: DEBUG provider <" << provider << ">!" << endl;
		} else {
			cerr << "ProjectionHelper::convertToDirectionAndLength: WARNING no Projection definition for provider <" << provider << ">!" << endl;
		}
	} //End mutex protected scope.
	
	if( u == FLT_MAX || v == FLT_MAX ) {
		cerr << "ProjectionHelper::convertToDirectionAndLength: failed u or/and v undefined!" << endl;
		return false;
	}
		
	float deg=180./3.141592654;
	float fTurn;
	
	if( turn ) 
		fTurn = 90.0;
	else
		fTurn = 270.0;
		
	length = sqrtf(u*u + v*v);
	     
	if( length>0.0001) {
		direction = fTurn - deg*atan2( v, u );	
		if( direction > 360 ) direction -=360;
		if( direction < 0   ) direction +=360;
	} else {
		direction = 0;
	}
	
	return true;
}


bool 
configureWdbProjection( ProjectionHelper &projectionHelper,
		                  const wdb2ts::config::ActionParam &params,
		                  int wciProtocol, 
		                  const std::string &wdbDB,
		                  Wdb2TsApp *app )
{
	ProviderList providerList;
	
	try {
		miutil::pgpool::DbConnectionPtr con = app->newConnection( wdbDB );
		
		projectionHelper.loadFromDB( con->connection(), params, wciProtocol );
		return true;
	}
	catch( std::exception &ex ) {
		cerr << "EXCEPTION: configureWdbProjection: " << ex.what() << endl;
	}
	catch( ... ) {
		cerr << "EXCEPTION: configureWdbProjection: UNKNOWN reason."<< endl;
	}
	
	return false;
}




std::ostream& 
operator<<(std::ostream &o, const MiProjection &proj )
{
	string grid;
	
	switch( proj.gridType ) {
	case MiProjection::undefined_projection:  grid="undefined_projection"; break;
	case MiProjection::polarstereographic_60: grid="polarstereographic_60"; break;
	case MiProjection::geographic:            grid="geographic"; break;
	case MiProjection::spherical_rotated:     grid="spherical_rotated"; break;
	case MiProjection::polarstereographic:    grid="polarstereographic"; break;
	case MiProjection::mercator:              grid="mercator"; break;
	case MiProjection::lambert:               grid="lambert"; break;
	default:
		grid="UNKNOWN grid type"; break;
	}
	
	o << "MiProjection[" << grid << "," 
	  << proj.gridSpec[0] << "," 
	  << proj.gridSpec[1] << "," 
	  << proj.gridSpec[2] << "," 
	  << proj.gridSpec[3] << "," 
	  << proj.gridSpec[4] << ","
	  << proj.gridSpec[5] << "]";
	return o;
}

}





