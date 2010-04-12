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
#ifndef __REQUESTHANDLER_H__
#define __REQUESTHANDLER_H__

#include <map>
#include <log4cpp/Category.hh>
#include <boost/shared_ptr.hpp>
#include <exception.h>
#include <Logger.h>
#include <Response.h>
#include <Request.h>

namespace webfw {

class App;
class IAbortHandler;

class RequestHandler 
{
   int majorVersion_;
   int minorVersion_; 

   std::string logDir;
   std::string tmpDir;
   std::string confDir;
   std::string logprefix;
   std::string moduleName;
  
   void doGet( Request &req, Response &response, Logger &logger, App *app );
   void doPost( Request &req, Response &response, Logger &logger, App *app );
   void doPut( Request &req, Response &response, Logger &logger, App *app );
   void doDel( Request &req, Response &response, Logger &logger, App *app );

   friend class App;
   
  protected:
	std::map< std::string, int > logLevels;  
   void setVersion( int &major, int &minor );
   
  public:
      RequestHandler();
      RequestHandler( int major, int minor )
         : majorVersion_( major ), minorVersion_( minor )
      {}
      
      virtual ~RequestHandler();
   
      virtual const char *name()const =0;
   
      void version( int &major, int &minor )const;
      bool isVersion( int major, int minor )const;

      void setPaths( const std::string &confpath,
          		     const std::string &logpath,
          		     const std::string &tmppath );

      std::string getLogDir()const  { return logDir;}
      std::string getConfDir()const { return confDir; }
      std::string getTmpDir()const  { return tmpDir; }

      void setLogprefix( const std::string &prefix ) { logprefix= prefix; }
      std::string getLogprefix()const { return logprefix; }

      void setModuleName( const std::string &mname ) { moduleName = mname; }
      std::string getModuleName()const { return moduleName; }


      /**
       * register an abort handler to be called if the user abort an 
       * operation. The handlers is automaticaly removed when the put,
       * get, etc operations returns. You only need to remove a handler manualy
       * if you decide it is not needed anymore.
       * 
       * @return An handlerid that later can be used to remove the handler.
       */
      unsigned long registerAbortHandler( IAbortHandler *handler );
      void removeAbortHandler( unsigned long handlerId );
      
      virtual void setLogLevels( const std::map<std::string, int > &loglevels );
               int getLogLevel( const std::string &name )const;
      virtual void get( Request &req, Response &response, Logger &logger );
      virtual void post( Request &req, Response &response, Logger &logger );
      virtual void put( Request &req, Response &response, Logger &logger );
      virtual void del( Request &req, Response &response, Logger &logger );
      
      static const RequestHandler *getRequestHandler();
      static log4cpp::Category& getLogger( const std::string &name, const RequestHandler *reqHandler=0 );
};

typedef boost::shared_ptr<RequestHandler> RequestHandlerPtr;

}
#endif 
