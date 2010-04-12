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
#ifndef __RESPONSE_H__
#define __RESPONSE_H__

#include <string>
#include <iostream>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace webfw {


class Response
{  
   public:
      enum Status{  NO_ERROR, 
                    PUT_NOT_SUPPORTED,
                    GET_NOT_SUPPORTED,
                    POST_NOT_SUPPORTED,
                    DELETE_NOT_SUPPORTED,
                    NOT_SUPPORTED,
                    NOT_FOUND, 
                    INVALID_PATH,
                    INVALID_QUERY, 
                    INTERNAL_ERROR,
                    SERVICE_UNAVAILABLE,
                    NO_DATA,
                    CONFIG_ERROR
                  };
   
   protected:
      boost::posix_time::ptime expire_;
      boost::posix_time::ptime retryAfter_;
      std::string contentType_;
      long         contentLength_;
      std::string errorDoc_;     
      Status status_;
      std::string aboutRequestHandler_;
      
  public:
      Response();
      virtual ~Response();
   
      
      std::string aboutRequestHandler()const { return aboutRequestHandler_; }
      void aboutRequestHandler(const std::string &rh ) { aboutRequestHandler_=rh; }
      
      virtual void contentType(const std::string &content_type ) { contentType_=content_type; }
      std::string contentType()const { return contentType_; };
      
      virtual void expire( const boost::posix_time::ptime &exp ){ expire_ = exp; }
      boost::posix_time::ptime expire()const { return expire_; }
      
      virtual void serviceUnavailable(const boost::posix_time::ptime &retryAfter ) 
      				 { retryAfter_ = retryAfter; }	
      
      boost::posix_time::ptime serviceUnavailable() const
                   { return retryAfter_; }
      
      virtual void contentLength( long content_length ) { contentLength_ = content_length; }
      long contentLength()const { return contentLength_; }
      
      void errorDoc( const std::string &doc ){ errorDoc_ = doc; }
      std::string errorDoc()const { return errorDoc_; }
   
      void status( Status s ) { status_ = s; }
      Status status() const { return status_; }

      /**
       * Does the outstream buffer the output before the data
       * is sendt to the client or is the data sendt in chuncks.
       * 
       * Buffred data support the \e content-size atribute to the 
       * HTTP protocol. Chuncked data does NOT support this atribute. 
       * 
       * @return true the data is sendt in chuncks. false data is
       *              buffered and sendt at the end of the request.
       * 
       */
      virtual bool directOutput()const =0;
      
      
      /**
       * Does the outstream buffer the output before the data
       * is sendt to the client or is the data sendt in chuncks.
       * 
       * Buffred data support the \e content-size atribute to the 
       * HTTP protocol. Chuncked data does NOT support this atribute. 
       *
       * The deafult value should be false. When the value is set
       * too true it can not be set to false again. The call will
       * fail silently in this case. 
       * 
       * When the stream is set to true the data buffred up to
       * this time is flushed to the client before the function return. 
       *  
       * @exception webFW::IOError.
       */
      virtual void directOutput(bool flag ) =0;
      
      /**
       * The output stream may throw webFW::IOError when 
       * an error from the protocol is detetected.
       * 
       * This may occour in the HTTP protocol.
       * 
       * @exception webFW::IOError.
       */
      virtual std::ostream& out()=0;

      /**
       * sendStream may throw webFW::IOError when
       * an error from the protocol is detetected.
       *
       * This may occour in the HTTP protocol.
       *
       * @exception webFW::IOError.
       */
      virtual void sendStream( std::istream &ist )=0;

      /**
       * sendFile may throw webFW::IOError when
       * an error from the protocol is detetected.
       *
       * The content length is set to the lenght of the file in the response.
       *
       * This may occour in the HTTP protocol.
       *
       * @param file The file to send.
       * @param deleteFile delete the file.
       * @return The number of bytes actually sendt.
       * @exception webFW::IOError.
       */
      virtual long sendFile( const std::string &file,  bool deleteFile )=0;

};

}


#endif
