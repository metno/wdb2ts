/*
  Diana - A Free Meteorological Visualisation Tool

  $Id: dbConnection.h 3123 2010-03-04 10:04:58Z borgem $

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



#ifndef __DBCONNECTION_H__
#define __DBCONNECTION_H__

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <pqxx/pqxx>
#include "dbConnectionHelper.h"




namespace miutil { 

namespace  pgpool {

class DbConnectionPool;


class DbConnection : public  boost::noncopyable {
   DbConnection();   //Not implemented
   
   internal::DbConnectionHelper *connectionHelper_;
   DbConnectionPool             *pool_;
   
   friend class DbConnectionPool;
   
   DbConnection(internal::DbConnectionHelper *connection, DbConnectionPool *pool);
         
   public:
      virtual ~DbConnection();
      
      
      pqxx::connection& connection();
};         

typedef boost::shared_ptr<DbConnection> DbConnectionPtr;
      
}
}

#endif 
