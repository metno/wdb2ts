

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
#include <ProviderList.h>

namespace wdb2ts {


class Wdb2TsApp;

class MiProjection {
	
public:
	typedef enum  {
			undefined_projection = 0,
			polarstereographic_60= 1,
			geographic=            2,
			spherical_rotated=     3,
			polarstereographic=    4,
			mercator=              5,
			lambert=               6
		} ProjectionType;
			
	typedef float GridSpec[6] ;
			  
	MiProjection();
	MiProjection( const std::string &projDef );
	MiProjection( const MiProjection &proj );
	MiProjection( GridSpec gridSpec, ProjectionType pt, const char *proj=0 );
	~MiProjection();

	MiProjection& operator=( const MiProjection &rhs );
	
	void set( GridSpec gridSpec, ProjectionType pt  );

	std::string createProjDefinition( bool addDatum );

	bool valid()const{ return geoproj && proj; }
	void makeGeographic();
	
	const float* getGridSpec()const { return gridSpec; }
	ProjectionType getProjectionType()const { return gridType; }
	

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
	 * Convert coordinates between projection.
	 * 
	 * On call of this method latitude and longitude is given in the 
	 * projection of proj. On return the latitude and longitude is given
	 * in the projection of this projection.
	 * 
	 * @param proj Convert latitude/longitude from proj to this projection.
	 * @param latitude Latitude in the projection given with proj.
	 * @param longitude Longitude in the projection given with proj.
	 * @return true on success and false on failure.
	 */
	bool xyconvert( const MiProjection &proj, float &latitude, float &longitude )const;
	

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
	 * Turn vector (ex. velocity) components between different projections.
    * 
    * You may use xyconvert to convert the positions first.
    * 
    * latitude and longitude is in this projection.
    * On input u and v is in the projection of proj and on output
    * it is in the projection of this projection.
    *
    * @param proj Convert u/v from proj to this projection. Latitude/longitude 
    *             is in this projection.
    * @param latitude Latitude in this projection.
    * @param longitude Longitude in this projection.
    * @param u On input in the projection of proj, on output in this projection.
    * @param v On input in the projection of proj, on output in this projection.
    *
	 * @return true on success and false on failure.
	 */
	bool uvconvert( const MiProjection &proj, float latitude, float longitude, float &u, float &v )const;
	
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

	bool convertToDirectionAndLengthMiLib( const MiProjection &srcProj,
									  float latitudeY, float longitudeX,
				                      float u, float v,
									  float &direction, float &length, bool turn=false )const;


	friend std::ostream& operator<<(std::ostream &o, const MiProjection &proj );

private:
	void init();
	std::string projString;
	projPJ proj;
	projPJ geoproj;
	ProjectionType gridType;
	GridSpec gridSpec;
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
	
	bool isInitialized;
	int wciProtocol;
	boost::mutex        mutex;
	

protected:
	
	ProjectionMap::const_iterator
			findProjection( const std::string &provider );

	bool loadFromDBWciProtocol_1( pqxx::connection& con, 
			                      const wdb2ts::config::ActionParam &params );
	
	bool loadFromDBWciProtocol_2( pqxx::connection& con, 
				                  const wdb2ts::config::ActionParam &params );

	bool loadFromDBWciProtocol_4( pqxx::connection& con,
			                      const wdb2ts::config::ActionParam &params,
			                      int protocol );


public:
	
	ProjectionHelper();
	

	static MiProjection createProjection( float startx, float starty,
				  	  	  	  	  	  	  float incrementx, float incrementy,
				  	  	  	  	  	  	  const std::string &projDefinition );


	/**
	 * Load the projection data in projectionMap from wdb.
	 * Which projection that is associated with each providername is defined
	 * by the actionparam wdb.projection in the configuration file.
	 * The value of wdb.projection is a ';' separated list of providername=placename.
	 * 
	 * Ex.
	 * 	value="probability_forecast=ec ensemble;proff=proff;hirlam 10=hirlam 10"
	 */
	bool loadFromDB( pqxx::connection& con, 
			           const wdb2ts::config::ActionParam &params,
			           int wciProtocol );
	
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
	 * @param[out] direction in degrease.
	 * @param[out] length The length of the vector (speed).
	 * @param[out] turn turn the direction 180 degrease. ocean ant atmosphere parameters is in
	 *    oposit direction. ocean parameters need turn=true.
	 */
	bool convertToDirectionAndLengthMiLib( const std::string &provider,
								      float latitude, float longitude,
								      float u, float v,
								      float &direction, float &length, bool turn=false )const;
	
	bool convertToDirectionAndLength( const std::string &provider,
									   float latitude, float longitude,
									   float u, float v,
									   float &direction, float &length, bool turn=false )const;
};

bool 
configureWdbProjection( ProjectionHelper &projectionHelper,
		                  const wdb2ts::config::ActionParam &params,
		                  int wciProtocol,
		                  const std::string &wdbDB,
		                  Wdb2TsApp *app );


std::ostream& 
operator<<(std::ostream &o, const MiProjection &proj );

}


#endif
