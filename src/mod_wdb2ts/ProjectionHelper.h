

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
	MiProjection( const MiProjection &proj );
	MiProjection( GridSpec gridSpec, ProjectionType pt );
	
	MiProjection& operator=( const MiProjection &rhs );
	
	void set( GridSpec gridSpec, ProjectionType pt  );
	void makeGeographic();
	
	const float* getGridSpec()const { return gridSpec; }
	ProjectionType getProjectionType()const { return gridType; }
	
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
	
	friend std::ostream& operator<<(std::ostream &o, const MiProjection &proj );

private:
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
			                      const wdb2ts::config::ActionParam &params );


public:
	
	ProjectionHelper();
	
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
