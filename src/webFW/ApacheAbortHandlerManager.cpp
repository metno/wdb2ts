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

#include <iostream>
#include <boost/thread/xtime.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <ApacheAbortHandlerManager.h>
#include <ApacheRequest.h>

using namespace std;

namespace webfw {


void	
ApacheAbortHandlerManager::
AbortHelper::
operator()()
{
	boost::xtime xt;

	while( true ) {
		xtime_get(&xt, boost::TIME_UTC);
		xt.sec+=15;

		boost::thread::sleep( xt );
		abortManager->checkAboretedConnections();
	}
}

ApacheAbortHandlerManager::
ApacheAbortHandlerManager()
	: nextId( 1 ), abortThread( 0 )
{
	abortThread = new boost::thread( AbortHelper( this ) );
}


ApacheAbortHandlerManager::
~ApacheAbortHandlerManager()
{
}

void 
ApacheAbortHandlerManager::
checkAboretedConnections()
{
	boost::mutex::scoped_lock locl( mutex );
	
	boost::posix_time::ptime now=boost::posix_time::second_clock::universal_time();
	//cerr << "checkAbortedConnections: " << now << endl;
	
	std::map<unsigned long, AbortHandlerHelper*>::iterator it=handlerMap.begin();
	
	while( it != handlerMap.end() ) {
		if( it->second->req->request->connection->aborted ) {
			cerr << "checkAbortedConnections: connection aborted '" << it->first << "'." << endl;
			std::map<unsigned long, AbortHandlerHelper*>::iterator itTmp=it;
			++it;
			itTmp->second->handler->onAbort();
			delete itTmp->second;
			handlerMap.erase( itTmp );
		} else {
			++it;
		}
	}
}

unsigned long
ApacheAbortHandlerManager::
registerAbortHandler( Request *req_, IAbortHandler *handler )
{
	boost::mutex::scoped_lock locl( mutex );
	
	ApacheRequest *req = static_cast<ApacheRequest*>( req_ ) ;
	
	try {
		AbortHandlerHelper *helper = new AbortHandlerHelper( req, handler );
		unsigned long id = nextId;
		nextId++;
		handlerMap[id] = helper;
		return id;
	}
	catch( ... ) {
	}
	
	return 0;
}
	
void 
ApacheAbortHandlerManager::
removeAbortHandler( unsigned long handlerId )
{
	boost::mutex::scoped_lock locl( mutex );
	
	std::map<unsigned long, AbortHandlerHelper*>::iterator it = handlerMap.find( handlerId );
	
	if( it != handlerMap.end() ) {
		AbortHandlerHelper *handler = it->second;
		handlerMap.erase( it );
		delete handler;
	}
}

void 
ApacheAbortHandlerManager::
removeAllAbortHandler( const std::list<unsigned long> &idList  )
{
	boost::mutex::scoped_lock locl( mutex );
	std::map<unsigned long, AbortHandlerHelper*>::iterator it;
	
	for( std::list<unsigned long>::const_iterator itIdList=idList.begin();
	     itIdList != idList.end(); 
	     ++itIdList )
	{
		it = handlerMap.find( *itIdList );
		
		if( it != handlerMap.end() ) {
			AbortHandlerHelper *handler = it->second;
			handlerMap.erase( it );
			delete handler;
		}
	}
}


}

