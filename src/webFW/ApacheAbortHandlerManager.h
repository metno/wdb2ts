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

#ifndef __APACHE_ABORTHANDLERMANAGER_H__
#define __APACHE_ABORTHANDLERMANAGER_H__

#include <list>
#include <boost/thread/thread.hpp>
#include <IAbortHandlerManager.h>
#include <ApacheRequest.h>



namespace webfw {

class Request;

class ApacheAbortHandlerManager : 
	   public IAbortHandlerManager, private boost::noncopyable
{
	class AbortHelper {
		ApacheAbortHandlerManager *abortManager;
		
	public:
		AbortHelper( ApacheAbortHandlerManager *mgr ):
			abortManager( mgr )
			{}
		
		void operator()();
	};
	
	struct AbortHandlerHelper{
		ApacheRequest *req;
		IAbortHandler *handler;
		
		AbortHandlerHelper( ApacheRequest *req_, IAbortHandler *handler_ )
			: req( req_ ), handler( handler_ ) {}
		~AbortHandlerHelper(){
			delete handler;
		}
	};
	
	
	unsigned long nextId;
	std::map<unsigned long, AbortHandlerHelper*> handlerMap;
	boost::thread *abortThread;
	boost::mutex mutex;
	
	void checkAboretedConnections();
	
public:
	explicit ApacheAbortHandlerManager();
	virtual ~ApacheAbortHandlerManager();
		
	virtual unsigned long  registerAbortHandler( Request *req, IAbortHandler *handler );
	virtual void removeAbortHandler( unsigned long handlerId );
	virtual void removeAllAbortHandler( const std::list<unsigned long> &idList  );
};


}



#endif 
