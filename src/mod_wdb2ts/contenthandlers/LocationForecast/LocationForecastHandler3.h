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
#ifndef __LOCATION_FORECAST_HANDLER_3_H__
#define __LOCATION_FORECAST_HANDLER_3_H__

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/shared_ptr.hpp>
#include <RequestHandler.h>
#include <PointDataHelper.h>
#include <WdbQueryHelper.h>
#include <HandlerBase.h>
#include <UpdateProviderReftimes.h>
#include <INoteUpdateListener.h>
#include <LocationData.h>
#include <SymbolConfConfigure.h>
#include <SymbolGenerator.h>
#include <MetaModelConf.h>
#include <ProjectionHelperConfigure.h>
#include <PrecipitationConfig.h>
#include <TopoProvider.h>
#include <NearestHeight.h>
#include <NearestLand.h>
#include <NoteProviderList.h>
#include <LocationPoint.h>
#include <ProviderGroups.h>
#include <ConfigUtils.h>
#include <WebQuery.h>
#include <QueryMaker.h>

namespace wdb2ts {

class Wdb2TsApp;

/**
 * The LocationForecastHandler takes care of requests on the form:
 * \em http://server/metno-wdb2ts/locationforecast
 */
class LocationForecastHandler3 :
	public HandlerBase,
	public INoteUpdateListener
{ 
public:
	/** Default Constructor */
	LocationForecastHandler3();
	/** Constructor 
	 * @param	major
	 * @param	minor 
	 */
	LocationForecastHandler3( int major, int minor, const std::string &note="" );
	/** Destructor */
	~LocationForecastHandler3();
	/** Identify the RequestHandler
	 * @returns	The name of the Request Handler
     */
	virtual const char *name()const { return "LocationForecastHandler2"; };

	void configureWdbProjection( const wdb2ts::config::ActionParam &params, wdb2ts::Wdb2TsApp *app );
	NoteProviderList* configureProviderPriority( const wdb2ts::config::ActionParam &params, Wdb2TsApp *app );
	NoteProviderList* doExtraConfigure(  const wdb2ts::config::ActionParam &params, Wdb2TsApp *app );

	/**
	 * extraConfigure does some late configuration that is not posible at 
	 * startup since the database subsystem is not ready.
	 * 
	 * The extra configuration that takes place is configuration of
	 * the projections (configureWdbProjection) and which wci protocol to use.
	 */
	void extraConfigure( const wdb2ts::config::ActionParam &params, Wdb2TsApp *app ); 
	
	///Overide IConfigure::configure
	virtual bool configure( const wdb2ts::config::ActionParam &params,
			                  const wdb2ts::config::Config::Query &query,
						         const std::string &wdbDB);
	
	///Overide webfw::RequestHandler::get
	virtual void get( webfw::Request  &req, 
                     webfw::Response &response, 
                     webfw::Logger   &logger    );

	///Overide INoteUpdateListener::noteUpdated
	virtual void noteUpdated( const std::string &noteName, 
                             boost::shared_ptr<NoteTag> note );
	
private:
	SymbolGenerator     symbolGenerator;
	std::string         updateid; //A namspace for notes to the LocationForecastUpdateHandler.
	std::string         note;
	int                 subversion;
	std::string         wdbDB;
   //WdbQueryHelper      *wdbQueryHelper;
   wdb2ts::config::Config::Query urlQuerys;
   wdb2ts::config::ActionParam actionParams;
   ProviderList        providerPriority_;
   bool                providerPriorityIsInitialized;
   TopoProviderMap     modelTopoProviders;
   std::list<std::string> topographyProviders;
   NearestHeights      nearestHeights;
   NearestLandConf     nearestLands;
   SymbolConfProvider  symbolConf_;
   PtrProviderRefTimes providerReftimes;
   MetaModelConfList   metaModelConf;  
   ProjectionHelper    projectionHelper;
   ParamDefListPtr     paramDefsPtr_;
   bool                projectionHelperIsInitialized;
   ProviderPrecipitationConfig *precipitationConfig;
   bool                wciProtocolIsInitialized;
   int                 wciProtocol;
   int                 expireRand; //Randomize the expire header. Default 120s.
   OutputParams        doNotOutputParams;
   NoDataResponse      noDataResponse;
   wdb2ts::qmaker::QueryMakerPtr queryMaker;

   boost::mutex        mutex; 
      
   LocationPointDataPtr  requestWdbDefault( const WebQuery &webQuery,
		   	   	   	                        int altitude,
		   	   	   	                        PtrProviderRefTimes refTimes,
		   	   	   	                        ParamDefListPtr     paramDefsPtr,
		   	   	   	                        const ProviderList  &providerPriority
             	 	 	 	 	 	 	 ) const;

   LocationPointDataPtr  requestWdbSpecific( const WebQuery &webQuery,
		   	   	   	                         int altitude,
		   	   	   	                         const PtrProviderRefTimes refTimes,
		   	   	   	                         ParamDefListPtr     paramDefsPtr,
		   	   	   	                         const ProviderList  &providerPriority
             	 	 	 	 	 	 	    ) const;


  	LocationPointDataPtr requestWdb( const LocationPointList &locationPoints,
  	                                 const boost::posix_time::ptime &to,
  	                                 bool isPolygon, int altitude,
  	                                 PtrProviderRefTimes refTime,
  	                                 ParamDefListPtr  paramDefs,
  		                             const ProviderList &providerPriority )const;
  	
  	/*
  	bool updateProviderReftimes( WciConnectionPtr con );
  	*/
  	
  	PtrProviderRefTimes getProviderReftimes();
  	
  	//Get some mutex protected data.
  	void getProtectedData( SymbolConfProvider &symbolConf, ProviderList &providerList, ParamDefListPtr &paramDefsPtr );
  	
  	void
  	nearestHeightPoint( const LocationPointList &locationPoints,
						const boost::posix_time::ptime &to,
  			            LocationPointDataPtr data,
  			            int altitude,
  			            PtrProviderRefTimes refTimes,
  			            ParamDefListPtr paramDefs,
  			            const ProviderList &providerPriority
  					  ) const;

  	void
  	nearestLandPoint( const LocationPointList &locationPoints,
  	                  const boost::posix_time::ptime &to,
  	                  LocationPointDataPtr data,
  	                  int altitude,
  	                  PtrProviderRefTimes refTimes,
  	                  ParamDefListPtr params,
  	                  const ProviderList &providerPriority
  	                ) const;

   
};

}


#endif
