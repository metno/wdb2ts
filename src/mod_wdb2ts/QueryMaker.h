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
#ifndef __QUERYMAKER_H__
#define __QUERYMAKER_H__

#include <map>
#include <list>
#include <string>
#include <boost/date_time.hpp>
#include <ParamDef.h>
#include <RequestConf.h>
#include <UpdateProviderReftimes.h>

namespace wdb2ts{
class Level {
public:
	typedef enum { exact, below, above, inside, any, udef } Qualifier;

protected:
	void setQualifier();
	int from_;
	int to_;
	std::string levelparametername_;
	std::string unit_;
	Qualifier qualifier_;

public:
	Level():from_(INT_MIN),to_(INT_MIN), qualifier_( udef ) {}
	Level( int from, int to, const std::string &levelparametername,
		   const std::string &unit, Qualifier qualifier=udef);
	Level( const ParamDef &param );

	int from() const { return from_; }
	int to() const { return to_; }
	std::string levelparametername() const { return levelparametername_; }
	std::string unit() const { return unit_; }
	Qualifier qualifier()const { return qualifier_; }
	std::string qualifierAsString()const;
	bool isEqual( const Level &toThis, bool ignoreQualifier )const;
	bool operator<(const Level &rhs)const;
	bool operator==(const Level &rhs )const;
	bool isDefined()const;
	std::string toWdbLevelspec()const;
	friend std::ostream& operator<<( std::ostream &out, const Level &lvl );
};

std::ostream& operator<<( const std::ostream &out, const Level &lvl );

namespace qmaker {

struct Query{
	std::string id;
	std::string provider;
	std::list<ParamDef> params;
	std::list<std::string> querys;

	Query( const std::string &id, const std::string &provider )
		: id( id ), provider( provider ) {}

	bool operator==( const Query &rhs)const;
	void addParam( const ParamDef &param );
	void merge( Query *q );
	std::list<std::string> getQuerys(  int wciProtocol )const;
	friend std::ostream& operator<<( std::ostream &out, const Query &q );
};

std::ostream&
operator<<( std::ostream &out, const Query &q );

struct QuerysAndParamDefs {
	std::list<Query*> querys;
	ProviderList providerPriority;
	ParamDefList params;
	PtrProviderRefTimes referenceTimes;
	int wciProtocol;

	QuerysAndParamDefs( int wciProtocol ): wciProtocol( wciProtocol ){}
	~QuerysAndParamDefs();

	void addQuery( Query *q );
	void addParamDefs( const ParamDefList &params );
	void merge( const QuerysAndParamDefs &with );
	friend std::ostream& operator<<( std::ostream &out, const QuerysAndParamDefs &q );
};

typedef boost::shared_ptr<QuerysAndParamDefs> QuerysAndParamDefsPtr;

std::ostream& operator<<( std::ostream &out, const QuerysAndParamDefs &q );

class QueryMaker {
public:
	typedef std::list<ParamDef> ParamDefs;
	typedef std::map<Level, ParamDefs> LevelParams;
	//[paramdefid][provider]
	typedef std::map<std::string, std::map<std::string, LevelParams > > Params;

private:
	Params params;
	int wciProtocol;
	float latitude;
	float longitude;
	PtrProviderRefTimes referenceTimes;
	std::pair<boost::posix_time::ptime, boost::posix_time::ptime> validTime;
	int dataVersion;

protected:

	void createQuery( Query *query,
			          const ParamDefs &params,
			          const Level &lvl,
			          ParamDefList &parOut )const;
	void getWdbReadQuerys( QuerysAndParamDefsPtr res,
			               const std::string &id,
			               const Level &level,
		                   const std::list<std::string> &providerList,
			               const std::map<std::string, LevelParams> &params )const;


//			float latitude, float longitude,
//							 const std::string &provider,
//							 const wdb2ts::ParamDef &param,
//							 const Level &lvl, ParamDef &parOut );

	QueryMaker();
	QueryMaker( int wciProtocol);
public:

	static QueryMaker* create( const wdb2ts::config::ParamDefConfig &params,
			                   int wciProtocol );

	std::list<std::string> getProviders( const std::string &id )const;
	QuerysAndParamDefsPtr
	getWdbReadQuerys( const std::string &id,
			          float latitude_, float longitude_,
		              const ProviderList &providerList,
		              const Level &level,
		              const PtrProviderRefTimes referenceTimes,
		              const std::pair<boost::posix_time::ptime, boost::posix_time::ptime> &validTime=
				          std::make_pair( boost::posix_time::ptime(), boost::posix_time::ptime() ),
		              int dataVersion=INT_MAX
	                );
};

typedef boost::shared_ptr<QueryMaker> QueryMakerPtr;
}

}


#endif
