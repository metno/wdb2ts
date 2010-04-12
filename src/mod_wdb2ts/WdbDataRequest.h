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
#include <queuemt.h>
#include <WdbDataRequestCommand.h>
#include <PointDataHelper.h>
#include <UpdateProviderReftimes.h>
#include <ParamDef.h>
#include <Config.h>


namespace wdb2ts {



class WdbDataRequest {
	WdbDataRequestCommandPtr request;
	miutil::queuemt<int> &readyQueue;
	int id;
public:
	WdbDataRequest( WdbDataRequestCommandPtr request_,
			        miutil::queuemt<int> &readyQueue_,
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

		ThreadInfo( WdbDataRequestCommand *command_, const std::string &wdbid_, bool mustHaveData_, bool stopIfData_ )
			: command( command_ ), thread( 0 ), wdbid( wdbid_ ), mustHaveData( mustHaveData_ ),
			  stopIfData( stopIfData_ ), completed( false )
		{}

		~ThreadInfo()
		{
			if( thread )
				delete thread;
		}
	};


	int nParalell;
	miutil::queuemt<int> readyQueue;
	std::list< boost::shared_ptr<ThreadInfo> > threadInfos;
	std::list< boost::shared_ptr<ThreadInfo> >::iterator nextToStart;
	std::list< boost::shared_ptr<ThreadInfo> >::iterator lastStarted;
	bool waitingForDbConnection;
	bool toManyThreads;
	std::map< int, boost::shared_ptr<ThreadInfo> > runningThreads;
	int nextThreadId;

	void
	populateThreadInfos( const std::string &wdbid,
						 const ParamDefList &paramDefs,
						 const LocationPointList &locationPoints,
						 const boost::posix_time::ptime &toTime,
						 bool isPloygon,
						 PtrProviderRefTimes refTimes,
						 const ProviderList  &providerPriority,
						 const wdb2ts::config::Config::Query &urlQuerys,
						 int wciProtocol );

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

	WdbDataRequestManager();

	/**
	 * @throws std::logic_error, ResourceLimit on failure.
	 */
	LocationPointDataPtr requestData( Wdb2TsApp *app,
									  const std::string &wdbid,
									  const LocationPointList &locationPoints,
			                          const boost::posix_time::ptime &toTime,
			                          bool isPloygon,
			                          int altitude,
			                          PtrProviderRefTimes refTimes,
			                          const ProviderList  &providerPriority,
			                          const wdb2ts::config::Config::Query &urlQuerys,
			                          int wciProtocol );
};

}

#endif
