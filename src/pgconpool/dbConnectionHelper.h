/*
  Diana - A Free Meteorological Visualisation Tool

  Copyright (C) 2006-2013 met.no

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

#ifndef __DBCONNECTIONHELPER_H__
#define __DBCONNECTIONHELPER_H__

#include <pqxx/pqxx>

namespace miutil { 

namespace  pgpool {

   
class DbConnectionPool;
    
namespace internal {

class DbConnectionHelper {
   pqxx::connection connection_;
   int useCount_;
   bool inuse_;
     
   friend class miutil::pgpool::DbConnectionPool;
  
   DbConnectionHelper(int count, const std::string &connectString)
      : connection_(connectString), useCount_(count), inuse_(false)
      {} 
      
   bool inUse() const { return inuse_; }
   void inUse(bool f) { if ( f )
                           useCount_--;
                        inuse_=f; 
                      }
   
   
   public:
      ~DbConnectionHelper(){
         //std::cerr << "DbConnectionHelper: DTOR " << std::endl;
         connection_.disconnect();
      }
      
      pqxx::connection& connection() { return connection_; }
};

} // namespace internal
} // namespace  pgpool
} // namespace miutil
   
class DbConnectionPool;

#endif 
