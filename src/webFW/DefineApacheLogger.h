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

#ifndef SRC_WEBFW_DEFINEAPACHELOGGER_H_
#define SRC_WEBFW_DEFINEAPACHELOGGER_H_

#include <iostream>
#include <httpd.h>
#include <http_config.h>
#include <http_protocol.h>
#include <apr_optional.h>
#include <ap_config.h>
#include <http_log.h>
#include "Logger.h"


#ifdef APLOG_USE_MODULE
#  define MY_APLOG_USE_MODULE(moduleName) APLOG_USE_MODULE(moduleName);
#else
#  define MY_APLOG_USE_MODULE(moduleName);
#endif


#define DEFINE_APACHE_LOGGER( Name ) \
class Name ## ApacheLogger           \
   : public webfw::Logger            \
{                                    \
   request_rec *request;             \
   apr_pool_t  *pool;                \
                                     \
	public:                           \
      Name ## ApacheLogger( request_rec *request );\
      Name ## ApacheLogger( apr_pool_t *pool );    \
      virtual ~Name ## ApacheLogger();             \
                                                   \
      virtual void log( LogLevel ll, const std::string &msg )const;    \
};                                                                     \
                                                                       \
Name ## ApacheLogger::                                                 \
Name ## ApacheLogger(  request_rec *request_ )                         \
   :  request( request_ ), pool( 0 )                                   \
{                                                                           \
}                                                                           \
                                                                            \
Name ## ApacheLogger::                                                      \
Name ## ApacheLogger( apr_pool_t *pool_ )                                   \
   : request( 0 ), pool( pool_ )                                            \
{                                                                           \
}                                                                           \
                                                                            \
Name ## ApacheLogger::                                                      \
~Name ## ApacheLogger()                                                     \
{                                                                           \
}                                                                           \
                                                                            \
void                                                                        \
Name ## ApacheLogger::                                                      \
log( LogLevel ll, const std::string &msg )const                             \
{                                                                           \
MY_APLOG_USE_MODULE(Name);                                                  \
   int l;                                                                   \
                                                                            \
   switch( ll ) {                                                           \
      case Fatal:   l = APLOG_CRIT;    break;                               \
      case Error:   l = APLOG_ERR;     break;                               \
      case Warning: l = APLOG_WARNING; break;                               \
      case Info:    l = APLOG_INFO;    break;                               \
      case Debug:                                                           \
      case Debug1:                                                          \
      case Debug2:  l =APLOG_DEBUG;    break;                               \
      default:                                                              \
         l = APLOG_DEBUG;                                                   \
   }                                                                        \
                                                                            \
   if( request ) {                                                          \
      ap_log_error( APLOG_MARK, l, 0, request->server, "%s",msg.c_str() );  \
   }                                                                        \
                                                                            \
   if( pool ) {                                                             \
      ap_log_perror(APLOG_MARK, APLOG_NOTICE, 0,pool, "%s", msg.c_str() );  \
   }                                                                        \
}



#endif /* SRC_WEBFW_DEFINEAPACHELOGGER_H_ */
