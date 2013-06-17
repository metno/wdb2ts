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

#ifndef __GETTHREAD_H__
#define __GETTHREAD_H__

#include <float.h>
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <string>
#include <UrlQuery.h>

namespace wdb2ts {

class GetThread
{
	struct Results {
		int nSuccess;
		int nFailed;
		int nCrcFail;
		int unavailable;
	};
	
	boost::shared_ptr<Results> results;
	boost::shared_ptr<webfw::UrlQuery> query;
	int nRuns;
	int id;
	unsigned int randState;
	std::string url;
	float lat, lon;
	bool all_;
	
protected:
	void nextLatLong( float &lat, float &lon );
	
public:
	GetThread(const std::string &url_, float lat_, float lon_, int runs, unsigned int randInitState, int id_ )
		: results( new Results ), query( new webfw::UrlQuery), nRuns( runs ), id( id_ ), randState( randInitState ),
		  url( url_ ), lat( lat_ ), lon( lon_ ), all_( false )
	{ 
		results->nSuccess = 0;
		results->nFailed  = 0;
		results->nCrcFail = 0; 
		results->unavailable = 0;
	};
	
	GetThread( const GetThread &gt )
			: results( gt.results ), query( gt.query ), nRuns( gt.nRuns ), id( gt.id ), randState( gt.randState ),
			  url( gt.url ), lat( gt.lat ), lon( gt.lon ), all_( gt.all_ )
		{};

	void operator()();
	
	int runs()const { return nRuns; };
	int success() const { return results->nSuccess; }
	int failed() const { return results->nFailed;}
	int crcFail() const { return results->nCrcFail; }
	int unavailable() const { return results->unavailable; }

	bool all()const{ return all_;}
	void all( bool f ) { all_ = f; }
};

}


#endif 
