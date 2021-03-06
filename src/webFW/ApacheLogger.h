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
#ifndef __APACHELOGGER_H__
#define __APACHELOGGER_H__
#include <httpd.h>
#include <http_config.h>
#include <http_protocol.h>
#include <apr_optional.h>


#include <Logger.h>


namespace webfw {

class ApacheLogger
   : public Logger 
{
   request_rec *request;
   apr_pool_t  *pool;
   std::string moduleName;

	public:
      ApacheLogger( request_rec *request, const std::string &modName );
      ApacheLogger( apr_pool_t *pool, const std::string &modName );
      virtual ~ApacheLogger();
      
      virtual void log( LogLevel ll, const std::string &msg )const; 
};

}

#endif 
