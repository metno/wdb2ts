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
#ifndef __LOCATIONHANDLER_H__
#define __LOCATIONHANDLER_H__

#include <boost/thread/thread.hpp>
#include <RequestHandler.h>
#include <HandlerBase.h>
#include <wdb2TsApp.h>

namespace wdb2ts {

/**
 * Handel location request.
 *  \em http://server/metno-wdb2ts/location
 */
class LocationHandler:
	public HandlerBase
{ 
   public:
      LocationHandler( int major, int minor );
      ~LocationHandler();
   
   virtual const char *name()const { return "LocationHandler"; };

   virtual bool configure( const wdb2ts::config::ActionParam &params,
   		                  const wdb2ts::config::Config::Query &query,
   					         const std::string &wdbDB);
   
   /**
    * extraConfigure does some late configuration that is not posible at 
    * startup since the database subsystem is not ready.
    * 
    * The extra configuration that takes place is configuration of
    * the wci protocol to use.
    */
   void extraConfigure( const wdb2ts::config::ActionParam &params, Wdb2TsApp *app ); 
   
   virtual void get( webfw::Request  &req, 
                     webfw::Response &response, 
                     webfw::Logger   &logger );
   
   private:
   	std::string  wdbDB;
      bool         wciProtocolIsInitialized;
      int          wciProtocol;
      boost::mutex mutex;
      wdb2ts::config::ActionParam actionParams;
   
};

}


#endif
