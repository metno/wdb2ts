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
#ifndef __REQUESTHANDLERMANAGER_H__
#define __REQUESTHANDLERMANAGER_H__

#include <stdexcept>
#include <RequestHandler.h>

namespace webfw {

class RequestHandlerManager 
{   
   protected:

      /**
       * @exceptions ResourceError
       */ 
      virtual RequestHandlerPtr 
              findRequestHandlerPathImpl( const std::string &path, 
                                          int majorVersion, int minorVersion )=0;
      
   public:
      RequestHandlerManager();

      /**
       * Decode and remove the version part of the path.
       * 
       * A path is on the form:
       * 
       *   /path/to/something/major.minor   major and minor is the version part.
       * 
       * Major and minor must be numers.
       * 
       * If the version part is missing \e major and \e minor is set to 0.
       * 
       * On return newPath is set to path, but the version part is stripped out.
       * 
       * @param path to decode.
       * @param newPath, the path stripped for the version part.
       * @param[out] major the major part of the version.
       * @param[out] minor the minor part of the version.
       * @exception webfw::InvalidPath
       */
      static void decodePath( const std::string &path, 
                              std::string &newPath, int &major, int &minor ) ;


      /**
       * @exceptions webfw::ResourceError
       */      
      virtual bool addRequestHandler( RequestHandler *reqHandler,
                                      const std::string &path )=0;
      
      
      /**
       * @exceptions webfw::ResourceError
       */                       
      virtual bool removeRequestHandler( const std::string &path, int major, int minor )=0;
           
      
      /**
       * @exceptions webfw::ResourceError, miServerPage::NotFound
       */
      virtual bool setDefaultRequestHandler( const std::string &path, int major, int minor )=0;

      /**
       * Search for a request handler given a path. 
       * 
       * The it first decodes the version part from the path. Then the
       * path is searched recursive until a RequestHandler is found.
       * If  no request handler is found a NotFound exception is 
       * thrown.
       * 
       * The serach:
       * 
       *    path /metno-ts2xml/data/Punkdata. 
       * 
       *    It search in order for 
       *    - /metno-ts2xml/data/Punkdata
       *    - /data/Punkdata
       *    - /Punkdata  
       *  
       * It reurns the first that is found.
       * 
       * 
       * @exceptions webfw::ResourceError, webfw::NotFound, 
       *             webfw::InvalidPath
       */
      RequestHandlerPtr findRequestHandlerPath( const std::string &path );
};

}
#endif
