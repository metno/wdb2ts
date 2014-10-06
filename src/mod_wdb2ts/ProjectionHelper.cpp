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
#include <ProjectionHelper.h>
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

namespace {

struct uv {
  double u;
  double v;

  uv( double u_, double v_) :
    u(u_), v(v_)
  {
  }

  explicit uv(double angle) :
    u(std::sin(angle * DEG_TO_RAD)), v(std::cos(angle * DEG_TO_RAD))
  {
  }

  double angle() const
  {
    if (0 == u and 0 == v) {
      return HUGE_VAL;
    }

    if (0 == v) {
      if (0 < u)
        return 90;
      else
        return 270;
    }

    double ret = std::atan(u / v) * RAD_TO_DEG;
    if (v < 0)
      ret += 180;

    if (ret < 0)
      ret += 360;
    if (360 < ret)
      ret -= 360;

    return ret;
  }
};


float
getProjFloat( const std::string &projdef, const boost::regex &what )
{
	using namespace boost;
	smatch match;
	float val;

	if( ! regex_search( projdef, match, what ) )
		return FLT_MAX;

	std::string sval = match[1];

	if( sscanf( sval.c_str(), " %f", &val) != 1 )
		return FLT_MAX;

	return val;
}

double
north( const wdb2ts::MiProjection &p,
       double x, double y )
{
	double north_x=x;
	double north_y=y;

	p.convertFromXYToGeographic( north_y, north_x );
	//north_y = std::min<double>(north_y + 0.1, 90);
	north_y = std::min<double>(north_y + 0.1, 89.999999);
	p.convertFromGeographicToXY( north_y, north_x );
	return uv(north_x - x, north_y - y).angle();
}

double*
north( const wdb2ts::MiProjection &p,
       int nvec, const double *x, const double *y)
{
	double* north_x = new double[nvec];
	double* north_y = new double[nvec];
	std::copy(x, x + nvec, north_x);
	std::copy(y, y + nvec, north_y);

	p.convertFromXYToGeographic( nvec, north_y, north_x );

	for (int i = 0; i < nvec; i++){
		//north_y[i] = std::min<double>(north_y[i] + 0.1, 90);
		north_y[i] = std::min<double>(north_y[i] + 0.1, 89.999999);
	}

	p.convertFromGeographicToXY( nvec, north_y, north_x );

	double *ret = new double[nvec];

	for (int i = 0; i < nvec; i++){
		ret[i] = uv(north_x[i] - x[i], north_y[i] - y[i]).angle();
	}
	delete[] north_x;
	delete[] north_y;
	return ret;
}


double turn( double angle_a, double angle_b)
{
	double angle = angle_a + angle_b;
	if (angle >= 360)
		angle -= 360;
	if (angle < 0)
		angle += 360;
	return angle;
}

}

namespace wdb2ts {




MiProjection::
MiProjection()
	: proj( 0 ), gridType( undefined_projection ), geoproj( 0 )
{
	init();
}

MiProjection::
MiProjection( const std::string &projDef )
	: projString( projDef ), gridType( undefined_projection ), geoproj( 0 )
{
	proj = pj_init_plus( projDef.c_str() );
	init();
}


MiProjection::
MiProjection( GridSpec gridSpec, ProjectionType pt, const char *projS )
	: proj( 0 ), geoproj( 0 )

{
	if( projS ) {
		projString = projS;
		proj = pj_init_plus( projString.c_str() );

		if( ! proj ){
			cerr << "FILED to inotialize proj: " << projString << endl;
		}
	}

	set( gridSpec, pt );
	init();
}

MiProjection::
MiProjection( const MiProjection &miProj )
	: projString( miProj.projString ), proj( 0 ), geoproj( 0 )
{
	for( int i = 0; i < 6; ++i )
		gridSpec[i] = miProj.gridSpec[i];

	gridType = miProj.gridType;

	if( miProj.proj ) {
		projString = miProj.projString;
		proj = pj_init_plus( projString.c_str() );
	}
	init();
}

MiProjection::
~MiProjection()
{
	if( proj )
		pj_free( proj );

	if( geoproj )
		pj_free( geoproj );
}

MiProjection&
MiProjection::
operator=( const MiProjection &rhs )
{
	if( this != &rhs ) {
		for( int i = 0; i < 6; ++i )
			gridSpec[i] = rhs.gridSpec[i];

		gridType = rhs.gridType;

		if( proj ) {
			pj_free( proj );
			proj = 0;
		}

		if( rhs.proj ) {
			projString = rhs.projString;
			proj = pj_init_plus( projString.c_str() );
		}
	}

	return *this;
}

bool
MiProjection::
calculateVectorRotationElements(const MiProjection& srcProj,
    int nvec, const double *to_x, const double *to_y, double *cosa,
    double *sina) const
{
	double *from_x = new double[nvec];
	double *from_y = new double[nvec];

	std::copy(to_x, to_x + nvec, from_x);
	std::copy(to_y, to_y + nvec, from_y);

	if(! srcProj.transform( *this, nvec, from_y, from_x ) ) // convert back to old projection
		return false;

	double *from_north = north(srcProj, nvec, from_x, from_y); // degrees
	double *to_north = north(*this, nvec, to_x, to_y); // degrees

	for (int i = 0; i < nvec; ++i) {
		// the difference between angles in the two projections:
		if (from_north[i] == HUGE_VAL || to_north[i] == HUGE_VAL) {
			cosa[i] = HUGE_VAL;
			sina[i] = HUGE_VAL;
		} else {
			double angle_diff = from_north[i] - to_north[i];
			// return cos() and sin() of this angle
			cosa[i] = std::cos(angle_diff * DEG_TO_RAD);
			sina[i] = std::sin(angle_diff * DEG_TO_RAD);
		}
	}
	delete[] from_north;
	delete[] to_north;
	delete[] from_x;
	delete[] from_y;

	return true;
}

bool
MiProjection::
calculateVectorRotationElements(const MiProjection& srcProj,
								const double to_x, const double to_y,
								double &cosa, double &sina) const
{
	double from_x=to_x;
	double from_y=to_y;


	if(! srcProj.transform( *this, 1, &from_y, &from_x ) ) // convert back to old projection
		return false;

	double from_north = north(srcProj, from_x, from_y); // degrees
	double to_north = north(*this, to_x, to_y); // degrees

		// the difference between angles in the two projections:
	if (from_north == HUGE_VAL || to_north == HUGE_VAL) {
		cosa = HUGE_VAL;
		sina = HUGE_VAL;
		return false;
	} else {
		double angle_diff = from_north - to_north;
		// return cos() and sin() of this angle
		cosa = std::cos(angle_diff * DEG_TO_RAD);
		sina = std::sin(angle_diff * DEG_TO_RAD);
	}

	return true;
}


bool
MiProjection::
getVectorRotationElements(	const MiProjection &data_area,
		                    const MiProjection &map_area,
		                    const int nvec, const double *x, const double *y,
		                    double ** cosx,	double ** sinx) const
{
	*cosx = new double[nvec];
    *sinx = new double[nvec];

    if( ! map_area.calculateVectorRotationElements( data_area,
    		nvec, x, y,  *cosx, *cosx ) ) {
    	delete[] *cosx;
    	delete[] *sinx;
    	return false;
    }
  return true;
}


bool
MiProjection::
getVectorRotationElement( const MiProjection &data_area,
		                  const MiProjection &map_area,
		                  double x, double y,
		                  double &cosx,	double &sinx) const
{

    if( ! map_area.calculateVectorRotationElements( data_area, x, y, cosx,cosx ) )
    	return false;
    return true;
}

// convert u,v vector coordinates for points x,y
bool
MiProjection::
getVectors(const MiProjection& data_area, const MiProjection& map_area,
			int nvec, const double *x, const double *y, double *u, double *v )const
{

  //const Projection pr = map_area;

 // if ( map_area.P().useRotationElements()) {
  double undef = +1.e+35;

  double *cosx = 0;
  double *sinx = 0;

  if( ! getVectorRotationElements(data_area, map_area, nvec, x, y, &cosx, &sinx) ) {
      return false;
  }

  for (int i = 0; i < nvec; ++i) {
	  if (u[i] != undef && v[i] != undef) {
		  double cu = u[i];
		  if (cosx[i] == HUGE_VAL || sinx[i] == HUGE_VAL) {
			  u[i] = undef;
			  v[i] = undef;
		  } else {
			  u[i] = cosx[i] * cu - sinx[i] * v[i];
			  v[i] = sinx[i] * cu + cosx[i] * v[i];
		  }
	  }
  }

  delete[] cosx;
  delete[] sinx;

  //    ierror = pr.convertVectors(data_area.P(), nvec, x, y, u, v);

  return true;
}


bool
MiProjection::
getVector(const MiProjection& data_area, const MiProjection& map_area,
		  double x, double y, double &u, double &v )const
{
  double undef = +1.e+35;

  double cosx;
  double sinx;

  if (!getVectorRotationElement(data_area, map_area, x, y, cosx, sinx)) {
      return false;
  }


  if (u != undef && v != undef) {
	  double cu = u;
	  if (cosx == HUGE_VAL || sinx == HUGE_VAL) {
		  u = undef;
		  v = undef;
		  return false;
	  } else {
		  u = cosx * cu - sinx * v;
		  v = sinx * cu + cosx * v;
	  }
  }

  return true;
}

bool
MiProjection::
getPoints(const MiProjection& srcProj, const MiProjection& dstProj,
          int npos, double *x, double *y ) const
{
  return  dstProj.transform( srcProj, npos, y, x );
}


bool
MiProjection::
uv2geo(const MiProjection& uvProj, int nvec, const double *longX_, const double *latY_, double *u,
    double *v)const
{

  // geographic projection - entire planet...
  MiProjection geoarea;
  geoarea.makeGeographic();

  //Make a copy of the input parameters and convert them to radians
  double *longX = new double[nvec];
  double *latY = new double[nvec];

  std::copy( longX_, longX_ + nvec, longX) ;
  std::copy( latY_, latY_ + nvec, latY );

  for (int i = 0; i < nvec; i++) {
	  latY[i] *= DEG_TO_RAD;
      longX[i] *= DEG_TO_RAD;
  }

  // transform all model points to geographical grid

  bool ret = false;


  if ( getVectors( uvProj, geoarea, nvec, longX, latY, u, v)) // convertVectors
      ret = true;

  delete[] longX;
  delete[] latY;

  return ret;
}



bool
MiProjection::
xyv2geo(const MiProjection& area, int nvec, const double *x_in, const double *y_in, double *u,
    double *v)const
{

  // geographic projection - entire planet...
  MiProjection geoarea;
  geoarea.makeGeographic();

  // create entire grid for the model
  double *x = new double[nvec];
  double *y = new double[nvec];

  for (int i = 0; i < nvec; i++) {
	  x[i] = x_in[i];
      y[i] = y_in[i];
  }

  // transform all model points to geographical grid

  bool ret = false;

  if (getPoints( area, geoarea, nvec, x, y))
    if ( getVectors(area, geoarea, nvec, x, y, u, v)) // convertVectors
      ret = true;

  delete[] x;
  delete[] y;

  return ret;
}


bool
MiProjection::
directionAndLength(int nvec, const double *longitude, const double  *latitude,
          double *u, double *v, const MiProjection &area,
          double *dd, double *length ) const
{

	//cerr << "--- IN2: u: " << *u << " v: " << *v << endl;
  // transform u,v to a geographic grid
  if( ! uv2geo( area, nvec, longitude, latitude, u, v ) ) {
	  return false;
  }

  //cerr << "--- CONVERTED2: u: " << *u << " v: " << *v << endl;

  double deg = 180. / 3.141592654;
  //double ff;

  for (int i = 0; i < nvec; i++) {
	  length[i] = sqrt(u[i] * u[i] + v[i] * v[i]);

      if (length[i] > 0.0001) {
    	  dd[i] = 270. - deg * atan2(v[i], u[i]);

    	  if (dd[i] > 360)
    		  dd[i] -= 360;
    	  if (dd[i] < 0)
    		  dd[i] += 360;
      } else
    	  dd[i] = 0;
  }

  return true;
}


std::string
MiProjection::
createProjDefinition( bool addDatum )

{
	const double earthRadius = 6371000;
	double gridResolutionX=1;
	double gridResolutionY=1;
	ostringstream ost;
	string projDefinition;
	std::string str_earth;

  if ( addDatum ) {
    std::ostringstream ellipsoid;
    ellipsoid << " +ellps=WGS84 +towgs84=0,0,0 +no_defs";
    str_earth = ellipsoid.str();
  } else {
    str_earth = " +R=6371000";
  }

  MiProjection tmpProj;
  double x;
  double y;
  int npos;

  switch (gridType ) {
  case polarstereographic_60:
  case polarstereographic:
    ost << "+proj=stere";
    /*
     +proj=stere +lat_ts=Latitude at natural origin
     +lat_0=90
     +lon_0=Longitude at natural origin
     +k_0=Scale factor at natural origin (normally 1.0)
     +x_0=False Easting
     +y_0=False Northing
     */
    /*
     gridspec in mi
     g[0] - x-position of pole
     g[1] - y-position of pole
     g[2] - number of grid distances between pole and equator
     g[3] - rotation angle of the grid (degrees)
     g[4] - projection latitude (degrees) (60 degrees north for gridtype=1)
     g[5] - 0. (not used)
     */

    // Latitude of true scale
    ost << " +lat_ts=" << gridSpec[4];
    // Latitude of origin
    ost << " +lat_0=" << (gridSpec[4] > 0 ? "90.0" : "-90.0");
    // Longitude of origin
    ost << " +lon_0=" << gridSpec[3];
    // Scale factor at origin
    //ost << " +k_0=" <<  (1.0 + sin(gridspec[4]*DEG_TO_RAD) + 1.0)/2.0;

    // Calculate grid resolution
    if (gridSpec[2] > 0.0) {
      /*gridResolutionX = 150000.0 * 79.0 / static_cast<double> (gridspec[2]);*/
      gridResolutionX = (earthRadius * (1.0 + sin(fabs(gridSpec[4]) * DEG_TO_RAD))) / static_cast<double> (gridSpec[2]);
      gridResolutionY = gridResolutionX;
    } else {
      gridResolutionX = 1.0;
      gridResolutionY = 1.0;
    }
    // False Easting
    ost << " +x_0=" << (gridResolutionX * (gridSpec[0]-1));
    // False Northing
    ost << " +y_0=" << (gridResolutionY * (gridSpec[1]-1));
    ost << str_earth;
    projDefinition = ost.str();
    break;
  case geographic: // Geographic
    /*
     g(1) - western boundary (degrees)
     g(2) - southern boundary (degrees)
     g(3) - longitude increment (degrees)
     g(4) - latitude increment (degrees)
     g(5) - 0
     g(6) - 0
     */
    if ((gridSpec[0] == 1.0) && (gridSpec[1] == 1.0) && (gridSpec[2] == 1.0)
        && (gridSpec[3] == 1.0)) {
      ost << "+proj=latlong";
      // Calculate grid resolution
      gridResolutionX = DEG_TO_RAD;
      gridResolutionY = DEG_TO_RAD;
    } else {
      //The latitude_longitude projection ignores x_0 and y_0,
      //the following projection is equal to +proj=latlong but does not ignore x_0 and y_0
      ost << "+proj=ob_tran +o_proj=longlat +lon_0=0 +o_lat_p=90";
      // Calculate grid resolution
      gridResolutionX = gridSpec[2] * DEG_TO_RAD;
      gridResolutionY = gridSpec[3] * DEG_TO_RAD;
      // False Easting
      ost << " +x_0=" << (gridSpec[0] * DEG_TO_RAD * -1.);
      // False Northing
      ost << " +y_0=" << (gridSpec[1] * DEG_TO_RAD * -1.);
    }
    ost << str_earth;
    projDefinition = ost.str();
    break;
  case spherical_rotated:
    /*
     gridspec in mi
     g[0] - western boundary (degrees)
     g[1] - southern boundary (degrees)
     g[2] - longitude increment (degrees)
     g[3] - latitude increment (degrees)
     g[4] - xcen: longitude position of rotated equator (degrees)
     g[5] - ycen: latitude  position of rotated equator (degrees)
     (lamda,theta)=(xcen,ycen) at (lamda',theta')=(0,0),
     where (lamda,theta) are usual spherical coord. and
     (lamda',theta') are rotated spherical coord.
     xcen = ycen = 0 for usual spherical coord.
     */
    // ob_tran: General Oblique Transformation
    ost << "+proj=ob_tran +o_proj=longlat";
    // Longitude Position of Rotated Pole
    ost << " +lon_0=" << gridSpec[4];
    // Latitude Position of Rotated Pole
    ost << " +o_lat_p=" << (90.0 - gridSpec[5]);
    // Calculate grid resolution
    gridResolutionX = gridSpec[2] * DEG_TO_RAD ;
    gridResolutionY = gridSpec[3] * DEG_TO_RAD ;
    // False Easting
    ost << " +x_0=" << (gridSpec[0] * DEG_TO_RAD  * -1.);
    // False Northing
    ost << " +y_0=" << (gridSpec[1] * DEG_TO_RAD  * -1.);
    ost << str_earth;
    projDefinition = ost.str();
    break;
  case mercator:
    /*
     g(0) - western boundary (longitude for x=1) (degrees)
     g(1) - southern boundary (latitude for y=1) (degrees)
     g(2) - x (longitude) increment (km)
     g(3) - y (latitude)  increment (km)
     g(4) - reference (construction) latitude (degrees)
     g(5) - 0.  (not used)
     */
    ost << "+proj=merc";
    // Latitude of True Scale
    ost << " +lat_ts=" << gridSpec[4];
    // Calculate grid resolution
    gridResolutionX = 1000.0 * gridSpec[2];
    gridResolutionY = 1000.0 * gridSpec[3];
    // Spherical Earth
    ost << str_earth;
    tmpProj = MiProjection( ost.str() );
    x = gridSpec[0];
    y = gridSpec[1];
    npos = 1;
    tmpProj.convertFromGeographicToXY( y, x );
    // False Easting
    ost << " +x_0=" << -x;
    // False Northing
    ost << " +y_0=" << -y;
    projDefinition = ost.str();
    break;
  case lambert:
    /*
     g(0) - west (longitude for x=1,y=1) (degrees)
     g(1) - south (latitude for x=1,y=1) (degrees)
     g(2) - x (easting) increment (km)
     g(3) - y (northing) increment (km)
     g(4) - reference longitude (degrees)
     g(5) - reference latitude (cone tangent) (degrees)
     */
    ost << "+proj=lcc"; // Lambert Conformal Conic Alternative
    // Latitude of origin
    ost << " +lat_0=" << gridSpec[5];
    ost << " +lat_1=" << gridSpec[5];
    ost << " +lat_2=" << gridSpec[5];
    // Longitude of origin
    ost << " +lon_0=" << gridSpec[4];
    // Calculate grid resolution
    gridResolutionX = 1000.0 * gridSpec[2];
    gridResolutionY = 1000.0 * gridSpec[3];
    // Spherical Earth
    ost << str_earth;
    // trick to reveal the false northern/eastern
    tmpProj = MiProjection( ost.str() );
    x = gridSpec[0];
    y = gridSpec[1];
    npos = 1;
    tmpProj.convertFromGeographicToXY( y, x);
    // False Easting
    ost << " +x_0=" << -x;
    // False Northing
    ost << " +y_0=" << -y;
    projDefinition = ost.str();
    break;
  default:
    projDefinition = "";
  }
  if (gridResolutionX == 0.0)
    gridResolutionX = 1.0;
  if (gridResolutionY == 0.0)
    gridResolutionY = 1.0;

//  if (gridType!=undefined_projection && gridType!=geographic) {
//    cerr << "GridDef: " << gridType << " - " << gridSpec[0] << ","
//        << gridSpec[1] << "," << gridSpec[2] << "," << gridSpec[3] << ","
//        << gridSpec[4] << "," << gridSpec[5] << "; ProjDef: " << projDefinition
//        << "; GridRes: " << gridResolutionX << "  -  " << gridResolutionY << endl;
//  }

  return projDefinition;

}


void
MiProjection::
init()
{
	geoproj = pj_init_plus( "+proj=lonlat +ellps=WGS84 +towgs84=0,0,0 +no_defs") ;
}


void 
MiProjection::
makeGeographic()
{
	GridSpec gs = { 1.0, 1.0, 1.0, 1.0, 0.0, 0.0 };

	set( gs, geographic );

	if( proj ) {
		pj_free( proj );
		proj = 0;
	}

	//projString="+proj=lonlat +ellps=WGS84 +towgs84=0,0,0 +no_defs";
	projString="+proj=lonlat +ellps=WGS84 +no_defs";
	proj = pj_init_plus( projString.c_str() );

	if( ! proj ) {
		cerr << "FAILED: MiProjection::makeGeographic: can't create geographic projection.\n";
		projString.erase();
	}
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
			WEBFW_USE_LOGGER( "handler" );
	      	WEBFW_LOG_ERROR( "movegrid error : " << ierror << " (in MiProjection::set)" );
		}
	}
}

bool
MiProjection::
convertFromGeographicToXY( int nvec,  double *latitudeToY, double *longitudeToX )const
{
	if( ! proj || ! geoproj ) {
		cerr << "ERROR: convertFromGeographicToXY: No proj or geoproj definition.\n";
		return false;
	}

	for( int i=0; i<nvec; ++i ) {
//		if( latitudeToY[i] == 90 ) {
//			latitudeToY[i] = 89.9999;
//			longitudeToX[i]=0;
//		} else if( latitudeToY[i] == -90 ) {
//			latitudeToY[i] = -89.9999;
//			longitudeToX[i]=0;
//		}
//
//		if( longitudeToX[i] == 180 )
//			longitudeToX[i] = 179.9999;
//		else if( longitudeToX[i] == -180 )
//			longitudeToX[i] = -179.9999;

		longitudeToX[i] *= DEG_TO_RAD;
		latitudeToY[i] *= DEG_TO_RAD;
	}

	if( pj_transform( geoproj, proj, nvec, 1, longitudeToX, latitudeToY, 0) != 0 )
		return false;

	return true;
}

bool
MiProjection::
convertFromGeographicToXY( double &latitudeToY, double &longitudeToX )const
{
	bool ret = convertFromGeographicToXY( 1, &latitudeToY, &longitudeToX );

	return ret && latitudeToY != HUGE_VAL && longitudeToX != HUGE_VAL;
}

bool
MiProjection::
convertFromXYToGeographic( double &yToLatitude, double &xToLongitude )const
{
	bool ret = convertFromXYToGeographic( 1, &yToLatitude, &xToLongitude );

	return ret && yToLatitude != HUGE_VAL && xToLongitude != HUGE_VAL;
}



bool
MiProjection::
convertFromXYToGeographic( int nvec, double *yToLatitude, double *xToLongitude )const
{
	if( ! proj || ! geoproj ) {
		//cerr << "ERROR: convertFromXYToGeographic: No proj or geoproj definition.\n";
		return false;
	}

	if( pj_transform( proj, geoproj, nvec, 1, xToLongitude, yToLatitude, 0) != 0 )
		return false;

	for( int i=0; i<nvec; ++i ) {
		xToLongitude[i] *= RAD_TO_DEG;
		yToLatitude[i] *= RAD_TO_DEG;
	}

	return true;
}


bool
MiProjection::
convertVectors(const MiProjection& srcProj, int nvec,
               double * to_x,  double * to_y, double * u, double * v) const
{
  double udef= HUGE_VAL;
  bool ret=true;

  //Check if this is a geo projection
  if( pj_is_latlong(proj) ) {
	  //We define that the north pole is at lat 90 and long 0
	  //so if to_y is equal to 90 we set to_x t0 0.
	  const double lat90 = 90 * DEG_TO_RAD;
	  for( int i = 0; i < nvec; ++i ) {
		  if( fabs(to_y[i]) == lat90 )
			  to_x[i] = 0;

	  }
  }

  double * from_x = new double[nvec];
  double * from_y = new double[nvec];

  std::copy(to_x, to_x + nvec, from_x);
  std::copy(to_y, to_y + nvec, from_y);

  // convert the position x/y (longitude/latitude) back to old projection
  srcProj.transform( *this, nvec, from_x, from_y);

  double * from_north = north( srcProj, nvec, from_x, from_y ); // degrees
  double * to_north = north( *this, nvec, to_x, to_y ); // degrees

  for (int i = 0; i < nvec; ++i) {
	  if(u[i] != udef && v[i] != udef ) {
		  const float length = std::sqrt(u[i]*u[i] + v[i]*v[i]);

		  // the difference between angles in the two projections:
		  double angle_diff = to_north[i] - from_north[i];

		  double new_direction = turn(uv(u[i], v[i]).angle(), angle_diff);
			  // float new_direction = to_north[ i ]; // This makes all directions be north.
		  uv convert(new_direction);
		  u[i] = convert.u * length;
		  v[i] = convert.v * length;
	  } else {
		  ret = false;
	  }
  }

  delete[] from_north;
  delete[] to_north;
  delete[] from_x;
  delete[] from_y;

  return ret;
}

bool
MiProjection::
convertVectors( const MiProjection& srcProj,
		        double to_x, double to_y,
				double &u, double &v) const
{
	double udef= HUGE_VAL;
	bool ret=true;

	if( pj_is_latlong( proj ) ) {
		//We define that the north pole is at lat 90 and long 0
		//so if to_y is equal to 90 we set to_x to 0 and we do
		//the same for the south pole ie lat is -90.
		const double lat90 = 90 * DEG_TO_RAD;
		if( fabs( to_y ) == lat90 )
			to_x = 0;
	}

	double from_x = to_x;
	double from_y = to_y;

	// convert the position x/y (longitude/latitude) to srcProj
	if( ! srcProj.transform( *this, 1, &from_y, &from_x) )
		return false;

	double from_north = north( srcProj, from_x, from_y ); // degrees
	double to_north = north( *this, to_x, to_y ); // degrees

	if(u != udef && v != udef ) {
		const float length = std::sqrt( u*u + v*v );

		// the difference between angles in the two projections:
		double angle_diff = to_north - from_north;

		double new_direction = turn(uv(u, v).angle(), angle_diff);
		// float new_direction = to_north[ i ]; // This makes all directions be north.
		uv convert(new_direction);
		u = convert.u * length;
		v = convert.v * length;
	} else {
		ret = false;
	}

	return ret;
}


bool
MiProjection::
transform( const MiProjection &srcProj, int nvec, double *y, double *x )const
{
	int error;
	if( ! srcProj.proj || ! proj ) {
		cerr << "ERROR: transform: No proj definition.\n";
		return false;
	}

	if( (error=pj_transform( srcProj.proj, proj, nvec, 1, x, y, NULL)) != 0 ) {
		if( error == -14 )
			cerr << "WARNING: transform: pj_transform. error: (" << error <<") " << pj_strerrno( error ) << endl;
		else
			cerr << "ERROR: transform: pj_transform. error: (" << error <<") " << pj_strerrno( error ) << endl;
		for( int i=0; i<nvec; ++i )
			cerr << "   x: " << x[i] << " y: " << y[i] << endl;
		cerr << "srcProj def: '" << pj_get_def( srcProj.proj, 0 ) << "'\n";
		cerr << "dstProj def: '" << pj_get_def( proj, 0 ) << "'\n";
		return error==-14; //We accept -14 here and leaves it to the caller to check that the values is NOT HUGE_VAL.
	}

	return true;
}




bool
MiProjection::
xyconvert( const MiProjection &miProj, float &y_, float &x_ )const
{
	int error;
	WEBFW_USE_LOGGER( "handler" );

//	if( miProj.proj && proj ) {
//		//x - longitude, y-latitude
//		double x, y;
//		x = x_;
//		y = y_;
//
//		if( pj_transform(miProj.proj, proj, 1, 1, &x, &y, NULL) != 0 )
//			return false;
//
//		if( x == HUGE_VAL || y == HUGE_VAL )
//			return false;
//
//		x_ = x;
//		y_ = y;
//		return true;
//	}


	if( gridType == undefined_projection || miProj.gridType == undefined_projection ) {
		WEBFW_LOG_ERROR("MiProjection::xyconvert: undefined projection.");
		return false;
	}
	/*
	 extern void xyconvert(int npos, float *x, float *y,
				int igtypa, float *ga,
				int igtypr, float *gr, int *ierror);
	 */
	::xyconvert( 1, &x_, &y_,
				  static_cast<int>(miProj.gridType), const_cast<float*>(miProj.gridSpec),
				  static_cast<int>(gridType), const_cast<float*>(gridSpec), &error);

	if( error != 0 ) {
		WEBFW_LOG_ERROR( "MiProjection::xyconvert: Conversion error. Error code: " << error << "." );
		return false;
	}

	return true;
}

bool 
MiProjection::
uvconvert( const MiProjection &miProj, float latitude, float longitude, float &u, float &v )const
{
	int error;
	WEBFW_USE_LOGGER( "handler" );

	if( gridType == undefined_projection || miProj.gridType == undefined_projection ) {
		WEBFW_LOG_DEBUG( "MiProjection::uvconvert: undefined projection." );
		return false;
	}

	/*
	    extern void uvconvert(int npos, float *xr, float *yr,
			float *u, float *v,
			int igtypa, float *ga,
			int igtypr, float *gr, float udef, int *ierror);
	 */ 

//	 npos - no. of positions (in xr,yr,u,v)
//	 xr - input x position in the output grid
//	 yr - input y position in the output grid
//	 u - input/output vector (velocity) component in x-direction
//	 v - input/output vector (velocity) component in y-direction
//	 igtypa - input grid type
//	 ga - input grid description
//	 igtypr - output grid type
//	 gr - output grid description
//	 ierror - output error status, 0=no error
//
//	cerr << "src: " << miProj << endl;
//	cerr << "dst: " << *this << endl;
//	cerr << "DEG_TO_RAD: " << DEG_TO_RAD << endl;
//	longitude *= DEG_TO_RAD;
//	latitude *= DEG_TO_RAD;
//	::xyconvert( 1, &longitude, &latitude,
//			    static_cast<int>(gridType), const_cast<float*>(gridSpec),
//				static_cast<int>(miProj.gridType), const_cast<float*>(miProj.gridSpec) , &error );
//
//	if( error != 0 ) {
//		WEBFW_LOG_ERROR( "MiProjection::uvconvert: xyconvert: Conversion error. Error code: " << error << "." );
//		return false;
//	}

	::uvconvert( 1, &longitude, &latitude, &u, &v,
			     static_cast<int>(miProj.gridType), const_cast<float*>(miProj.gridSpec),
			     static_cast<int>(gridType), const_cast<float*>(gridSpec), FLT_MAX, &error);

	if( error != 0 ) {
		WEBFW_LOG_ERROR( "MiProjection::uvconvert: Conversion error. Error code: " << error << "." );
		return false;
	}

	return true;

}

bool
MiProjection::
convertToDirectionAndLength( const MiProjection &srcProj,
		                     double latitudeY, double longitudeX,
		                     double u, double v,
							 double &direction, double &length, bool turn )const
{
	WEBFW_USE_LOGGER( "handler" );
	log4cpp::Priority::Value loglevel = WEBFW_GET_LOGLEVEL();

	ostringstream log;

	if( u == HUGE_VAL || v == HUGE_VAL )
		return false;

	if( u == 0 && v == 0 ) {
		direction = 0;
		length = 0;
		return true;
	}

	if( loglevel >= log4cpp::Priority::DEBUG )
		log << " MiProjection::convertToDirectionAndLength IN: u: " << u << " v: " << v << endl;

	if( !convertVectors( srcProj, longitudeX*DEG_TO_RAD, latitudeY*DEG_TO_RAD, u, v ) ){
		WEBFW_LOG_ERROR( "ProjectionHelper::convertToDirectionAndLength: failed!" );
		return false;
	}

	if( u == HUGE_VAL || v == HUGE_VAL ) {
		WEBFW_LOG_ERROR( "ProjectionHelper::convertToDirectionAndLength: failed u or/and v undefined!" );
		return false;
	}
	if( loglevel >= log4cpp::Priority::DEBUG ) {
		log << "----- CONVERTED: u: " << u << " v: " << v << endl;
		WEBFW_LOG_DEBUG( log.str() );
	}

	double deg=180./3.141592654;
	double fTurn;

	if( turn )
		fTurn = 90.0;
	else
		fTurn = 270.0;

	length = sqrt(u*u + v*v);

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
MiProjection::
convertToDirectionAndLengthMiLib( const MiProjection &srcProj,
							 float latitudeY, float longitudeX,
				             float u, float v,
							 float &direction, float &length, bool turn )const
{
	if( u == FLT_MAX || v == FLT_MAX )
		return false;

	WEBFW_USE_LOGGER( "handler" );
	MiProjection geographic;
	geographic.makeGeographic();
	float x, y;

	x = longitudeX;
	y = latitudeY;

//	if( ! xyconvert( srcProj, y, x ) )
//		return false;

	if( ! geographic.uvconvert( srcProj, y, x, u, v ) ){
		WEBFW_LOG_ERROR( "ProjectionHelper::convertToDirectionAndLength: failed!" );
		return false;
	}

	if( u == FLT_MAX || v == FLT_MAX ) {
		WEBFW_LOG_ERROR( "ProjectionHelper::convertToDirectionAndLength: failed u or/and v undefined!" );
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




ProjectionHelper::
ProjectionHelper()
	: wciProtocol( 5 )
{
	geographic.makeGeographic();
}

ProjectionHelper::
ProjectionHelper( int wciProtocol_ )
	: wciProtocol( wciProtocol_ )
{
	geographic.makeGeographic();
}


void
ProjectionHelper::
add( const std::string &provider, const MiProjection &projection )
{
	boost::mutex::scoped_lock lock( mutex );
	placenameMap[provider] = projection;
}

MiProjection
ProjectionHelper::
createProjection( float startx, float starty,
   				  float incrementx, float incrementy,
		          const std::string &projDefinition )
{
	using namespace boost;

	WEBFW_USE_LOGGER( "handler" );
	const string reFloat = "[-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?";
	regex reproj( "\\+proj *= *(\\w+)" );
	regex relat_0( "\\+lat_0 *= *("+ reFloat + ")" );
	regex relat_1( "\\+lat_1 *= *("+ reFloat + ")" );
	regex relat_2( "\\+lat_2 *= *("+ reFloat + ")" );
	regex relon_0( "\\+lon_0 *= *("+ reFloat + ")" );
	regex relat_ts( "\\+lat_ts *= *("+ reFloat + ")" );
	regex reo_lat_p( "\\+o_lat_p *= *("+ reFloat + ")" );
	string proj;
	ostringstream msg;
	ostringstream inData;

	try {

		inData << "ProjectionHelper::createProjection: startx: " << startx << " starty: " << starty
		       << " incrementx: " << incrementx << " incrementy: " << incrementy
		       << " projdef: '" << projDefinition << "'";
		WEBFW_LOG_DEBUG( "ProjectionHelper::createProjection: " << inData.str() );

		MiProjection::GridSpec gs;
		MiProjection::ProjectionType pType;
		smatch match;

		gs[0] = startx;  //row["startx"].as<float>();
		gs[1] = starty; //row["starty"].as<float>();
		gs[2] = incrementy; //row["incrementy"].as<float>();
		gs[3] = incrementx; //row["incrementx"].as<float>();
		proj = projDefinition; //row["projdefinition"].c_str();

		if( ! regex_search( proj, match, reproj ) ) {
			WEBFW_LOG_ERROR( inData.str() << "\nNo projection defined in the proj string '"<<projDefinition << "'." );
			return MiProjection();
		} else if( match[1] == "longlat" ) {
			pType = MiProjection::geographic;
		} else if( match[1] == "stere" ) {
			pType = MiProjection::polarstereographic;
		} else if( match[1] == "ob_tran" ) {
			pType = MiProjection::spherical_rotated;
		} else if( match[1] == "lcc" ) {
			pType = MiProjection::lambert;
		}else {
			WEBFW_LOG_INFO( inData.str() << "\nUnsupported projection: '" << match[1] << " projdef '" << projDefinition << "'");
			return MiProjection();
		}

		switch( pType ) {
		case MiProjection::geographic: {
			//				WEBFW_LOG_DEBUG( "ProjectionHelper::P4: geographic: "+proj);
			//From comments in milib shall gs allways be set to
			//this value for geographic projection.
			gs[0] = 1.0;
			gs[1] = 1.0;
			gs[2] = 1.0;
			gs[3] = 1.0;
			gs[4] = 0.0;
			gs[5] = 0.0;
			pType = MiProjection::geographic;
		}
		break;

		case MiProjection::polarstereographic: {
			float lat_0 = getProjFloat( proj, relat_0 );
			float lon_0 = getProjFloat( proj, relon_0 );
			float lat_ts = getProjFloat( proj, relat_ts );

			if( lat_0 == FLT_MAX || lon_0==FLT_MAX || lat_ts == FLT_MAX ) {
				WEBFW_LOG_ERROR( inData.str() << "\nPolarstereographic projection missing one of: +lat_0, +lon_0 or +lat_ts.");
				return MiProjection();
			}
			//				msg.str("");
			//				msg << "ProjectionHelper::P4: polarstereographic: lat_0: " << lat_0 << " lon_0: " << lon_0 << " lat_ts: " << lat_ts <<
			//						" (" << proj << ")";
			//				WEBFW_LOG_DEBUG( msg.str() );

			gs[0] = ( 0 - gs[0] ) / gs[3]; //poleGridX = (0 - startX) / iIncrement
			gs[1] = ( 0 - gs[1] ) / gs[2];  //poleGridY = (0 - startY) / jIncrement
			gs[3] = lon_0;
			gs[4] = lat_ts;
			gs[5] = 90-lat_0;
		}
		break;

		case MiProjection::spherical_rotated: {
			float lon_0 = getProjFloat( proj, relon_0 );
			float o_lat_p = getProjFloat( proj, reo_lat_p );

			if( lon_0==FLT_MAX || o_lat_p == FLT_MAX ) {
				WEBFW_LOG_ERROR( inData.str() << "\nSpherical Rotated projection missing one of: +lon_0 or +o_lat_p." );
				return MiProjection();
			}

			//				msg.str("");
			//				msg << "ProjectionHelper::P4: spherical_rotated: lon_0: " << lon_0 << " o_lat_p: " << o_lat_p <<
			//				       " (" << proj << ")";
			//				WEBFW_LOG_DEBUG( msg.str() );
			gs[4] = lon_0;
			gs[5] = 90 - o_lat_p;
		}
		break;

		case MiProjection::lambert: {
			float lat_0 = getProjFloat( proj, relat_0 );
			float lat_1 = getProjFloat( proj, relat_1 );
			float lat_2 = getProjFloat( proj, relat_2 );
			float lon_0 = getProjFloat( proj, relon_0 );


			//The milib projection library support only tangential lambert
			//projections so we should verify that lat_0, lat_1 and lat_2 are equal
			//if more than lat_1 is given.
			if( lon_0==FLT_MAX || lat_1 == FLT_MAX ) {
				WEBFW_LOG_ERROR( inData.str() <<  "\nLambert projection missing one of: +lon_0 or +lat_1." );
				return MiProjection();
			}

			//			"+proj=lcc +lat_0=64.35 +lat_1=64.35 +lat_2=64.35 +lon_0=-23 +R=6371000 +units=m"
			//				 gs[0] = false_easting + start_x;
			//			 	// south (latitude for x=1,y=1) (degrees)
			//				gs[1] = false_northing + start_y;
			// x (easting) increment (km)
			gs[2] /= 1000;  // in km
			// y (northing) increment (km)
			gs[3] /= 1000; // in km
			//				// reference longitude (degrees)
			gs[4] = lon_0; //longitude_of_projection_origin;
			//				// reference latitude , first and second parallel
			gs[5] = lat_1; //standard_parallel;
		}
		break;

		default:
			WEBFW_LOG_WARN( inData.str() << "\nUnsupported projection '" <<  match[1] << "'." );
			return MiProjection();

		}

		return MiProjection( gs, pType, proj.c_str() );
	}
	catch( std::exception &ex ) {
		WEBFW_LOG_ERROR( "EXCEPTION: ProjectionHelper::createProjection: " << ex.what() );
	}
	catch( ... ) {
		WEBFW_LOG_ERROR( "EXCEPTION: ProjectionHelper::createProjection: UNKNOWN reason!" );
	}

	return MiProjection();

}


ProjectionHelper::ProjectionMap::const_iterator
ProjectionHelper::
findProjection( const std::string &provider ) 
{
	ProjectionMap::const_iterator it;
	WEBFW_USE_LOGGER( "handler" );

#if 0
	WEBFW_LOG_DEBUG( "ProjectionHelper::findProjection: protocol: " << wciProtocol );
	WEBFW_LOG_DEBUG( "ProjectionHelper::findProjection: provider: " << provider );
	for( it = projectionMap.begin(); it != projectionMap.end(); ++it ) {
		WEBFW_LOG_DEBUG( "ProjectionHelper::findProjection: " << it->first << ": " << it->second );
	}
#endif

	it = projectionMap.find( provider );

	if( it != projectionMap.end() )
		return it;

	if( wciProtocol == 1 )
		return projectionMap.end();


#if 0	
	for( it = placenameMap.begin(); it != placenameMap.end(); ++it ) {
		WEBFW_LOG_DEBUG( "ProjectionHelper::findProjection: placenameMap: " << it->first << ": " << it->second );
	}
#endif		

	//The provider may be on the form 'provider [placename]'. 
	//Search the placename map for the place name and add it to the 
	//projectionMap.
	ProviderItem pi=ProviderList::decodeItem( provider );

#if 0
	WEBFW_LOG_DEBUG( "ProjectionHelper::findProjection: placenameMap: pi.placename: " << pi.placename );
	WEBFW_LOG_DEBUG( "ProjectionHelper::findProjection: placenameMap: pi.provider:  " << pi.provider );
	WEBFW_LOG_DEBUG( "ProjectionHelper::findProjection: placenameMap: pi.providerWithPlacename:  " << pi.providerWithPlacename() );
#endif

	if( pi.placename.empty() )
		return projectionMap.end();

	it = placenameMap.find( pi.placename );

	if( it == placenameMap.end() )
		return projectionMap.end();

	WEBFW_LOG_DEBUG( "ProjectionHelper: Added provider <" << pi.providerWithPlacename() << "> to the projection cache! " << it->second );

	projectionMap[ pi.providerWithPlacename() ] = it->second;

	return projectionMap.find( provider );
}




bool 
ProjectionHelper::
convertToDirectionAndLengthMiLib( const std::string &provider,
		                       float latitude, float longitude,
								     float u, float v,
								     float &direction, float &length, bool turn )const
{
	if( u == FLT_MAX || v == FLT_MAX )
		return false;

	WEBFW_USE_LOGGER( "handler" );

	{ //Mutex protected scope.
		boost::mutex::scoped_lock lock( mutex );

		ProjectionMap::const_iterator it =
				const_cast<ProjectionHelper*>(this)->findProjection( provider );

		if( it != projectionMap.end() )
		{
			if( it->second.getProjectionType() == MiProjection::undefined_projection ) {
				WEBFW_LOG_WARN( "ProjectionHelper::convertToDirectionAndLength: Projection definition for provider <" << provider << "> is 'undefined_projection'!" );
				return false;
			}

			if( ! geographic.uvconvert( it->second, latitude, longitude, u, v ) ){
				WEBFW_LOG_ERROR( "ProjectionHelper::convertToDirectionAndLength: failed!" );
				return false;
			}

			//WEBFW_LOG_DEBUG( "ProjectionHelper::convertToDDandFF: DEBUG provider <" << provider << ">!" );
		} else {
			WEBFW_LOG_WARN( "ProjectionHelper::convertToDirectionAndLength: WARNING no Projection definition for provider <" << provider << ">!" );
		}
	} //End mutex protected scope.

	if( u == FLT_MAX || v == FLT_MAX ) {
		WEBFW_LOG_ERROR( "ProjectionHelper::convertToDirectionAndLength: failed u or/and v undefined!" );
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
ProjectionHelper::
convertToDirectionAndLength( const std::string &provider,
		                      float latitude, float longitude,
							  float u_, float v_,
							  float &direction_, float &length_, bool turn )const
{
	if( u_ == FLT_MAX || v_ == FLT_MAX )
		return false;

	double direction=direction_;
	double length=length_;
	double u=u_;
	double v=v_;

	WEBFW_USE_LOGGER( "handler" );

	{ //Mutex protected scope.
		boost::mutex::scoped_lock lock( mutex );

		ProjectionMap::const_iterator it =
				const_cast<ProjectionHelper*>(this)->findProjection( provider );

		if( it != projectionMap.end() )
		{
			if( !it->second.valid() ) {
				WEBFW_LOG_WARN( "ProjectionHelper::convertToDirectionAndLength: Projection definition for provider <" << provider << "> is 'undefined_projection'!" );
				return false;
			}

			if( ! geographic.convertToDirectionAndLength(it->second, latitude, longitude, u, v, direction, length, turn ) ){
				WEBFW_LOG_ERROR( "ProjectionHelper::convertToDirectionAndLength: failed!" );
				return false;
			}

			//WEBFW_LOG_DEBUG( "ProjectionHelper::convertToDDandFF: DEBUG provider <" << provider << ">!" );
		} else {
			WEBFW_LOG_WARN( "ProjectionHelper::convertToDirectionAndLength: WARNING no Projection definition for provider <" << provider << ">!" );
		}
	} //End mutex protected scope.

	direction_ = static_cast<float>( direction );
	length_ = static_cast<float>( length );

	return true;
}

ProjectionHelper&
ProjectionHelper::
operator=( const ProjectionHelper &rhs )
{
	boost::mutex::scoped_lock lock( mutex );
	if( this != &rhs ) {

		projectionMap = rhs.projectionMap;
		placenameMap = rhs.placenameMap;
		geographic = rhs.geographic;
		wciProtocol = rhs.wciProtocol;
	}
	return *this;
}



std::ostream& 
operator<<(std::ostream &o, const MiProjection &proj )
{
	string grid;

	if( proj.gridType == MiProjection::undefined_projection ) {
		if( ! proj.proj )
			o << "MiProjection[] proj: 'undefined'";
		o << "MiProjection[] proj: '" << proj.projString << "'";
		return o;
	}

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

	if( ! proj.projString.empty( ) )
		o << " proj: '" << proj.projString << "'";

	return o;
}

}





