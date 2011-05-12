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


#ifndef __WDB2TS_REQUEST_CONF_H__
#define __WDB2TS_REQUEST_CONF_H__

#include <limits.h>
#include <boost/shared_ptr.hpp>
#include <string>
#include <list>
#include <map>
#include <ostream>
#include <Value.h>
#include <ParamDef.h>
#include <NextRun.h>

namespace wdb2ts {
namespace config {


typedef std::map<std::string, miutil::Value> ActionParam;

struct ParamDef {
	miutil::Value id;
	miutil::Value valueparameterName;
	miutil::Value valueparameterUnit;
	miutil::Value levelName;
	miutil::Value levelUnit;
	int           levelFrom;
	int           levelTo;
	float   valueScale;
	float   valueOffset;
	int     dataVersion;
	wdb2ts::ParamDef::Compare compare;
	int                       compareValue;

	ParamDef()
		: levelFrom( INT_MIN ), levelTo( INT_MIN ),
		  valueScale(1.0f), valueOffset(0.0f),
		  dataVersion( -1 ),
		  compare(wdb2ts::ParamDef::undef ), compareValue( INT_MAX )
		  {}
	
	void clear() {
		id = miutil::Value();
		valueparameterName = miutil::Value();
		valueparameterUnit = miutil::Value();
		levelName = miutil::Value();
		levelUnit = miutil::Value();
		levelFrom = INT_MIN;
		levelTo = INT_MIN;
		valueScale = 1.0f;
		valueOffset = 0.0f;
      dataVersion = -1;
      compare = wdb2ts::ParamDef::undef;
      compareValue = INT_MAX;
	}
	
	bool valid() {
		if( id.asString("").empty() ||
			 valueparameterName.asString("").empty() ||
			 levelName.asString("").empty() ||
			 levelFrom == INT_MIN || levelTo == INT_MIN )
			return false;
		return true;
	}
	
};



struct Version
{
	Version( const Version &v ) 
		: majorVer( v.majorVer), minorVer( v.minorVer )
		{}
	
	Version( int major, int minor ) 
		: majorVer( major ), minorVer( minor ) 
		{}

	Version( const std::string &version); 

	Version():
		majorVer( -1 ), minorVer( -1 )
		{}
	
	std::string operator()()const;
	
	bool defaultVersionHighest()const  { return majorVer==-1 && minorVer==-1; }
	bool invalid()const { return majorVer==0 && minorVer==0; }
	
	friend std::ostream& operator<<(std::ostream& output,
		                             const Version& v);
	
	Version& operator=(const Version& rhs );
	
	
	bool operator<( const Version &rhs );
	bool operator==( const Version &rhs );
	bool operator!=( const Version &rhs );
	
	int majorVer;
	int minorVer;
	
	
};

std::ostream& 
operator<<(std::ostream& output,
           const Version& v);


struct RequestConf
{
public:
	
	RequestConf(){}
	RequestConf(const RequestConf &r )
		: actionParam( r.actionParam ), queryid( r.queryid ),
		  action( r.action ), wdbDB( r.wdbDB ), version( r.version )
	{}
	
	~RequestConf(){}
	
	ActionParam actionParam;
	Update     nextRun; //Metadata
	miutil::Value queryid;
	miutil::Value action;
	miutil::Value wdbDB;
	miutil::Value schema;
	Version       version;
};


class Request
{
public:
	Request() {}
	Request( const Request &r )
		: requestDefault( r.requestDefault ), requestVersions( r.requestVersions )
		  
	{}
	
	~Request() {}
	
	typedef std::list<boost::shared_ptr<RequestConf> > RequestVersions;
	
	RequestConf     requestDefault;
	RequestVersions requestVersions;
	
	miutil::Value path;
	
	bool addRequestVersion( boost::shared_ptr<RequestConf> requestVersion, std::string &warning );

	/**
	 * Fill in missing actions params from requestConf into 
	 * requestVersions or fill inn default values;
	 */  
	void resolve();
	
	friend std::ostream& operator<<(std::ostream& output,
	                                const Request& r);
};

std::ostream& 
operator<<(std::ostream& output, const Request& r );

typedef std::map<std::string, boost::shared_ptr<Request> > RequestMap;

void addRequest( RequestMap &requestMap, boost::shared_ptr<Request> request );

}
}

#endif 
