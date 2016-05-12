/*
  Diana - A Free Meteorological Visualisation Tool

  $Id: dbConnectionException.h 3278 2010-05-21 06:36:29Z borgem $

  Copyright (C) 2006 met.no

  Contact information:
  Norwegian Meteorological Institute
  Box 43 Blindern
  0313 OSLO
  NORWAY
  email: diana@met.no
  
  This file is part of Diana

  Diana is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  Diana is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with Diana; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef __DBCONNECTIONEXCEPTION_H__
#define __DBCONNECTIONEXCEPTION_H__

#include <string>
#include <exception>

namespace miutil { 

namespace  pgpool {


/**
 * Base class for all DbConnection and DbConnectionPool exceptions.
 */
class DbConnectionException : public std::exception
{
   std::string reason_;
   
   public:
      DbConnectionException(const std::string &reason) throw(): 
         reason_(reason)   { }
      virtual ~DbConnectionException() throw() {};
      const char* what() const throw() { return reason_.c_str(); }
};

class DbConnectionDisabledEx : public DbConnectionException
{
   public:
   DbConnectionDisabledEx(const std::string &reason="Connection pool disabled!")
         : DbConnectionException(reason){}
};


class DbConnectionPoolMaxUseEx : public DbConnectionException
{
   public:
      DbConnectionPoolMaxUseEx(const std::string &reason="Max connection in use!")
         : DbConnectionException(reason){}
};

class DbConnectionPoolCreateEx : public DbConnectionException
{
   public:
      DbConnectionPoolCreateEx(const std::string &reason="Cant create an database connection!")
         : DbConnectionException(reason){}
};


}
}

#endif
