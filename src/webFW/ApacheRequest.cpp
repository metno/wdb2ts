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
#include <replace.h>
#include <ApacheRequest.h>

webfw::
ApacheRequest::
ApacheRequest( request_rec *r )
	: request( r )
{
   RequestType rt;
   
   switch(r->method_number ) {
      case M_GET:    rt = Request::GET;    break;
      case M_PUT:    rt = Request::PUT;    break;
      case M_DELETE: rt = Request::DELETE; break;
      case M_POST:   rt = Request::POST;   break;
      default:
         rt = Request::UNDEF;
   }
   
   requestType( rt );
   
   if( r->parsed_uri.path ) {
      path = r->parsed_uri.path;
     // miutil::replace( path, "%20", " ");
   }

   if( r->parsed_uri.query ) {
      query = r->parsed_uri.query;
    //  miutil::replace( query, "%20", " ");
   }

}

webfw::
ApacheRequest::
~ApacheRequest()
{
};
      
std::string 
webfw::
ApacheRequest::
urlPath()const
{
   return path;
}
 
std::string 
webfw::
ApacheRequest::
urlQuery()const
{
   return query;
}
