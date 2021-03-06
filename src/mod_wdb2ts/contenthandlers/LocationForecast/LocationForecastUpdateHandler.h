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
	

	/**
	 * Valid querys for update requests:
	 *
	 * http://host/...update?provider=reftime[,dataversion]
	 * If the dataversion part is missing -1 is assumed.
	 * reftime is on the form YYYY-MM-DDThh:mm:ssZ
	 *
	 * Reftime may also be left out. This updates reftime
	 * to latest reftime for the provider.
	 * http://host/...update?provider=,dataversion
	 *
	 * Reftime may also be set to 0. This only
	 * updates the dataversion we use for the provider
	 * without touching the reftime.
	 * http://host/...update?provider=0,dataversion
	 *
	 * A provider can be disabled/enabled with querys on the form
	 * http://host/...update?provider=(enable/diable)
	 * If a provider is disabled it will be disabled until it is enabled
	 * with http://host/...update?provider=enable
	 */
   virtual void get( webfw::Request  &req, 
                     webfw::Response &response, 
                     webfw::Logger   &logger );
   static bool decodeQueryValue( const std::string &provider, const std::string &val,
		                         ProviderRefTimeList &newRefTime);
   static bool decodeQuery( const std::string &query, ProviderRefTimeList &newRefTime, bool &debug );
private:

	std::string  wdbDB;
	std::string  updateid;
	int          wciProtocol;
	boost::mutex mutex;
	std::string getWdbId( Wdb2TsApp *app );
	ProviderRefTimeList getProviderReftimes( Wdb2TsApp *app );
	std::string updateStatus( ProviderRefTimeList &oldRefTime, 
							  ProviderRefTimeList &newRefTime )const;
	bool getProviderPriorityList( Wdb2TsApp *app, const std::string &wdbID, ProviderList &providerPriorityList )const;
	bool getProviderPriorityList( Wdb2TsApp *app, ProviderList &providerPriorityList )const;

	/**
	 * checkProviders checks if the requested providers is defined in the provider_priority list.
	 * If not remove it from the requestedUpdate list.
	 *
	 * The disable status from oldRefTime is set for each requestedUpdate. This is to preserve
	 * the disable status from between update request.
	 *
	 * @return true if all requested providers is defined in the providerlist.
	 *         false if one of the requested providers is not defined in the providerList.
	 *         The providers that is not defined is marked with dataversion == INT_MAX.
	 */
	bool checkProviders( const ProviderList &providerList,
			             const ProviderRefTimeList &oldRefTime,
			             ProviderRefTimeList &requestedUpdate )const;
	void checkAgainstExistingProviders( const ProviderRefTimeList &exitingProviders, 
			                              ProviderRefTimeList &newRefTime ) const;
	std::string statusDoc( const std::string &status, const std::string &comment );
};

}


#endif
