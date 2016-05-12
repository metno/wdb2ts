/*
  Diana - A Free Meteorological Visualisation Tool

  $Id: dbConnectionPool.h 3694 2011-07-13 11:33:09Z borgem $

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

#ifndef __DBCONNECTIONPOOL_H__
#define __DBCONNECTIONPOOL_H__

#include <list>
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/noncopyable.hpp>

#include "dbConnectionException.h"
#include "dbConnection.h"
#include "dbSetup.h"
namespace miutil {

namespace pgpool {

//class DbConnectionHelper;

namespace internal {
class DbConnectionHelper;
}

class ConnectionExtra {
public:

	///Called when a new connection is created on the server.
   virtual void onCreateConnection( pqxx::connection &con );
   
   ///Called when a connection is closed (server).
   virtual void onRemoveConnection( pqxx::connection &con );
   
   ///Called every time on DbConnectionPool::newConnection.
   virtual void onNewConnection( pqxx::connection &con );
   
   /**
    * Called every time on a DbConnectionPtr when the connection is put back to the pool
    * ie. the referance count is zero.
    */
   virtual void onCloseConnection( pqxx::connection &con );
};

class DbConnectionPool : public boost::noncopyable {
   std::list<internal::DbConnectionHelper*> pool;
   std::list<internal::DbConnectionHelper*>::size_type maxpoolsize_;
   std::list<internal::DbConnectionHelper*>::size_type minpoolsize_;
   int maxUseCount_;
   std::string connect_;
   bool isEnabled_;
   
   ConnectionExtra *conExtra;
   
   mutable boost::mutex mutex;
   mutable boost::condition cond;
   
   friend class DbConnection;
  
   protected:
      void release(internal::DbConnectionHelper *con);
      internal::DbConnectionHelper* createConnection();
      internal::DbConnectionHelper* findOrCreateConnection();

      void createMinpoolsize();

  
   public:
      DbConnectionPool(const std::string &connect="", ConnectionExtra *conExtra=0 );
      DbConnectionPool( const DbDef &dbdef,
                        ConnectionExtra *conExtra=0);
      
      DbConnectionPool(int maxConnections, 
                       const std::string &connect="", 
                       int maxUseCount=1,
                       ConnectionExtra *conExtra=0);
                
      ~DbConnectionPool();
  
      void disable();
      void enable();

      bool isEnabled()const;

      std::string connectString()const { return connect_; }
      void        connectString(const std::string &connect) { connect_=connect; }
      
      int maxConnections()const { return maxpoolsize_; }
      void maxConnection(int max) { maxpoolsize_ = max; }
      
      int maxUseCount()const { return maxUseCount_;}
      void maxUseCount(int max) { maxUseCount_ = max; }
      
      
      /**
       * @param timeoutInMillisecond Wait at most this time for a connection.
       *        If 0 do not wait.
       * @throw DbConnectionPoolMaxUseEx, DbConnectionPoolCreateEx,
       *        DbConnectionDisabledEx
       */
      DbConnectionPtr newConnection( unsigned int timeoutInMillisecond=0 );
   
}; 

typedef boost::shared_ptr<DbConnectionPool> DbConnectionPoolPtr;

}
}
#endif 
