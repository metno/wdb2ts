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
#include <cctype>
#include <stdio.h>
#include <readfile.h>
#include <sstream>
#include <splitstr.h>
#include <trimstr.h>
#include <DefaultRequestHandlerManager.h>
#include <App.h>

using namespace std;

webfw::   
App::
App( RequestHandlerManager *reqHandlerMgr )
   :requestHandlerManager_( reqHandlerMgr ), abortHandlerManager_( 0 )
{
}

webfw::   
App::
App( RequestHandlerManager *reqHandlerMgr, IAbortHandlerManager *abortHandlerMgr )
	:requestHandlerManager_( reqHandlerMgr ), abortHandlerManager_( abortHandlerMgr )
{
}

webfw::
App::
App()
   : requestHandlerManager_( new DefaultRequestHandlerManager() ),
     abortHandlerManager_( 0 )
   
{
}

void 
webfw::
App::
initAction( RequestHandlerManager&  reqHandlerMgr, Logger &logger )
{
}

void 
webfw::
App::
onShutdown()
{
}

void
webfw::
App::
init( Logger &logger, IAbortHandlerManager *abortHandlerMgr )
{
	if( ! abortHandlerManager_ ) 
		abortHandlerManager_ = abortHandlerMgr;
	
	if( requestHandlerManager_ )
		initAction( *requestHandlerManager_, logger );
}


void
webfw::
App::
setPathsFromConffile( const char *confpath, const char *logpath, const char *tmppath )
{
	boost::mutex::scoped_lock locl( mutex );

	if( confpathFromConffile_.empty() )
		confpathFromConffile_ = confpath;

	if( logpathFromConffile_.empty() )
		logpathFromConffile_ = logpath;

	if( tmppathFromConffile_.empty() )
		tmppathFromConffile_ = tmppath;
}
   
void 
webfw::
App::
setConfDir( const std::string &confdir )
{
	boost::mutex::scoped_lock locl( mutex );
	confdir_ = confdir;
}

std::string
webfw::
App::
getConfDir()const
{
	 boost::mutex::scoped_lock locl( mutex );

	return confpathFromConffile_.empty() ? confdir_ : confpathFromConffile_;
}


void 
webfw::
App::
setLogDir( const std::string &logdir )
{
	logdir_ = logdir;
}

std::string
webfw::
App::
getLogDir()const
{
	boost::mutex::scoped_lock locl( mutex );

	return logpathFromConffile_.empty() ? logdir_ : logpathFromConffile_;
}

void
webfw::
App::
setTmpDir( const std::string &tmpdir )
{
	boost::mutex::scoped_lock locK( mutex );

	tmpdir_ = tmpdir;
}

std::string
webfw::
App::
getTmpDir()const
{
	boost::mutex::scoped_lock locK( mutex );

	return tmppathFromConffile_.empty() ? tmpdir_ : tmppathFromConffile_;
}



bool
webfw:: 
App::
readConfFile( const std::string &confile, std::string &content )
{
   string file( confile );
   string path=getConfDir();
   
   if( ! path.empty() )
      file = path + "/" + confile;
     
   cerr << "readConfFile: " << file << endl;
   return miutil::readFile( file, content );
}
   
   
   
void 
webfw::
App::
dispatch( Request &req, Response &res, Logger &logger )
{
   RequestHandlerPtr reqHandler;
   
   try{
      reqHandler = requestHandlerManager_->findRequestHandlerPath( req.urlPath() );
   }
   catch( const NotFound& ex ) {
      res.status( Response::NOT_FOUND );
      res.errorDoc( "Path NOT found: <" + req.urlPath() + ">"  );
      return;
   }
   catch( const InvalidPath& ex ) {
      res.status( Response::INVALID_PATH );
      res.errorDoc( "Inavlid path (format error): <" + req.urlPath() + ">"  );
      return;
   }
   catch( const ResourceError& ex ) {
      res.status( Response::INTERNAL_ERROR );
      res.errorDoc( "Unknown error in the lookup of an RequestHandler: <" + req.urlPath() + ">"  );
      return;
   }
   
   ostringstream ost;
   int minor, major;
   reqHandler->version( major, minor );
    
   ost << reqHandler->name() <<" version: " << major << "." << minor;
   logger.info( "Handler: " + ost.str() );
   res.aboutRequestHandler( ost.str() );
   
   RequestHandler *myReqHandler = reqHandler.get();
   
   try{
      switch( req.requestType() ) {
         case Request::GET:    myReqHandler->doGet( req, res, logger, this );  break;
         case Request::PUT:    myReqHandler->doPut( req, res, logger, this );  break;
         case Request::POST:   myReqHandler->doPost( req, res, logger, this ); break;
         case Request::DELETE: myReqHandler->doDel( req, res, logger, this );  break;
         default:
            res.status( Response::NOT_SUPPORTED );
            return; 
      }
   }
  catch( ... ) {
      res.status( Response::INTERNAL_ERROR );
      res.errorDoc( "Uncaught exception from a request handler: <" + req.urlPath() + ">"  );
  }
}

std::map<std::string, int> 
webfw::
decodeLogLevels( const std::string &logLevels )
{
	int nLevel;
	string level;
	string loggerName;
	map<std::string, int> loggers;
	vector<std::string> logger;
	vector<std::string> keyVals=miutil::splitstr( logLevels, ';' );
	
	for( vector<string>::size_type i=0; i < keyVals.size(); ++i ) {
		logger = miutil::splitstr( keyVals[i], '=' );
		if( logger.size() != 2 )
			throw logic_error("Invalid format of the loglevel '" + keyVals[i] + "'.");
		
		loggerName = logger[0];
		level = logger[1];
		
		miutil::trimstr( loggerName );
		miutil::trimstr( level );
		
		if( loggerName.empty() )
			throw logic_error("Invalid format of the loglevel '" + keyVals[i] + "'. No logger name.");
		
		if( level.empty() )
			throw logic_error("Invalid format of the loglevel '" + keyVals[i] + "'. No log level.");
		
		if( isdigit( level[0]) ) {
			if( sscanf( level.c_str(), "%d", &nLevel) != 1 )
				throw logic_error("Invalid format of the loglevel '" + keyVals[i] + "'. Log level must be a number or one of"+
						"emerg (0), fatal (1), alert (2), crit (3), error (4), warn (5), notice (6), info (7) or debug(8).");
		} else {
			if( level== "emerg" )
				nLevel=0;
			else if( level== "fatal" )
				nLevel = 1;
			else if( level== "alert" )
				nLevel=2;
			else if( level== "crit" )
				nLevel=3;
			else if( level== "error")
				nLevel=4;
			else if( level== "warn" )
				nLevel=5;
			else if( level== "notice")
				nLevel=6;
			else if( level== "info")
				nLevel=7;
			else if( level== "debug")
				nLevel=8;
			else {
				throw logic_error("Invalid format of the loglevel '" + keyVals[i] + "'. Log level must be a number or one of"+
									"emerg (0), fatal (1), alert (2), crit (3), error (4), warn (5), notice (6), info (7) or debug(8).");
			}
		}

		if( nLevel < 0 || nLevel > 8 )
			throw range_error("Invalid loglevel '" + keyVals[i] + "'. Valid range [0,8]. (emerg (0), fatal (1), alert (2), crit (3), error (4), warn (5), notice (6), info (7) or debug(8))" );
		
		loggers[ loggerName ] = nLevel;
	}
	
	return loggers;
}

