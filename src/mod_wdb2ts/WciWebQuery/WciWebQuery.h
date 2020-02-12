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

#ifndef __WCIWEBQUERY_H__
#define __WCIWEBQUERY_H__

#include <stdexcept>
#include <string>
#include <UrlParamFloat.h>
#include <UrlParamInteger.h>
#include <UrlParamDataProvider.h>
#include <UrlParamTimeSpec.h>
#include <UrlParamParameter.h>
#include <UrlParamLevelSpec.h>
#include <UrlParamFormat.h>
#include <UrlParamPolygon.h>

namespace wdb2ts {


/**
 * WciWebQuery is a helper class to decode querys on the form:
 * 
 * lat=10;lon=10;alt=10;
 * dataprovider=hirlam 8;
 * point_interpolation=bilinear; 
 * reftime=2007-12-10T10:00,2007-12-10T10:00,exact;
 * validtime=2007-12-10T00:00,2007-12-10T10:00,intersect
 * parameter=instant pressure of air,instant temperature of air,instant velocity of air (u-component);
 * levelspec=2,2,above ground,exact;
 * dataversion=-1;
 * format=CSV
 * 
 * Valid keys is:
 *  - lat The latitude part of the point of interest.
 *  - lon The longitude part of the point of interest.
 *  - alt The altitude to the point of interest.
 *  - dataprovider Who has "created" the data.
 *  - point_interpolation Interpolate the data value according to this value.
 *    Valid values is:  exact|nearest|surround|bilinear.
 *    Default value: 'nearest'.
 *  - reftime The reference time for the creation of the dataset.
 *  - validtime Only search for data with valid time in this interval.
 *  - parameter A comma separated list of parameters to search for. It is
 *      importent to enclose a parameter with ',' in the name with '"'.
 *  - levelspec The levelspecification to the data to search for.
 *  - dataversion The dataversion of the data of interest, -1 define the
 *     most updated data (this is the default).
 *  - format The format we want the data in. This is not implemented. 
 */
class WciWebQuery
{
	std::string returnColoumns;
	bool onlyDecodeParams_;
public:
	UrlParamDataProvider  dataprovider;
	UrlParamFloat         latitude;
	UrlParamFloat         longitude;
	UrlParamString        pointInterpolation;
	UrlParamTimeSpec      reftime;
	UrlParamTimeSpec      validtime;
	UrlParamParameter     parameter;
	UrlParamLevelSpec     levelspec;
	UrlParamInteger   	  dataversion;
	UrlParamFormat        format;
	UrlParamFloat         altitude;
	UrlParamPolygon       polygon;
	bool                  isPolygon;


	/** 
	 * Default Constructor
	 */
	WciWebQuery( const std::string &returnCol="*" );
	WciWebQuery( int protocol, const std::string &returnCol="*");
	/** 
	 * Copy Constructor
	 * @param query	Object to be copied
	 */
	/*WciWebQuery( const WciWebQuery &query );*/
	/**
	 * Default Destructor
	 */
	virtual ~WciWebQuery();

	void onlyDecodeParams(const std::string &query );
   
	/**
	 * Decodes a query on the form:
	 * 
	 * http://server/path/?lat=10;lon=10;alt=10;
	 *   dataprovider=hirlam 8;
	 *   reftime=2007-12-10T10:00,2007-12-10T10:00,exact;
	 *   validtime=2007-12-10T00:00,2007-12-10T10:00,intersect
	 *   parameter=instant pressure of air,instant temperature of air,instant velocity of air (u-component);
	 *   levelspec=2,2,above ground,exact;
	 *   dataversion=-1;
	 *   format=CSV
	 *
	 * @return The decoded query wci.read query.
	 * @exception logic_error
	 */
	virtual std::string decode( const std::string &query );
   
	/**
	 * Return the decoded wci.read query.
	 * Throws exception if decode has not been successfully called prior 
	 * to the function call.
	 * 
	 * @exception logic_error
	 */
	std::string wciReadQuery() const;

private:
	/// Parameter to store the decoded wci.read query
	std::string wciReadQuery_;
   
};

}



#endif 
