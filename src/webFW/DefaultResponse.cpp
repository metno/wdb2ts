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

#include <ptimeutil.h>
#include <DefaultResponse.h>

webfw::
DefaultResponse::
DefaultResponse()
{
}

webfw::
DefaultResponse::
~DefaultResponse()
{
}
 

void
webfw::
DefaultResponse::
contentType(const std::string &content_type )
{
   Response::contentType( content_type );
   out_ << "Countent-type: " << content_type << std::endl; 
}

void 
webfw::
DefaultResponse::
expire( const boost::posix_time::ptime &exp )
{
   Response::expire( exp );
   
   if( ! exp.is_special() ) {                                 
      std::string rfc1123;                                       
      rfc1123 = miutil::rfc1123date( exp );                   
                                                                     
      if( ! rfc1123.empty() )
         out_ << "Expire: " << rfc1123 << std::endl;     
   }
}

void 
webfw::
DefaultResponse::
contentLength( int content_length )
{
   Response::contentLength( content_length );
   out_ << "Content-length: " << content_length << std::endl; 
}


std::string 
webfw::
DefaultResponse::
content()const
{
   return out_.str();
}


std::ostream& 
webfw::
DefaultResponse::
out()
{
   return out_;
}

void
webfw::
DefaultResponse::
sendStream( std::istream &ist )
{
}
