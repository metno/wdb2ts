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
#include <contenthandlers/Location/LocationHandler.h>
#include <replace.h>
#include <exception.h>
#include <WciWebQuery.h>
#include <contenthandlers/Location/EncodeCSV.h>
#include <wdb2TsApp.h>
#include <Logger4cpp.h>

using namespace std;

namespace wdb2ts {

LocationHandler::
LocationHandler( int major, int minor )
   : HandlerBase( major, minor), wciProtocolIsInitialized( false )
{
}

LocationHandler::
~LocationHandler()
{
} 

bool 
LocationHandler::
configure( const wdb2ts::config::ActionParam &params,
		     const wdb2ts::config::Config::Query &query,
			  const std::string &wdbDB_ )
{
	wdbDB = wdbDB_;
	actionParams = params;

	return true;
}
    

void
LocationHandler::
extraConfigure( const wdb2ts::config::ActionParam &params, Wdb2TsApp *app ) 
{
	WEBFW_USE_LOGGER( "handler" );
	boost::mutex::scoped_lock lock( mutex );
	try {
		if( ! wciProtocolIsInitialized ) {
			wciProtocol = app->wciProtocol( wdbDB );
		
			WEBFW_LOG_INFO("WCI protocol: " << wciProtocol );
		
			if( wciProtocol > 0 )
				wciProtocolIsInitialized = true;
			else
				wciProtocol = 1;
		}
	}
	catch( const std::exception &ex) {
		WEBFW_LOG_ERROR( "EXCEPTION: extraConfigure: " << ex.what() );
	}
	catch( ... ) {
		WEBFW_LOG_ERROR("EXCEPTION: extraConfigure: Unknown exception! " );
	}
}


void 
LocationHandler::
get( webfw::Request &req, 
     webfw::Response &response, 
     webfw::Logger & )
{
     ostringstream ost;
     ostream &out = response.out();
     response.contentType("text/plain");
     response.directOutput( true );

     WEBFW_USE_LOGGER( "handler" );
     WEBFW_LOG_DEBUG("LocationHandler: Query: " << req.urlQuery() );
     
     Wdb2TsApp *app=Wdb2TsApp::app();
     
     extraConfigure( actionParams, app );

     WciWebQuery webQuery( wciProtocol );
     
     std::string queryIn( req.urlQuery() );
     

     try {
   	  webQuery.decode( queryIn );
   	  
   	  if( webQuery.reftime.fromTime.is_special() && webQuery.validtime.fromTime.is_special() ) {
   		  ost.str("");
   		  ost << "Invalid qurey, reftime or validtime must be different from 'NULL'. Query: "  
   		      << req.urlQuery();
   		  WEBFW_LOG_ERROR( ost.str() );
   		  response.errorDoc( ost.str() );
   		  response.status( webfw::Response::INVALID_QUERY );
   		  return;
   	  }
     }
     catch( const logic_error &ex ){
   	  ost.str("");
   	  ost << "Exception, decode query: " << ex.what() << ": Query: " << req.urlQuery();
   	  WEBFW_LOG_ERROR( ost.str() );
   	  response.errorDoc( ost.str() );
   	  response.status( webfw::Response::INVALID_QUERY );
   	  return;
     }
     catch( const std::exception &ex ) {
   	  ost.str("");
   	  ost << "Exception, decode query: " << ex.what() << ": Query: " << req.urlQuery();
   	  WEBFW_LOG_ERROR( ost.str() );
   	  response.errorDoc( ost.str() );
   	  response.status( webfw::Response::INVALID_QUERY );
   	  return;
     }
     catch( ... ) {
   	  ost.str("");
   	  ost << "Unknown Exception, decode query: Query: " << req.urlQuery();
   	  WEBFW_LOG_ERROR( ost.str() );
   	  response.errorDoc( ost.str() );
   	  response.status( webfw::Response::INVALID_QUERY );
   	  return;
     }

     try{
    	 WciConnectionPtr con = app->newWciConnection( wdbDB );
    	 EncodeCSV encode( webQuery, con, wciProtocol );
    	 encode.encode( out );
     }
     catch( const webfw::IOError &ex ) {
    	 WEBFW_LOG_ERROR( "Exception: " << ex.what() );
    	 response.errorDoc( ex.what() );
        
        if( ex.isConnected() )
           response.status( webfw::Response::INTERNAL_ERROR );
     }
     catch( const std::ios_base::failure &ex ) {
    	 WEBFW_LOG_ERROR( "Exception: " << ex.what() );
    	 response.errorDoc( ex.what() );
    	 response.status( webfw::Response::INTERNAL_ERROR );
     }
     catch( const logic_error &ex ){
    	 WEBFW_LOG_ERROR( "Exception: " << ex.what() );
    	 response.errorDoc( ex.what() );
    	 response.status( webfw::Response::INTERNAL_ERROR );
    	 return;
     }
     catch( const exception &ex ){
    	 WEBFW_LOG_ERROR( "Exception: " << ex.what() );
    	 response.errorDoc( ex.what() );
    	 response.status( webfw::Response::INTERNAL_ERROR );
    	 return;
     }
     catch( ... ) {
    	 WEBFW_LOG_ERROR( "Unknown Exception." );
    	 response.errorDoc("Unexpected exception!");
    	 response.status( webfw::Response::INTERNAL_ERROR );
     }
}

}
