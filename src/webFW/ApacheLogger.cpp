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

#include <iostream>
#include <httpd.h>
#include <http_config.h>
#include <http_protocol.h>
#include <apr_optional.h>
#include <ap_config.h>
#include <http_log.h>
#include <ApacheLogger.h>


using namespace std;

webfw::
ApacheLogger:: 
ApacheLogger(  request_rec *request_, const std::string &modName  )
   :  request( request_ ), pool( 0 ), moduleName( modName )
{
}

webfw::
ApacheLogger::
ApacheLogger( apr_pool_t *pool_, const std::string &modName )
   : request( 0 ), pool( pool_ ), moduleName(modName)
{
}

webfw::
ApacheLogger:: 
~ApacheLogger()
{
}
      
void 
webfw::
ApacheLogger:: 
log( LogLevel ll, const std::string &msg )const
{  
#ifdef APLOG_USE_MODULE
APLOG_USE_MODULE(moduleName);
#endif
   int l;
   
   switch( ll ) {
      case Fatal:   l = APLOG_CRIT;    break;
      case Error:   l = APLOG_ERR;     break;
      case Warning: l = APLOG_WARNING; break;
      case Info:    l = APLOG_INFO;    break;
      case Debug: 
      case Debug1:
      case Debug2:  l =APLOG_DEBUG;    break;
      default:
         l = APLOG_DEBUG;
   }
   
   bool endl=true;

   if(!msg.empty() && msg[msg.length()-1]!='\n')
	   endl=false;

   if( request ) {
   	//cerr << "request : " << endl;
	   if( endl)
		   ap_log_error( APLOG_MARK, l, 0, request->server, "%s",msg.c_str() );
	   else
		   ap_log_error( APLOG_MARK, l, 0, request->server, "%s\n",msg.c_str() );
   }
   
   if( pool ) {
   	//cerr << "pool : " << endl;
	   if( endl )
		   ap_log_perror(APLOG_MARK, APLOG_NOTICE, 0,pool, "%s", msg.c_str() );
	   else
		   ap_log_perror(APLOG_MARK, APLOG_NOTICE, 0,pool, "%s\n", msg.c_str() );
   }
}
