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
#include <wdb2TsApp.h>
#include <DbManager.h>
#include <Logger4cpp.h>
#include <boost/regex.hpp>
#include <sstream>

//#if defined RAD_TO_DEG
//#  undef RAD_TO_DEG
//#  define RAD_TO_DEG (180. / 3.141592654)
//#else
//#  define RAD_TO_DEG (180. / 3.141592654)
//#endif

using namespace std;

namespace {

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

	if( x == HUGE_VAL || y == HUGE_VAL )
		return HUGE_VAL;

	p.convertFromXYToGeographic( north_y, north_x );

	if( north_y == HUGE_VAL || north_x == HUGE_VAL )
		return HUGE_VAL;

	//north_y = std::min<double>(north_y + 0.1, 90);
	if( north_y >= 0 )
		north_y = std::min<double>(north_y + 0.1, 89.999999);
	else
		north_y = std::max<double>(north_y + 0.1, -89.999999);

	p.convertFromGeographicToXY( north_y, north_x );

	if( north_y == HUGE_VAL || north_x == HUGE_VAL )
		return HUGE_VAL;

	return wdb2ts::uv(north_x - x, north_y - y).angle();
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
		if( north_x[i] == HUGE_VAL || north_y[i] == HUGE_VAL)
			ret[i] = HUGE_VAL;
		else
			ret[i] = wdb2ts::uv(north_x[i] - x[i], north_y[i] - y[i]).angle();
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


uv::
uv( double u, double v) :
u_( u ), v_( v )
{
	if( u_ == HUGE_VAL || v_ == HUGE_VAL ||
		 u_ == DBL_MAX || v_ == DBL_MAX ) {
		u_ = HUGE_VAL;
		v_ = HUGE_VAL;
	}
}

uv::
uv(double angle)
{
	if( angle == HUGE_VAL ) {
		u_ = HUGE_VAL;
		v_ = HUGE_VAL;
	} else {
		u_ = std::sin(angle * DEG_TO_RAD);
		v_ = std::cos(angle * DEG_TO_RAD);
	}
}

double
uv::
angle() const
{
	if ( u_ == HUGE_VAL || v_ == HUGE_VAL ) {
		return HUGE_VAL;
	}

	double ret = 90 - std::atan2(v_, u_) * RAD_TO_DEG;

	if (ret < 0)
		ret += 360;
	if (360 < ret)
		ret -= 360;

	return ret;
}

void
uv::
directionAndLength( double &direction, double &length, bool turn )const
{
	double fTurn;

	if( u_ == HUGE_VAL || v_ == HUGE_VAL ) {
		direction = HUGE_VAL;
		length = HUGE_VAL;
		return;
	}

	if( ! turn )
		fTurn = 180.0;

	length = std::sqrt(u_*u_ + v_*v_);

	if( length>0.0001) {
		direction = angle() - fTurn;
		if( direction > 360 ) direction -=360;
		else if( direction < 0   ) direction +=360;
	} else {
		direction = 0;
		length = 0;
	}
}

std::ostream&
operator<<( std::ostream &o, const uv &uv_ )
{
	if( uv_.u_ == HUGE_VAL || uv_.v_ == HUGE_VAL ) {
		o << "( (n/a)°, (n/a) )";
	} else {
		double d, l;
		uv_.directionAndLength( d, l);
		o << "( " << d << "°, " << l << " )";
	}
	return o;
}

MiProjection::
MiProjection()
	: proj( 0 ), geoproj( 0 )
{
	init();
	makeGeographic();
}

MiProjection::
MiProjection( const std::string &projDef )
	: projString( projDef ), geoproj( 0 )
{
	proj = pj_init_plus( projDef.c_str() );
	init();
}



MiProjection::
MiProjection( const MiProjection &miProj )
	: projString( miProj.projString ), proj( 0 ), geoproj( 0 )
{
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

  //double ff;

  for (int i = 0; i < nvec; i++) {
	  length[i] = sqrt(u[i] * u[i] + v[i] * v[i]);

      if (length[i] > 0.0001) {
    	  dd[i] = 270. - atan2(v[i], u[i])*RAD_TO_DEG;

    	  if (dd[i] > 360)
    		  dd[i] -= 360;
    	  if (dd[i] < 0)
    		  dd[i] += 360;
      } else
    	  dd[i] = 0;
  }

  return true;
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


bool
MiProjection::
convertFromGeographicToXY( int nvec,  double *latitudeToY, double *longitudeToX )const
{
	if( ! proj || ! geoproj ) {
		cerr << "ERROR: convertFromGeographicToXY: No proj or geoproj definition.\n";
		return false;
	}

	for( int i=0; i<nvec; ++i ) {
		if( longitudeToX[i] != HUGE_VAL && latitudeToY[i] != HUGE_VAL &&
			 longitudeToX[i] != DBL_MAX  && latitudeToY[i] != DBL_MAX) {
			longitudeToX[i] *= DEG_TO_RAD;
			latitudeToY[i] *= DEG_TO_RAD;
		} else {
			longitudeToX[i] = HUGE_VAL;
			latitudeToY[i] = HUGE_VAL;
		}
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
		if( xToLongitude[i] != HUGE_VAL && yToLatitude[i] != HUGE_VAL &&
			 xToLongitude[i] != DBL_MAX && yToLatitude[i] != DBL_MAX) {
			xToLongitude[i] *= RAD_TO_DEG;
			yToLatitude[i] *= RAD_TO_DEG;
		} else {
			xToLongitude[i] = HUGE_VAL;
			yToLatitude[i] = HUGE_VAL;
		}
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
		  u[i] = convert.u() * length;
		  v[i] = convert.v() * length;
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

	if(u == udef || v == udef || from_north == udef || to_north == udef )
		return false;

	const float length = std::sqrt( u*u + v*v );

	// the difference between angles in the two projections:
	double angle_diff = to_north - from_north;

	double new_direction = turn(uv(u, v).angle(), angle_diff);
	// float new_direction = to_north[ i ]; // This makes all directions be north.
	uv convert(new_direction);
	u = convert.u() * length;
	v = convert.v() * length;
	return true;
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
convertToDirectionAndLength( const MiProjection &srcProj,
		                     double latitudeY, double longitudeX,
		                     double u, double v,
							 double &direction, double &length, bool turn )const
{
	WEBFW_USE_LOGGER( "handler" );
	log4cpp::Priority::Value loglevel = WEBFW_GET_LOGLEVEL();

	ostringstream log;

	if( u == HUGE_VAL || v == HUGE_VAL || u == DBL_MAX || v == DBL_MAX )
		return false;

	if( fabs(u) < 0.0001 && fabs(v) < 0.0001 ) {
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

	uv( u, v ).directionAndLength( direction, length );

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
createProjection( const std::string &projDefinition )
{
	using namespace boost;

	WEBFW_USE_LOGGER( "handler" );
	string proj;
	ostringstream msg;
	ostringstream inData;

	try {

		inData << "ProjectionHelper::createProjection:  projdef: '" << projDefinition << "'";
		WEBFW_LOG_DEBUG( "ProjectionHelper::createProjection: " << inData.str() );
		return MiProjection( proj.c_str() );
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

	if( ! proj.proj )
		o << "MiProjection: proj: 'undefined'";
	else
		o << "MiProjection: proj: '" << proj.projString << "'";
		return o;
}

}
