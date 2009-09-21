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


#ifndef __WDB2TS_LOCATIONDATA_H__
#define __WDB2TS_LOCATIONDATA_H__

#include <iostream>
#include <list>
#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/shared_ptr.hpp>
#include <PointDataHelper.h>
#include <UpdateProviderReftimes.h>
#include <LocationElem.h>

namespace wdb2ts {

/**
 * LocationData is a helper class to to iterates trough 
 * 'validtotime's values in a TimeSerie map,
 * a TimeSerie map. A TimeSerie map is defined as:
 *  map[validtotime][validfromtime][providername].
 */
class LocationData
{
	LocationData( const LocationData& );
	LocationData& operator=( const LocationData & );
	
	ProviderList providerPriority_;
	TopoProviderMap modelTopoProviders_;
	TimeSeriePtr timeSeriePtr;
	TimeSerie *timeSerie;
	CITimeSerie itTimeSerie;
	LocationElem locationElem;
	
	CITimeSerie	findProviderData( const std::string &provider, 
			                        const boost::posix_time::ptime &startAtToTime )const;

	
public:
	LocationData():timeSerie(0){}
	LocationData( wdb2ts::TimeSeriePtr timeSerie,
				  float longitude, float latitude, int hight,
			      const ProviderList &providerPriority,
			      const TopoProviderMap &modelTopoProviders );
	
	~LocationData();
	
	float latitude() const { return locationElem.latitude(); }
	float longitude() const { return locationElem.longitude(); }
	int hight() const { return locationElem.hight(); }
	void hight( int h ) { locationElem.hight( h ); }
	

	/**
	 *Use the hight to model topografi for the point given with latitude/longitude.
	 *
	 *The call of the method destroy the state of the clss so a new call to init is needed
	 *before hasNext and next is called again.
	 */
	int hightFromModelTopo();

	ProviderList providerPriority() const { return providerPriority_; }
	
	bool hasDataForTime( const boost::posix_time::ptime &fromtime, 
			               const boost::posix_time::ptime &totime,
			               const std::string &provider ) const;
	
	/**
	 * Initialize the iterator. Start at 'validtotime' equal
	 * or greater than 'startAtToTime'.
	 * 
	 * A invalid value of startAtToTime set initialize to the first element.
	 * So this can be uses to scan trough all times for a given reference time.
	 *  
	 * @param startAtToTime Initialize the iterator to start at 'validtotime'
	 *   equal or greater than 'startAtToTime'. 
	 * @param provider set the forecastprovider to provider. 
	 */
	bool init( const boost::posix_time::ptime &startAtToTime, const std::string &provider="" );

	/**
	 * Returns true if there is more 'validtotime' values.
	 */
	bool hasNext();
	
	/**
	 * Returns the next element.
	 * 
	 * @return LocationElem The next element with data.
	 */
	LocationElem *next();
};

typedef boost::shared_ptr<LocationData> LocationDataPtr;

}


#endif //__WDB2TS_POINTDATA_H__
