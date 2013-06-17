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
#ifndef __DB_MANAGER_H__
#define __DB_MANAGER_H__

#include <stdexcept>
#include <map>
#include <string>
#include <boost/shared_ptr.hpp>
#include <pgconpool/dbConnectionPool.h>
#include <pgconpool/dbSetup.h>
#include <boost/thread/thread.hpp>
#include <transactor/WciTransactor.h>

namespace wdb2ts {

class DbExceptionResourceLimit :
   public std::exception
{
public:
      explicit DbExceptionResourceLimit(){}
};


	class WciConnection 
	{
		WciConnection();
		WciConnection( const WciConnection& );
		WciConnection& operator=( const WciConnection &);
		
		std::string wciuser;
		miutil::pgpool::DbConnectionPtr con;
			
		friend class DbManager;
		WciConnection( miutil::pgpool::DbConnectionPtr con, const std::string &wciuser )
			: wciuser( wciuser ), con( con) {}
				
	public:
		 template<typename TRANSACTOR>
		 void perform(const TRANSACTOR &T, int Attempts)
		 {
			 con->connection().perform( wdb2ts::WciTransactor<TRANSACTOR>( T, wciuser), Attempts );
		 }
		 	
		/// Perform the transaction defined by a transactor-based object.
	   /**
		 * @param T The transactor to be executed.
		 */
		 template<typename TRANSACTOR>
		 void perform(const TRANSACTOR &T) { perform(T, 3); }
	};

	typedef boost::shared_ptr<WciConnection> WciConnectionPtr; 
	

   /**
    * DbManager maintaince a list of database pools, one for
    * each of the database configurations in the configuration
    * file.
    */
   class DbManager
   {
      std::map<std::string, miutil::pgpool::DbConnectionPoolPtr> pools_;
      miutil::pgpool::DbDefList                         dbSetup_;
      std::string                                       defaultDbId_;
    //  boost::mutex                                    mutex;
      
   public:
      DbManager( const miutil::pgpool::DbDefList     &dbSetup );

      ///Shutdown all connections that is not in use.
      ///The rest of the connections is shutdown when they are released.
      void disable();

      void defaultDbId( const std::string &id ) { defaultDbId_ = id; }
      std::string defaultDbId( ) { return defaultDbId_; }
      
      /**
       * @exception std::logic_error, miutil::pgpool::DbConnectionPoolMaxUseEx,
       *            miutill::pgpool::DbConnectionPoolCreateEx
       *
       */
      miutil::pgpool::DbConnectionPtr newConnection(const std::string &dbid="", int timeoutInMilliSeconds=2000 );
      
      /**
       *
       * @exception std::logic_error, miutil::pgpool::DbConnectionPoolMaxUseEx,
       *            miutill::pgpool::DbConnectionPoolCreateEx
	   */
      WciConnectionPtr newWciConnection(const std::string &dbid="", int timeoutInMilliSeconds=2000 );
      
   };
   
   typedef boost::shared_ptr<DbManager> DbManagerPtr;
}


#endif 
