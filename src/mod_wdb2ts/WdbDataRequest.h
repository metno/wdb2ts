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

#ifndef __WDBDATAREQUEST_H__
#define __WDBDATAREQUEST_H__

#include <exception>
#include <string>
#include <boost/shared_ptr.hpp>
#include <queuemt.h>
#include <WdbDataRequestCommand.h>
#include <PointDataHelper.h>
#include <UpdateProviderReftimes.h>
#include <ParamDef.h>
#include <Config.h>
#include <QueryMaker.h>
#include <Metric.h>
#include <pgconpool/dbConnectionException.h>


namespace wdb2ts {

typedef boost::shared_ptr< miutil::queuemt<int> > ReadyQue;

class WdbDataRequest {
   WdbDataRequestCommandPtr request;
   ReadyQue readyQueue;
   //miutil::queuemt<int> &readyQueue;
   int id;
public:
   WdbDataRequest( WdbDataRequestCommandPtr request_,
                   //miutil::queuemt<int> &readyQueue_,
		   	        ReadyQue readyQueue_,
                   int id_ )
   : request( request_ ), readyQueue( readyQueue_ ), id( id_ )
   {
   }

   void operator()();
};


class WdbDataRequestManager {
   struct ThreadInfo {
      WdbDataRequestCommandPtr command;
      boost::thread *thread;
      std::string wdbid;
      bool mustHaveData;
      bool stopIfData;
      bool completed;
      boost::posix_time::ptime minPrognosisLength;

      ThreadInfo( WdbDataRequestCommand *command_, const std::string &wdbid_,
    		      bool mustHaveData_, bool stopIfData_,
				  const boost::posix_time::ptime &minPrognosisLength_ )
      : command( command_ ), thread( 0 ), wdbid( wdbid_ ), mustHaveData( mustHaveData_ ),
        stopIfData( stopIfData_ ), completed( false ), minPrognosisLength( minPrognosisLength_ )
      {}

      ~ThreadInfo()
      {
         if( thread )
            delete thread;
      }
   };


   int nParalell;
   ReadyQue readyQueue;
   //miutil::queuemt<int> readyQueue;
   std::list< boost::shared_ptr<ThreadInfo> > threadInfos;
   std::list< boost::shared_ptr<ThreadInfo> >::iterator nextToStart;
   std::list< boost::shared_ptr<ThreadInfo> >::iterator lastStarted;
   bool waitingForDbConnection;
   bool toManyThreads;
   bool noConnection;
   std::map< int, boost::shared_ptr<ThreadInfo> > runningThreads;
   int nextThreadId;
   boost::posix_time::ptime from;


   void populateThreadInfos(
		   ConfigData *config,
   		   ParamDefListPtr  paramDefs,
   		   ProviderListPtr  &providerPriority,
   		   const wdb2ts::config::Config::Query &urlQuerys,
		   int wciProtocol
		   );
   /*
   populateThreadInfos( const std::string &wdbidDefault,
                        ParamDefListPtr paramDefs,
                        const LocationPointList &locationPoints ,
                        const boost::posix_time::ptime &toTime ,
                        bool isPolygon ,
                        PtrProviderRefTimes refTimes,
                        ProviderListPtr  providerPriority,
                        const wdb2ts::config::Config::Query &urlQuerys,
                        int wciProtocol )


   void
   populateThreadInfos( const std::string &wdbid,
                        ParamDefListPtr paramDefs,
                        const LocationPointList &locationPoints,
                        const boost::posix_time::ptime &toTime,
                        bool isPloygon,
                        PtrProviderRefTimes refTimes,
                        ProviderListPtr  providerPriority,
                        const wdb2ts::config::Config::Query &urlQuerys,
                        int wciProtocol );
*/
   void
   populateThreadInfos( const qmaker::QuerysAndParamDefsPtr querys,
		                const std::string &wdbid,
                        const LocationPointList &locationPoints,
                        const boost::posix_time::ptime &toTime,
                        bool isPloygon );


   /**
   * wait for threads in the runningThreads list to complete. It is waited for
   * at least the number of threads given with waitForAtLeast.
   * If waitForAtLeast is zero we wait to all threads in the list has completed.
   * If the list is empty the the function returns imidetly.
   *
   * @param waitForAtLeast 0 wait for all, else wait for the number of threads given.
   * @return The number of threads completed.
   */
   int	waitForCompleted( int waitForAtLeast=0 );


   void startThreads( Wdb2TsApp &app );

   void runRequests( Wdb2TsApp &app );

   LocationPointDataPtr mergeData( bool isPolygon );


public:
   class ResourceLimit : public std::exception
   {
      std::string reason_;
   public:
      ResourceLimit( const std::string &reason) throw():
         reason_(reason)   { }
      virtual ~ResourceLimit() throw() {};
      const char* what() const throw() { return reason_.c_str(); }
   };

   miutil::Metric dbMetric;
   miutil::Metric dbDecodeMetric;
   miutil::Metric validateMetric;

   WdbDataRequestManager();

   /**
   * @throws std::logic_error, ResourceLimit on failure.
   */
   LocationPointDataPtr requestData( ConfigData *config,
                                     ParamDefListPtr  paramDefs,
                                     const ProviderList  &providerPriority,
                                     const wdb2ts::config::Config::Query &urlQuerys,
                                     int wciProtocol );


   LocationPointDataPtr requestData( Wdb2TsApp *app,
		   	   	   	   	   	   	   	 const qmaker::QuerysAndParamDefsPtr querys,
                                     const std::string &wdbid,
                                     const LocationPointList &locationPoints,
									 const boost::posix_time::ptime &from,
                                     const boost::posix_time::ptime &toTime,
                                     bool isPloygon,
                                     int altitude);

};

}

#endif
