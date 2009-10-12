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
#include <ApacheResponse.h>
#include <Logger4cpp.h>

webfw::
ApacheResponse::
ApacheResponse( request_rec *r )
   : out_( r )
{
   out_.exceptions( std::ios_base::badbit | std::ios_base::failbit );   
}

webfw::
ApacheResponse::
~ApacheResponse()
{
}
      

void
webfw::
ApacheResponse::
contentType(const std::string &content_type )
{
   Response::contentType( content_type );
   (*const_cast<OutStream*>(&out_))->contentType( content_type.c_str() );
}


void 
webfw::
ApacheResponse::
expire( const boost::posix_time::ptime &exp )
{
   Response::expire( exp );
   
   if( ! exp.is_special() ) {                                 
      std::string rfc1123;                                       
      rfc1123 = miutil::rfc1123date( exp );                   
                                                                     
      if( ! rfc1123.empty() )
         (*const_cast<OutStream*>(&out_))->expire( rfc1123.c_str() );
              
   }
}

void 
webfw::
ApacheResponse::
serviceUnavailable(const boost::posix_time::ptime &retryAfter )
{
   Response::serviceUnavailable( retryAfter );
   
   if( ! retryAfter.is_special() ) {                                 
      std::string rfc1123;                                       
      rfc1123 = miutil::rfc1123date( retryAfter );                   
                                                                     
      if( ! rfc1123.empty() )
         (*const_cast<OutStream*>(&out_))->serviceUnavailable( rfc1123.c_str() );
   }
}


void 
webfw::
ApacheResponse::
contentLength( int content_length )
{
   Response::contentLength( content_length );
   (*const_cast<OutStream*>(&out_))->contentLength( content_length );
}



bool 
webfw::
ApacheResponse::
directOutput()const
{
   return (*const_cast<OutStream*>(&out_))->directOutput();
}

void 
webfw::
ApacheResponse::
directOutput(bool flag )
{
   (*const_cast<OutStream*>(&out_))->directOutput( flag );
}
      


std::string 
webfw::
ApacheResponse::
content()const
{
   return (*const_cast<OutStream*>(&out_))->content();
}

void 
webfw::
ApacheResponse::
flush()
{
   out_.flush();
   (*const_cast<OutStream*>(&out_))->flushStream();
}

std::ostream& 
webfw::
ApacheResponse::
out()
{
   return out_;
}

void
webfw::
ApacheResponse::
sendStream( std::istream &ist )
{
	int N(1024*1024);
	char buf[N+1];
	int n=0;

//	WEBFW_USE_LOGGER( "encode" );

	directOutput( true );

	while( ist.read( buf, N ) ) {
		n = ist.gcount();
		buf[ n ] = '\0';
		(*const_cast<OutStream*>(&out_))->sendToClient( buf );
	}

	if( ist.eof() ) {
		n = ist.gcount();
		buf[ n ] = '\0';
		(*const_cast<OutStream*>(&out_))->sendToClient( buf );
	} else {
		throw IOError("ERROR: Failed to read the input when sending the response to the clent.!", true );
	}
}
