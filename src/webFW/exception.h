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
#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__

#include <stdexcept>
#include <ios>

namespace webfw {

class ServerPageException : 
   public std::logic_error
{
   public:
      explicit ServerPageException( const std::string &what ) 
         : std::logic_error( what ){};
};


class InvalidPath : 
   public ServerPageException
{
   public:
      explicit InvalidPath( const std::string &what ) 
         : ServerPageException( what ){};
};
     
class NotFound : 
   public ServerPageException
{
   public:
      explicit NotFound( const std::string &what ) 
         : ServerPageException( what ){};
};
   
class ResourceError : 
   public ServerPageException
{
   public:
      explicit ResourceError( const std::string &what ) 
         : ServerPageException( what ){};
};

 
class NotSupported :
   public ServerPageException
{
   public:
      explicit NotSupported( const std::string &what ) 
         : ServerPageException( what ){};
};

class IOError : 
     public std::ios_base::failure
{
   bool connected_;
   
   public:
      explicit IOError( const std::string &what, bool connected=false )
         : std::ios_base::failure( what ), connected_( connected ) {}
      
   bool isConnected() const { return connected_; }   
};
     
}
 

#endif
