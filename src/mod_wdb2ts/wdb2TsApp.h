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

#ifndef __WDB2TSAPP_H__
#define __WDB2TSAPP_H__

#include <limits.h>
#include <boost/thread/thread.hpp>
#include <App.h>
#include <DbManager.h>
#include <PointDataHelper.h>
#include <UpdateProviderReftimes.h>
#include <map>
#include <vector>
#include <Map.h>
#include <Config.h>
#include <NoteManager.h>

namespace wdb2ts {


class Wdb2TsApp : public webfw::App 
{
	MISP_DECLARE_APP( Wdb2TsApp );

	void initHightMapImpl();
	
public:
	/// Default Constructor
	Wdb2TsApp();
	
	/** 
	 * Module Name
	 * @return 	Name of the module
	 */
	virtual char *moduleName()const { return "metno-wdb2ts"; }
	
	NoteManager notes;
      
	/**
     * @exception logic_error on failure.
     */
	miutil::pgpool::DbConnectionPtr newConnection(const std::string &dbid="");

	/**
	 * @exception logic_error on failure.
	 */
	WciConnectionPtr newWciConnection(const std::string &dbid="");

	
	ParamDefList &paramDefs() { return paramDefs_; }
    
    
    
	/**
	 * Return the hight for a given position.
	 * 
	 * INT_MIN is returned if there is no hight value
	 * for the position.
	 * 
	 * @exception InInit if the hightMap is under construction.
	 * @exception logic_error There is no hightMap.
	 */
	int getHight( float latitude, float longitude );
    
	void initHightMap();
	
	/**
	 * Return WCI protocol version.
	 * 
	 * Defined versions:
	 *   WDB 0.7.x  ==> 1
	 *   WDB 0.8.x  ==> 2
	 *   > WDB 0.8  ==> 2
	 * 
	 * Default version is 1 if the version number from the database 
	 * is not on the form x.y.z;  x, y and z is numbers. -1 is returned
	 * if an exception from the database is detected.
	 * 
	 * @return The protocol version > 0 on success and -1 on failure.  
	 */
	int wciProtocol( const std::string &wdbid );

    
protected:
	
	virtual void initAction( webfw::RequestHandlerManager& reqHandlerMgr,
                             webfw::Logger &logger );
	
	void readConfiguration( webfw::RequestHandlerManager&  reqHandlerMgr,
			                  webfw::Logger &logger );
	
	void configureRequestsHandlers( wdb2ts::config::Config *config,
			                          webfw::RequestHandlerManager&  reqHandlerMgr,
				                       webfw::Logger &logger );
	
   DbManagerPtr  dbManager;
 	ParamDefList  paramDefs_;
 	Map           *hightMap;
 	bool          initHightMapTryed;
 	bool          inInitHightMap;
 	
 	///Serialize access to the hightMap
 	boost::mutex  mapMutex;
 	

 	
 	///Serialize access to an instance of this object.
 	boost::mutex  mutex;

};

}

#endif // __WDB2TSAPP_H__ 
