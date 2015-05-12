

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


#ifndef __PROJECTIONHELPER_H__
#define __PROJECTIONHELPER_H__

//#include <projects.h>
#include <proj_api.h>
#include <string>
#include <map>
#include <pqxx/pqxx>
#include <Config.h>
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <ProviderList.h>

namespace wdb2ts {

class uv {
  double u_;
  double v_;
public:

  uv():u_( HUGE_VAL), v_( HUGE_VAL ) {}
  uv( double u, double v);

  explicit uv(double angle);

  double u()const { return u_;}
  double v()const { return v_;}
  double angle() const;
  void  directionAndLength( double &direction, double &length, bool turn=false)const;

  friend std::ostream& operator<<( std::ostream &o, const uv &uv_ );
};

std::ostream& operator<<( std::ostream &o, const uv &uv_ );


class Wdb2TsApp;

class MiProjection {
	
public:
			  
	MiProjection();
	MiProjection( const std::string &projDef );
	MiProjection( const MiProjection &proj );
	~MiProjection();

	MiProjection& operator=( const MiProjection &rhs );
	
	bool valid()const{ return geoproj && proj; }
	void makeGeographic();

	bool calculateVectorRotationElements(const MiProjection& srcProj, int nvec,
			                             const double *to_x, const double *to_y,
			                             double *cosa_out,
			                             double *sina_out) const;

	bool calculateVectorRotationElements( const MiProjection& srcProj,
									      const double to_x, const double to_y,
									      double &cosa_out, double &sina_out) const;

	bool getVectorRotationElements( const MiProjection &data_area,
		                            const MiProjection &map_area,
		                            const int nvec, const double *x, const double *y,
		                            double ** cosx,	double ** sinx) const;

	bool getVectorRotationElement( const MiProjection &data_area,
								   const MiProjection &map_area,
			                       double x, double y,
			                       double &cosx_out, double &sinx) const;

	bool getVectors(const MiProjection& data_area, const MiProjection& map_area,
		     		int nvec, const double *x, const double *y, double *u, double *v )const;

	bool getVector( const MiProjection& data_area, const MiProjection& map_area,
		        	double x, double y, double &u, double &v )const;


	bool getPoints(const MiProjection& area, const MiProjection& map_area,
	                int npos, double *x, double *y )const;

	bool xyv2geo( const MiProjection& area,
			      int nvec, const double *to_x, const double *to_y,
			      double *u, double *v ) const;

	bool uv2geo( const MiProjection& uvProj,
				 int nvec, const double *longX, const double *latY, double *u,
	             double *v)const;


	bool directionAndLength(int nvec, const double *longitude, const double  *latitude,
							double *u, double *v, const MiProjection &area,
							double *dd, double *length ) const;


	/**
	 * Convert coordinates between projection.
	 *
	 * On call of this method x (longitude) and y (latitude) is given in the
	 * projection of proj. On return the x and y is given
	 * in the projection of this projection.
	 * x and y are arrays and nvec is number of elements in the arrays.
	 *
	 * @param proj Convert x/y (longitude/latitude) from projection (proj) to this projection.
	 * @param nvec numbers of elements in the x and y arrays.
	 * @param x (longitude) x in the projection given with proj.
	 * @param y (latitude) y in the projection given with proj.
	 * @return true on success and false on failure.
	 */

	bool transform( const MiProjection &proj, int nvec, double *y, double *x )const;
	

	/**
	 * Convert from geographic to the projection represented
	 * by this projection definition.
	 *
	 * On input latitude and longitude is in decimal degrees. On
	 * output the coordinates is in X and Y.
	 *
	 * @param latitude Geographic latitude on input and Y on output.
	 * @param longitude Geographic longitude on input and Y on output.
	 * @return true on success and false on failure.
	 */
	bool convertFromGeographicToXY( double &latitudeToY, double &longitudeToX )const;
	bool convertFromXYToGeographic( double &yToLatitude, double &xToLongitude )const;

	bool convertFromGeographicToXY( int nvec, double *latitudeToY, double *longitudeToX )const;
	bool convertFromXYToGeographic( int nvec, double *yToLatitude, double *xToLongitude )const;

	/**
	 * Convert vector values from the srcProj projections to this projection
	 * at position to_x/to_y  (longitude/latitude). The vector components is
	 * given with u and v.
	 *
	 * The converted vectors is returned in u and v.
	 */
	bool convertVectors( const MiProjection& srcProj, int nvec,
					     double * to_x,  double * to_y,
						 double * u, double * v) const;

	bool convertVectors( const MiProjection& srcProj,
						 double to_x, double to_y,
						 double &u, double &v) const;
	
	/**
	 * Convert a vector represented with u and v to direction and length at a
	 * given location given with latitudeY and longitudeX. The location
	 * (latitudeY/longitudeX) is in the projectin given with srcProj. The projection
	 * to the vector components is projection given with this MiProjection object.
	 * Most of the time the location is in the geographic projection and the projection
	 * to the vector is in the projection to original field.
	 *
	 * @param srcProj The projection the latitudeY and longitudeX location is in.
	 * @param latitudeY the latitude (Y) component of the location.
	 * @param longitudeX the longitude (X) component of the location.
	 * @param u the u component the vector.
	 * @param v the v component the vector.
	 * @param[out] direction The direction to the vector in decimal deegrees.
	 * @param[out] length the length of the vector.
	 * @param turn Turn the resulting direction 180 deegrees. (The atmosphere people and
	 * sea-physic people can't agree on the direction a wind blow or the sea flows :-).)
	 */
	bool convertToDirectionAndLength( const MiProjection &srcProj,
									  double latitudeY, double longitudeX,
			                          double u, double v,
								      double &direction, double &length, bool turn=false )const;

	friend std::ostream& operator<<(std::ostream &o, const MiProjection &proj );

private:
	void init();
	std::string projString;
	projPJ proj;
	projPJ geoproj;
};


/**
 * ProjectionHelper is a class to help reproject vector data like wind, etc.
 *  
 */
class ProjectionHelper {
public:
	/// providername, Projection
	typedef std::map<std::string, MiProjection> ProjectionMap;
	
	
private: 
	/// projectionMap associate provider name with projection of the field. 
	ProjectionMap projectionMap;
	
	/// placenameMap associate a placename with projection of the field.
	ProjectionMap placenameMap;
	
	///A geographic projection
	MiProjection  geographic;

	int wciProtocol;
	mutable boost::mutex mutex;

protected:
	
	ProjectionMap::const_iterator
			findProjection( const std::string &provider );

public:
	ProjectionHelper();
	ProjectionHelper(int wciProtocol);
	
	void add( const std::string &provider, const MiProjection &projection );

	static MiProjection createProjection( const std::string &projDefinition );

	ProjectionHelper& operator=( const ProjectionHelper &rhs );

	
	/**
	 * Compute to direction and length (speed) from vector
	 * components u and v. u and v is oriented to the x and y direction
	 * of the projections of the fields. The field projection is 
	 * loaded from the database and maintained on the basis of provider 
	 * name in projectionMap.
	 * 
	 * latitude and longitude is geographic latitude and longitude.
	 * 
	 * @param[in] provider Use the projection for this provider.
	 * @param[in] latitude geographic latitude.
	 * @param[in] longitude geographic longitude.
	 * @param[in] u The x component to the vector in the projection given by the provider (placename).
	 * @param[in] v The y component to the vector in the projection given by the provider (placename).
	 * @param[out] direction in degrees.
	 * @param[out] length The length of the vector (speed).
	 * @param[out] turn turn the direction 180 degrees. ocean ant atmosphere parameters is in
	 *    opposite direction. ocean parameters need turn=true.
	 */
	bool convertToDirectionAndLength( const std::string &provider,
									   float latitude, float longitude,
									   float u, float v,
									   float &direction, float &length, bool turn=false )const;
};


typedef boost::shared_ptr<ProjectionHelper> ProjectionHelperPtr;

std::ostream& 
operator<<(std::ostream &o, const MiProjection &proj );

}


#endif
