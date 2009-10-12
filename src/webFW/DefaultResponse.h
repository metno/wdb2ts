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
#ifndef __DEFAULTRESPONSE_H__
#define __DEFAULTRESPONSE_H__

#include <sstream>
#include <Response.h>

namespace webfw {

class DefaultResponse 
   : public Response
{  
   protected:
      std::ostringstream out_;
      
   public:
      DefaultResponse();
      virtual ~DefaultResponse();

      virtual void contentType(const std::string &content_type );
      virtual void expire( const boost::posix_time::ptime &exp );
      virtual void contentLength( int content_length );
      
      std::string content()const; 

      virtual bool directOutput()const { return false; }
      virtual void directOutput(bool /*flag */) { /* do nothing*/ };
      
      virtual std::ostream& out() ;
      virtual void sendStream( std::istream &ist );
};

}


#endif
