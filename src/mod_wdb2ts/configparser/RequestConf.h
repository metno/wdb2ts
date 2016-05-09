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
#include <boost/lexical_cast.hpp>
#include <stdexcept>
#include <string>
#include <list>
#include <map>
#include <ostream>
#include <Value.h>
#include <ParamDef.h>
#include <NextRun.h>


namespace wdb2ts {
namespace config {


class ActionParam : public std::map<std::string, miutil::Value>{
public:
	ActionParam(){}
	~ActionParam(){}

	/**
	 * @throw std::range_error if the key do not exist.
	 */
	std::string getStr(const std::string &key)const;
	std::string getStr(const std::string &key, const std::string &defaultValue)const;


	/**
	 * @throw std::range_error if the key do not exist.
	 * @throw boost::bad_lexical_cast if the value can not be converted to T.
	 */
	template <typename T>
	T get(const std::string &key)const{
		return boost::lexical_cast<T>( getStr(key) );
	}

	template <typename T>
	T get(const std::string &key, T defaultValue)const{
		try {
			return boost::lexical_cast<T>( getStr(key) );
		}
		catch( const std::exception &e) {
			return defaultValue;
		}
	}

	bool hasKey(const std::string &key, bool valueMustBeDefined=true)const;
};

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
	
	bool valid( bool ignoreLevelParameters ) {

		if( id.asString("").empty() ||
			valueparameterName.asString("").empty() )
			return false;

		if( ! ignoreLevelParameters )
			return hasLevelDef();

		return true;
	}
	
	bool hasLevelDef()const {
		return levelName.defined() && levelUnit.defined() &&
			   levelFrom != INT_MIN && levelTo != INT_MIN;
	}
};

struct ParamDefConfig {
	typedef std::map< std::string, wdb2ts::ParamDefList > ParamDefs;
	ParamDefs            idParamDefs;
	//wdb2ts::ParamDefList paramDefs;

	void clear();
	/**
	 * Add a paramdef to either paramDefs or idParamDefs.
	 *
	 * It is added to idParamDefs if paramdefId is set and to
	 * paramdefs if peramdefId is not set.
	 *
	 * Return false if a paramdef with the same alias allready exist.
	 * Return true if the paramdef was added to the paramsDefs.
	 */
	bool
	addParamDef( const std::string &paramdefId,
			     const wdb2ts::ParamDef    &pd,
		         const std::list<std::string> &provider,
		         bool replace,
		         std::ostream &err );

	bool hasParam(  const std::string &alias,
	                const std::string &provider,
	                const std::string paramdefsid )const;

	bool hasParamDefId( const std::string &id )const;
	wdb2ts::ParamDefList paramDefs( const std::string paramdefsId="")const;

	wdb2ts::ParamDefList getParamDefs( const std::list<std::string> &providers );
	void merge( ParamDefConfig *other, bool replace=false );
	friend std::ostream &operator<<(std::ostream &o, const ParamDefConfig &pd );

};
std::ostream &operator<<(std::ostream &o, const ParamDefConfig &pd );


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
	
	std::string asString()const;
	
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
	ParamDefConfig paramdef;
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
