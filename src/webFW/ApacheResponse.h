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
#ifndef __APACHERESPONSE_H__
#define __APACHERESPONSE_H__


#include <iostream> 
#include <boost/iostreams/stream.hpp>
#include <httpd.h>
#include <http_config.h>
#include <http_protocol.h>
#include <Response.h>
#include <ApacheStream.h>


namespace webfw {

class ApacheResponse 
   : public Response
{  
   protected:
      typedef boost::iostreams::stream<ApacheStream> OutStream;

      OutStream    out_;
      
   public:
      
      
      ApacheResponse(request_rec *r);
      virtual ~ApacheResponse();
      
      /**internal don't use it*/
      std::string content()const;
      
      /**internal don't use it*/
      void flush();
      
      using Response::contentType;
      virtual void contentType(const std::string &content_type );
      
      using Response::expire;
      virtual void expire( const boost::posix_time::ptime &exp );
      
      using Response::contentLength;
      virtual void contentLength( long content_length );
      
      using Response::serviceUnavailable;
      virtual void serviceUnavailable(const boost::posix_time::ptime &retryAfter ); 
      
      virtual bool directOutput()const;
      virtual void directOutput(bool flag );
            
      /**
       * The output stream may throw webFW::IOError when 
       * the connection to the client is lost or when other
       * error is detected when streaming data to the client.
       * 
       * The IOError::isConnected can be called to check if 
       * the connection is lost.
       *  
       * @exception webFW::IOError.
       */
      virtual std::ostream& out() ;
      
      virtual void sendStream( std::istream &ist );

      virtual long sendFile( const std::string &file, bool deleteFile );

};

}


#endif
