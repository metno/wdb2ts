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
#include <sstream>
#include <UrlQuery.h>
#include <boost/crc.hpp>      // for boost::crc_basic, boost::crc_optimal
#include <map>
#include <miutil/Metric.h>
#include "LatLongBase.h"


namespace wdb2ts {

class GetThread
{
	struct Results {
		int nSuccess;
		int nFailed;
		int nCrcFail;
		int unavailable;
		miutil::Metric timer;
		Results(int id):timer("thread_"+std::to_string(id)){
		}
	};
	
	boost::shared_ptr<Results> results;
	boost::shared_ptr<webfw::UrlQuery> query;
	boost::shared_ptr<std::map<std::string, boost::crc_basic<16>::value_type > > crcResults;
	boost::crc_basic<16>::value_type crcValue;
	boost::crc_basic<16> crc_ccitt1;
	int nRuns;
	int id;
	std::string url;
	LatLongBase *nextLatLong_;
	bool all_;
	
protected:
	void nextLatLong( float &lat, float &lon );
	void validateResult(const std::string &path, const std::string &request, std::ostringstream &out);

public:
	GetThread(const std::string &url_, LatLongBase *latlong, int runs, int id_ )
		: results( new Results(id_) ),  query( new webfw::UrlQuery),
		  crcResults( new std::map<std::string, boost::crc_basic<16>::value_type >()),
		  crcValue(), crc_ccitt1( 0x1021, 0xFFFF, 0, false, false ),
		  nRuns( runs ), id( id_ ),
		  url( url_ ), nextLatLong_(latlong) , all_( false )
	{ 
		results->nSuccess = 0;
		results->nFailed  = 0;
		results->nCrcFail = 0; 
		results->unavailable = 0;
	};
	
	GetThread( const GetThread &gt )
			: results( gt.results ), query( gt.query ),
			  crcResults(gt.crcResults),
			  crcValue(gt.crcValue), crc_ccitt1(gt.crc_ccitt1),
			  nRuns( gt.nRuns ), id( gt.id ),
			  url( gt.url ), nextLatLong_(gt.nextLatLong_), all_( gt.all_ )
		{};

	void operator()();
	
	int getId()const { return id;}

	int runs()const { return nRuns; };
	int success() const { return results->nSuccess; }
	int failed() const { return results->nFailed;}
	int crcFail() const { return results->nCrcFail; }
	int unavailable() const { return results->unavailable; }
	miutil::Metric& metric(){ return results->timer; }

	bool all()const{ return all_;}
	void all( bool f ) { all_ = f; }
};

}


#endif 
