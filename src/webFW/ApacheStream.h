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
#ifndef __APACHESTREAM_H__
#define __APACHESTREAM_H__

#include <sstream>
#include <string>
#include <httpd.h>
#include <http_config.h>
#include <http_protocol.h>
#include <apr_strings.h>
#include <iosfwd>    //streamsize
#include <boost/iostreams/categories.hpp>  // sink_tag
#include <exception.h>

namespace webfw {

class ApacheStream{
   const int chunckSize;
   std::ostringstream ost; //For buffred output.
   request_rec        *r;  //Apache request
   int                size_;
   int                nChunck;
   bool               directOutput_;
   apr_bucket_brigade *bb;
   
   
public:
    typedef char                       char_type;
    typedef boost::iostreams::sink_tag category;
    
    ApacheStream( const ApacheStream &cp );
    ApacheStream(request_rec *r);
    ~ApacheStream();
   
    void contentType( const char *content_type );
    void expire( const char *exp );
    void contentLength( int content_length );
    void serviceUnavailable( const char *retryAfter );
    
    /**
     * Check the IO status and throw IOError if
     * an IO error ocured.
     */
    void checkIOStatusAndThrow( apr_status_t status );
    
    void sendToClient( const char *buf );
    
    std::streamsize write(const char_type* s, std::streamsize n);
   
    void directOutput( bool flag );
    bool directOutput()const { return directOutput_;}
    int size()const { return size_;}
    void flushStream();
    std::string content(){ return ost.str(); }
};

}
#endif 
