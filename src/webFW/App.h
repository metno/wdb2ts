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
#ifndef __APP_H__
#define __APP_H__


#include <boost/thread/thread.hpp>
#include <stdexcept>
#include <macros.h>
#include <exception.h>
#include <Logger.h>
#include <Request.h>
#include <Response.h>
#include <IAbortHandlerManager.h>
#include <RequestHandlerManager.h>

namespace webfw {

/**
 * @addtogroup webFWAP
 * @{
 * 
 * App is a singelton class that holds the globale state of
 * this instance of the application.
 */
class App 
{
   RequestHandlerManager *requestHandlerManager_;
   IAbortHandlerManager  *abortHandlerManager_;
   std::string confpathFromConffile_;
   std::string logpathFromConffile_;
   std::string tmppathFromConffile_;
   std::string confdir_;
   std::string logdir_;
   std::string tmpdir_;
   
   mutable boost::mutex mutex;

   friend class RequestHandler;
   
   IAbortHandlerManager *abortHandler(){ return abortHandlerManager_; } 
   
   protected:
      RequestHandlerManager&  requestHandlerManager(); 
      virtual void initAction( RequestHandlerManager&  reqHandlerMgr,
                               Logger &logger );
      
   public:
      App( RequestHandlerManager *regHandlerMgr );
      App( RequestHandlerManager *regHandlerMgr, IAbortHandlerManager *abortHandlerMgr );
      App();

      /**
       * Set the path from the modulename_set_confpath variable in the configuration file.
       *
       * modulename is the name of the module.
       */
      void setPathsFromConffile( const char *confpath,
    		                     const char *logpath,
    		                     const char *tmppath );

      /**
       * All configuration files is read from the directory
       * set here as default.
       * 
       * @param confdir The directory the configurations files resides in.
       */
      void setConfDir( const std::string &confdir );
      
      /**
       * getConfDir returns the confpathFromConfile if it is not empty.
       * If it is empty the confdir_ set with setConfDir is used.
       */
      std::string getConfDir()const;
      
      /**
       * Write logfiles to this directory.
       * 
       * @param logdir The directory to write logfiles
       */
      void setLogDir( const std::string &logdir );
      
      std::string getLogDir()const;
      
      /**
       * Write temporary files to this directory.
       *
       * @param tmpdir The directory to write temporary files to.
       */
      void setTmpDir( const std::string &tmpdir );

      std::string getTmpDir()const;


      
      /**
       * Read the contents of a configuration file into a string. The
       * file must be in the directory set by a prevously call to setConfDir.
       * 
       * @param confile The configuration file to read. The file must be
       *  in the directory set by setConfDir.
       * @param[out] content The string to hold the content of the file.
       * @return true if the file existed an is read into content.
       */
      bool readConfFile( const std::string &confile, std::string &content );

      ///This is a private init function do not call,
      void init( Logger &logger, IAbortHandlerManager *abortHandlerMgr );
      
      /**
       * The name this module is known as.
       */
      virtual const char *moduleName()const =0;
      
      
      void dispatch( Request &req, Response &res, Logger &logger );
};

/**
 * decodeLogLevels, decodes a string with loglevels on the form.
 * default=l;logger1=l;logger2=l; ... ;loggerN=l"
 * 
 * Where:
 *  - default is the default level used for a logger if no loglevel is specified
 *    for the logger.
 *  - logger is the name of the logger, ex db.
 *  - l is the logger level. A value in the range 0-8.
 * 
 * Loglevel:
 * 	- 0 EMERG, 
 *  - 1 FATAL
 *  - 2 ALERT
 *  - 3 CRIT
 *  - 4 ERROR
 *  - 5 WARN
 *  - 6 NOTICE
 *  - 7 INFO
 *  - 8 DEBUG
 *
 * @param logLevels the string to decode.
 * @return a map witch associate logger with a loglevel.
 * @throws std::range_error if an loglevel not in the range 0-8.
 * @throws std::logic_error if the format is not valid. 
 */
std::map<std::string, int> decodeLogLevels( const std::string &logLevels );


/** @} */
}


#endif 
