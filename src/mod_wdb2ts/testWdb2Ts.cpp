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
#include <sstream>
#include <ptimeutil.h>
#include <TestRequest.h>
#include <DefaultResponse.h>
#include <CErrLogger.h>
#include <macros.h>
#include <wdb2TsApp.h>


using namespace std;
using namespace wdb2ts;
using namespace webfw;

void
splitUrl( const char *url, string &dirPart, string &queryPart );

int
main( int argn, char **argv )
{
   ostringstream ost;
   CErrLogger logger;
   string content_type;
   string method("GET");
   string sexpire;
   string dirPart, queryPart;
   
   MISP_CREATE_AND_INIT_APP( wdb2ts::Wdb2TsApp, logger, 0 );
   
   /*
    * http://server/path/?lat=10;lon=10;alt=10;
    *   reftime=2007-12-10T10:00,2007-12-10T10:00,exact;
    *   dataprovider=1096;
    *   dataversion=-1;
    *   parameter=instantaneous pressure of air,instantaneous temperature of air,instantaneous velocity of air (u-component);
    *   levelspec=2,2,above ground,exact;
    *   validtime=2007-12-10T00:00,2007-12-10T10:00,intersect
    *   format=CSV
    */
   if( argn > 1 ) {
   	splitUrl( argv[1], dirPart, queryPart );
   } else {
   	dirPart = "/metno-wdb2ts/location";
   	ost << "lat=59;lon=10;alt=10;dataprovider=hirlam 10;"
   		 << "reftime=2008-05-25T06:00:00;"
   		 << "parameter=instant pressure of air,"
   		 <<            "instant temperature of air,"
   		 <<            "instant velocity of air (u-component);";
   	queryPart = ost.str();
   }
   
   TestRequest   request( Request::GET, dirPart, queryPart );    
   DefaultResponse response;         
   
   Wdb2TsApp*   myApp = Wdb2TsApp::app();         
                                                     
   if( ! myApp )  {
      cerr << "Cant get the AppClass!" << endl;
      return 1;           
   } 

   cerr << "Request: " <<
           "  path:  "  << request.urlPath() << "\n" << 
           "  query: "  << request.urlQuery() << "\n";
   
   myApp->dispatch( request, response, logger );   

   std::string error( response.errorDoc() );       
   ost.str("");   
                     
   
     
   switch (response.status()) {                   
      case webfw::Response::PUT_NOT_SUPPORTED:    
      case webfw::Response::GET_NOT_SUPPORTED:    
      case webfw::Response::POST_NOT_SUPPORTED:   
      case webfw::Response::DELETE_NOT_SUPPORTED: 
      case webfw::Response::NOT_SUPPORTED:        
           ost << method << ": Not supported!";
           logger.error( ost.str() );
           return 1;
        case webfw::Response::NOT_FOUND:            
           if( error.empty() )
              ost << "Path NOT found: " << request.urlPath();
           else
              ost << error;
           logger.info( ost.str() );
           return 1;                          
        case webfw::Response::NO_DATA:
           if( error.empty() )
              ost << "NO DATA: " << request.urlPath();
           else
              ost << error;
           return 1;
        case webfw::Response::INVALID_PATH:
           if( error.empty() )
              ost << "NO DATA: " << request.urlPath();
           else
              ost << error;
           return 1;
        case webfw::Response::INVALID_QUERY:
           if( error.empty() )
              ost << "NO DATA: " << request.urlQuery();
           else
              ost << error;
           return 1;
        case webfw::Response::INTERNAL_ERROR:
           if( error.empty() )
              ost << "Unknown INTERNAL ERROR" << request.urlPath();
           else
              ost << error;
           return 1;
        case webfw::Response::NO_ERROR:
           /*Nothing to do here.*/
           break;
        default:
           if( error.empty() )
              ost << "Unknown Response from the request handler!" << request.urlPath();
           else
              ost << error;
           return 1;
     }
                                                           
     cout << response.content() << endl;
     
     cerr << "\n" << "Error: " << error << "\n";
}


void
splitUrl( const char *url_, string &dirPart, string &queryPart )
{
	string  url( url_ );
	
	string::size_type i = url.find("?");
	
	if( i == string::npos ) { 
		dirPart = url;
		queryPart.erase();
		cerr << "url: " << url << endl
		     << "WARNING: No query part." << endl;
		return;
	}
	
	dirPart = url.substr( 0, i );
	queryPart = url.substr( i+1 );
	
	cerr << "Url: " << url << endl
	     << "dirPart  : " << dirPart << endl
	     << "queryPart: " << queryPart << endl;
}
