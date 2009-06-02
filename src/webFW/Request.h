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
#ifndef __REQUEST_H__
#define __REQUEST_H__

#include <string>

namespace webfw {

class Request
{
   public:
      typedef enum{ UNDEF, GET, PUT, POST, DELETE } RequestType;
      
   private:
      RequestType reqType;
      
   protected:
   
      void requestType( RequestType rt ) { reqType = rt; }
         
   public:
      
   
      Request()
         : reqType( UNDEF )
      {};
      
      Request( RequestType req )
         : reqType( req )
      {};
      virtual ~Request(){};
      
      RequestType requestType() const { return reqType; }
      virtual std::string urlPath()const =0;
      virtual std::string urlQuery()const =0;
};

}

#endif 
