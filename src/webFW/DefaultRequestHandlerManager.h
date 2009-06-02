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
#ifndef __TESTREQUESTHANDLERMANAGER_H__
#define __TESTREQUESTHANDLERMANAGER_H__

#include <string>
#include <map>
#include <set>
#include <RWMutex.h>
#include <RequestHandlerManager.h>
#include <RequestHandler.h>

namespace webfw {

/**
 * DefaultRequestHandlerManager is used as a default RequestHandler.
 */

class DefaultRequestHandlerManager : public RequestHandlerManager
{
   struct Version
   {
      int verMajor;
      int verMinor;
      
      Version( int major, int minor );
      Version( const Version &v );
      Version();
      
      bool operator<(const Version &rhs)const;
      Version& operator=(const Version &rhs);
   };


   miutil::thread::RWMutex rwMutex;   
   std::map<std::string, std::map<Version, RequestHandlerPtr> > handlers;  

   protected:
      RequestHandlerPtr findRequestHandlerPathImpl( const std::string &path, 
                                                    int majorVersion, int minorVersion );
      
   public:
      DefaultRequestHandlerManager();
   
      void addRequestHandler( RequestHandler *reqHandler,
                              const std::string &path );
      
      
      void removeRequestHandler( const std::string &path, int major, int minor );
      
      void setDefaultRequestHandler( const std::string &path, int major, int minor );
};

}
#endif
