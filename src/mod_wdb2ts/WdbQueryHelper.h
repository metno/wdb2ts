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


#ifndef __WDB2TS_WDBQUERYHELPER_H__
#define __WDB2TS_WDBQUERYHELPER_H__

#include <string>
#include <list>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <WciWebQuery.h>
#include <wdb2TsApp.h>
#include <wdb2TsExceptions.h>
#include <Config.h>
#include <LocationPoint.h>

namespace wdb2ts {

/**
 * WdbQueryHelper is a helper class to hold all querys 
 * to a wdb database that is needed to retrive all data 
 * for a multi query computation.
 * 
 * The format of the query is the url format that
 * is used in WdbWebQuery.
 */
class WdbQueryHelper
{
	WdbQueryHelper( const WdbQueryHelper &);
	WdbQueryHelper& operator=( const WdbQueryHelper &);
	
	const wdb2ts::config::Config::Query                 urlQuerys;
	wdb2ts::config::Config::Query::const_iterator itNext;
	std::string position;
	ProviderRefTimeList reftimes;
	ProviderList           providerPriority;
	ProviderList::iterator itCurProviderPriority;
	ProviderList           curProviderPriority;
	bool first;
	std::string dataProviders;
	std::string reftimeProvider;
	WciWebQuery webQuery;
	bool        queryMustHaveData; 
	bool        stopIfQueryHasData;
	bool        refTimeFrom_IsEqualTo_ReftTimeTo;
	bool        isPolygon;
	std::string validTime;
	std::string wdbid_;
	
	std::string getDataversionString( const std::list<std::string> &dataproviderList )const;

	void doGetProviderReftime( const std::string &provider, 
			                     boost::posix_time::ptime &refTimeFrom,  
			                     boost::posix_time::ptime &refTimeTo ) const; 

	
	bool getProviderReftime( const std::string &provider, 
				                boost::posix_time::ptime &refTimeFrom,
				                boost::posix_time::ptime &refTimeTo );

	bool getProviderReftime( const std::list<std::string> &providerList, 
					             boost::posix_time::ptime &refTimeFrom,
					             boost::posix_time::ptime &refTimeTo );
	
public:
	
	/**
	 * Default contructor to create a empty list of url
	 * calls.
	 */
	WdbQueryHelper();
	
	/**
	 * Contructor that takes a list of urlQuerys that is to
	 * be used to create wci.read call to a wdb database.
	 * 
	 * @param urlQuerys a list of url to be decoded to wci.read 
	 *        call.
	 */
	WdbQueryHelper( const wdb2ts::config::Config::Query &urlQuerys, int wciProtocol );
	
	/**
	 * Initialize the query before a call to hasNext and next.
	 * 
	 * @param lat the latitude part of the request.
	 * @param lon the longitude part of the request.
	 * @param A map of provider/reftimes for the request.
	 * @param extraParams additional url params to use in the request.
	 * @throws logic_error if the locationPoints is empty.
	 */
	void init( const LocationPointList &locationPoints,
			   const boost::posix_time::ptime &toTime,
			   bool isPolygon,
			   const ProviderRefTimeList &reftimes,
			   const ProviderList &providerPriority,
			   const std::string &extraParams="" );
	
	/**
	 * Check if there is more querys to be executed.
	 */
	bool hasNext();
	
	
	/**
	 * Return a comma separated list of data providers.
	 */
	std::string dataprovider()const;
	
	std::string wdbid()const { return wdbid_; }

	/**
	 * Return the next wci read query.
	 * 
	 * An exception is thrown if the url for the
	 * query is malformed. 
	 * 
	 * @param mustHaveData This query is defined with that is muist return data.
	 * @see WciWebQuery
	 * @exception std::logic_error
	 * @exception wdb2ts::NoReftime when we dont have  any data for a providerid.
	 */
	std::string next( bool &mustHaveData, bool &stopIfData );
	
};


}


#endif
