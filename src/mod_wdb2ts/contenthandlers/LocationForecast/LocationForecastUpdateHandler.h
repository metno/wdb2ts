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
#ifndef __LOCATION_FORECAST_UPDATE_HANDLER_H__
#define __LOCATION_FORECAST_UPDATE_HANDLER_H__

#include <RequestHandler.h>
#include <UpdateProviderReftimes.h>
#include <HandlerBase.h>
#include <wdb2TsApp.h>


namespace wdb2ts {

/**
 * Handel location forecast update request.
 *  \em http://server/metno-wdb2ts/locationforecast/update
 */
class LocationForecastUpdateHandler:
	public HandlerBase
{ 
public:
	/** Default Constructor */
	LocationForecastUpdateHandler();
	/** Constructor
	 * @param	major	??
	 * @param	minor	??
	 */
	LocationForecastUpdateHandler( int major, int minor );
	/** Default Destructor */
	~LocationForecastUpdateHandler();
   /** Identify the RequestHandler
    * @returns	The name of the Request Handler
    */
   virtual const char *name() const { return "LocationForecastUpdateHandler"; };
  
	bool configure( const wdb2ts::config::ActionParam &params,
				       const wdb2ts::config::Config::Query &query,
						 const std::string &wdbDB );
	
   virtual void get( webfw::Request  &req, 
                     webfw::Response &response, 
                     webfw::Logger   &logger );
   
private:

	std::string  wdbDB;
	std::string  updateid;
	int          wciProtocol;
	std::string getWdbId( Wdb2TsApp *app );
	ProviderRefTimeList getProviderReftimes( Wdb2TsApp *app );
	std::string updateStatus( ProviderRefTimeList &oldRefTime, 
							  ProviderRefTimeList &newRefTime )const;
	bool decodeQuery( const std::string &query, ProviderRefTimeList &newRefTime )const;
	bool getProviderPriorityList( Wdb2TsApp *app, ProviderList &providerPriorityList )const;

	/**
	 * checkProviders checks if the requested providers is defined in the provider_priority list.
	 * If not remove it from the requestedUpdate list.
	 *
	 * The disable status from oldRefTime is set for each requestedUpdate. This is to preserve
	 * the disable status from between update request.
	 */
	void checkProviders( const ProviderList &providerList,
			             const ProviderRefTimeList &oldRefTime,
			             ProviderRefTimeList &requestedUpdate )const;
	void checkAgainstExistingProviders( const ProviderRefTimeList &exitingProviders, 
			                              ProviderRefTimeList &newRefTime ) const;
};

}


#endif
