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

#include <unistd.h>
#include <WdbDataRequest.h>
#include <WdbQueryHelper.h>
#include <wdb2tsProfiling.h>
#include <Logger4cpp.h>
#include <sstream>

using namespace std;

namespace wdb2ts {


void
WdbDataRequest::
operator()()
{
	try{
		(*request)();
	}
	catch( ... ) {
		//NOOP. No exceptions is allowed to escape.
	}

	readyQueue.push( id );
}

WdbDataRequestManager::
WdbDataRequestManager()
	: nParalell(1), nextThreadId( 0 )
{
}

/**
 * @throws logic_error
 */

void
WdbDataRequestManager::
populateThreadInfos( const std::string &wdbidDefault,
			         const ParamDefList &paramDefs,
					 const LocationPointList &locationPoints ,
					 const boost::posix_time::ptime &toTime ,
					 bool isPolygon ,
					 PtrProviderRefTimes refTimes,
					 const ProviderList  &providerPriority,
					 const wdb2ts::config::Config::Query &urlQuerys,
					 int wciProtocol )
{
	string wciReadQuery;
	string dbProfileProvider__; //Only used when profiling
	string decodeProfileProvider__; //Only used when profiling
	string wdbid;
	bool   mustHaveData;
	bool   stopIfQueryHasData;

	WEBFW_USE_LOGGER( "wdb" );


	if( urlQuerys.empty() ) {
		throw logic_error( "EXCEPTION: No wdb querys is defined." );
	}

	WdbQueryHelper wdbQueryHelper( urlQuerys, wciProtocol );

	try {
		wdbQueryHelper.init( locationPoints, toTime, isPolygon, *refTimes, providerPriority );

		while ( wdbQueryHelper.hasNext() ) {
			try {
				wciReadQuery = wdbQueryHelper.next( mustHaveData, stopIfQueryHasData );
				wdbid = wdbQueryHelper.wdbid();

				if( wdbid.empty() )
					wdbid = wdbidDefault;
			}
			catch( const NoReftime &exNoReftime ) {
				WEBFW_LOG_WARN(exNoReftime.what());
				continue;
			}


			boost::shared_ptr<ThreadInfo> threadInfo(
						new ThreadInfo(
							new WdbDataRequestCommand( WciConnectionPtr(),
									                   webfw::RequestHandler::getRequestHandler(),
									                   wciReadQuery,
									                   paramDefs,
									                   providerPriority,
									                   refTimes,
									                   wciProtocol,
									                   isPolygon ),
							wdbid,
							mustHaveData,
							stopIfQueryHasData )
					);
			threadInfos.push_back( threadInfo );
		}
	}
	catch( const exception &ex ){
		WEBFW_LOG_ERROR( "WciReadLocationForecast: query [" << wciReadQuery << "] reason: " << ex.what() );
		throw;
	}
}

void
WdbDataRequestManager::
startThreads( Wdb2TsApp &app )
{
	WEBFW_USE_LOGGER( "wdb" );

	while( nextToStart != threadInfos.end() ) {
		if( nParalell>0 && runningThreads.size() >= nParalell ) {
			toManyThreads=true;
			return;
		}

		try {
			WciConnectionPtr wciConnection = app.newWciConnection( (*nextToStart)->wdbid );
			(*nextToStart)->command->setConnection( wciConnection );
			waitingForDbConnection = false;
			toManyThreads = false;
		}
		catch( const miutil::pgpool::DbConnectionPoolMaxUseEx &ex ) {
			waitingForDbConnection = true;
			return;
		}
		catch( const miutil::pgpool::DbConnectionPoolCreateEx &ex ) {
			waitingForDbConnection = true;
			return;
		}
		catch( ... ) {
			WEBFW_LOG_ERROR("startThreads: Unable to create a new database connection.");
			throw;
		}

		try {
			WEBFW_LOG_DEBUG("startThreads (tid=" << nextThreadId << "): wdb: " << (*nextToStart)->wdbid );
			boost::thread *thread = new boost::thread( WdbDataRequest( (*nextToStart)->command, readyQueue, nextThreadId ) );
			(*nextToStart)->thread = thread;
			runningThreads[nextThreadId] = *nextToStart;
			nextThreadId++;
			lastStarted = nextToStart;
			++nextToStart;
		}
		catch( ... ) {
			WEBFW_LOG_ERROR("startThreads (tid=" << nextThreadId << "): Unable to start a new request thread");
			throw logic_error( "Cant start a new request thread." );
		}

		if( (*lastStarted)->stopIfData ) {
			//WEBFW_LOG_DEBUG("startThreads (tid=" << nextThreadId-1 << "):Has stopIfData flag set.");
			return;
		}
	}

}

int
WdbDataRequestManager::
waitForCompleted( int waitForAtLeast )
{
	WEBFW_USE_LOGGER( "wdb" );
	int n=0;
	int tid;
	map< int, boost::shared_ptr<ThreadInfo> >::iterator it;

	if( runningThreads.empty() )
		return 0;

	while( ! runningThreads.empty() ) {
		if( waitForAtLeast==0
		    || ( waitForAtLeast > 0 && n<waitForAtLeast ) ) {
			readyQueue.waitAndPop( tid );
		}else {
			if( ! readyQueue.tryPop( tid ) )
				return n;
		}

		n++;
		it = runningThreads.find( tid );

		if( it == runningThreads.end() ) {
			WEBFW_LOG_ERROR("waitForCompleted (tid=" <<tid << "): FATAL: A thread not waited for completed.");
			continue;
		}

		WEBFW_LOG_DEBUG("waitForCompleted (tid=" <<tid << "): Thread completed.");
		it->second->completed=true;
		it->second->thread->join();
		runningThreads.erase( it );
	}

	return n;
}

void
WdbDataRequestManager::
runRequests( Wdb2TsApp &app )
{
	WEBFW_USE_LOGGER( "wdb" );
	const int WAIT_SECONDS = 10;
	nextToStart = threadInfos.begin();
	lastStarted = threadInfos.end();
	waitingForDbConnection = false;
	bool stop=false;
	bool inWaitingForDbConnection=false;
	time_t waitUntil;
	time_t now;
	time_t prevTime;

	try {
		while( nextToStart != threadInfos.end() && !stop) {
			startThreads( app );

			if( toManyThreads ) {
				WEBFW_LOG_DEBUG("runRequests: To many threads is running." );
				waitForCompleted( 1 );
			}

			if( waitingForDbConnection  ) {
				if( ! inWaitingForDbConnection ) {
					//WEBFW_LOG_DEBUG("runRequests: Waiting for a db connection." );
					inWaitingForDbConnection = true;
					time( &waitUntil );
					prevTime = waitUntil;
					waitUntil += WAIT_SECONDS;  //Wait in max 5 seconds
				} else {
					time( &now );
					if( now != prevTime ) {
						prevTime = now;
						//WEBFW_LOG_DEBUG("runRequests: Continue waiting for a db connection." );
					}
					if( now > waitUntil ) {
						//WEBFW_LOG_DEBUG("runRequests: Failed to obtain a db connection (waited " << WAIT_SECONDS <<" seconds)." );
						throw ResourceLimit( "Waiting more than 5 seconds on db connection.");
					}
				}

				if( runningThreads.size()>0 )
					waitForCompleted( 1 );
				else
					usleep( 1000 ); //For now use usleep in one millisecond, should use nanosleep

				continue;
			}

			if( lastStarted != threadInfos.end() ) {

				if( (*lastStarted)->stopIfData ) {
					waitForCompleted();

					//Check that we have data for this query.
					//If not, start more request if any.
					if( ! (*lastStarted)->command->data().empty() )
						stop=true;
				}
			}
		}

		waitForCompleted();
	}
	catch( const exception &ex ) {
		WEBFW_LOG_DEBUG("runRequests: EXCEPTION: " << ex.what() );

		try {
			waitForCompleted();
		}
		catch( ... ) {
			WEBFW_LOG_DEBUG("runRequests: EXCEPTION: waitForCompleted failed!" );

		}

		throw logic_error( ex.what() );
	}
	catch( ... ) {
		WEBFW_USE_LOGGER( "wdb" );
		WEBFW_LOG_DEBUG("runRequests: EXCEPTION: Unknown!" );

		try {
			waitForCompleted();
		}
		catch( ... ) {
			WEBFW_LOG_DEBUG("runRequests: EXCEPTION: waitForCompleted failed!" );
		}
		throw logic_error( "runRequests: Unknown exception.");
	}
}



LocationPointDataPtr
WdbDataRequestManager::
mergeData( bool isPolygon )
{
	WEBFW_USE_LOGGER( "wdb" );
	LocationPointData *data = new LocationPointData();
	bool first = true;

	for( std::list< boost::shared_ptr<ThreadInfo> >::iterator it=threadInfos.begin();
		 it != threadInfos.end(); ++it )
	{
		if( ! (*it)->completed )
			continue;

		if( ! (*it)->command->ok() ) {
			WEBFW_LOG_ERROR("mergeData: A query failed. Message: " << (*it)->command->errMsg() );
			data->clear();
			return LocationPointDataPtr( data );
		}

		if( (*it)->mustHaveData && (*it)->command->data().empty() ) {
			WEBFW_LOG_INFO("mergeData: A query that is marked with 'mustHavaData' do NOT have data.");
			data->clear();
			return LocationPointDataPtr( data );
		}

		if( first ) {
			*data = (*it)->command->data();
			first = false;
			continue;
		}

		for( LocationPointData::iterator its1 = (*it)->command->data().begin();
				its1 != (*it)->command->data().end(); ++its1 )
		{
			LocationPointData::iterator itd1 = data->find( its1->first );

			for( TimeSerie::iterator its2=its1->second->begin();
				 its2 != its1->second->end(); ++its2 ) {
				for( FromTimeSerie::iterator its3 = its2->second.begin();
					 its3 != its2->second.end(); ++its3 ) {
					for( ProviderPDataList::iterator its4 = its3->second.begin();
						 its4 != its3->second.end(); ++its4 ) {
						if( isPolygon || data->size()==0 ) {
							if( itd1 == data->end() )
								(*data)[its1->first] = TimeSeriePtr( new TimeSerie() );

							(*(*data)[its1->first])[its2->first][its3->first][its4->first].merge( its4->second );
						} else {
							(*data->begin()->second)[its2->first][its3->first][its4->first].merge( its4->second );
						}
					}
				}
			}
		}
	}

	std::ostringstream ost;

	for( LocationPointData::iterator it = data->begin();
		 it != data->end(); ++it )
		ost << " ( " << it->first.latitude() << ", " << it->first.longitude() << " )";

	WEBFW_LOG_DEBUG("mergeData: locations: " << data->size() << ost.str() );

	return LocationPointDataPtr( data );
}

LocationPointDataPtr
WdbDataRequestManager::
requestData( Wdb2TsApp *app,
		     const std::string &wdbid,
		     const LocationPointList &locationPoints,
		     const boost::posix_time::ptime &toTime,
		     bool isPolygon,
		     int altitude,
		     PtrProviderRefTimes refTimes,
		     const ProviderList  &providerPriority,
		     const wdb2ts::config::Config::Query &urlQuerys,
		     int wciProtocol )
{
	nParalell = urlQuerys.dbRequestsInParalells();

	populateThreadInfos( wdbid, app->paramDefs(), locationPoints, toTime,
						 isPolygon, refTimes, providerPriority, urlQuerys,
						 wciProtocol );

	runRequests( *app );

	return mergeData( isPolygon );
}

}
