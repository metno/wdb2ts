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
#include <limits.h>
#include <stdlib.h>
#include <boost/date_time/posix_time/ptime.hpp>
#include <configdata.h>
#include <Logger4cpp.h>
#include <ProviderReftimes.h>
#include "wdb2TsApp.h"

using namespace std;
namespace pt=boost::posix_time;

namespace wdb2ts {



ExpireConfig ExpireConfig::readConf(const wdb2ts::config::ActionParam &conf ) {

	Reference ref=Now;
	int modelResolution = conf.get<int>("model_resolution", 3600);
	int expire = conf.get<int>("expire", INT_MAX);
	int expireRand = conf.get<int>("expire_rand", 0);

	if( expire == INT_MAX )
		ref=NearestModelTimeResolution;

	return ExpireConfig( ref, modelResolution, expireRand, expire);
}


boost::posix_time::ptime
ExpireConfig::expire(const boost::posix_time::ptime &ref)const
{
	using miutil::nearesTimeInTheFuture;
	pt::ptime myExpire;

	switch( ref_ ) {
	case NearestModelTimeResolution:
		myExpire=nearesTimeInTheFuture(modelTimeResolution_, ref);
	break;

	case Now:
		myExpire=ref+pt::seconds(expire_);
	break;
	}

 	if( expireRand_ > 0)
		myExpire += pt::seconds( rand_r( &seed_ ) % expireRand_ );

 	return myExpire;
}

std::ostream &operator<<(std::ostream &o, const ExpireConfig &conf) {
	o << "ExpireConfig: Ref: ";
	switch( conf.ref_ ) {
		case ExpireConfig::NearestModelTimeResolution: o << "NearestModelTimeResolution"; break;
		case ExpireConfig::Now: o << "Now"; break;
	}

	o << " model_resolution: " << conf.modelTimeResolution_ << " expire_rand: " << conf.expireRand_
	  << " expire: " << conf.expire_;
	return o;
}

ConfigData::
ConfigData():
webQueryDummy(new WebQuery()),webQuery(*webQueryDummy), throwNoData( false ),isForecast(true),altitude(INT_MAX),dbMetric("wdb_request"),
decodeMetric("wdb_decode"), validateMetric("validate"),
requestMetric("request"), encodeMetric("encode"),app(nullptr)
{
}

ConfigData::
ConfigData(const WebQuery  &webQuery_, wdb2ts::Wdb2TsApp *app_)
    : webQueryDummy(nullptr), webQuery(webQuery_), throwNoData( false ),isForecast(true),altitude(INT_MAX),dbMetric("wdb_request"),
		decodeMetric("wdb_decode"), validateMetric("validate"),
		requestMetric("request"), encodeMetric("encode"),app(app_)
{
}

ConfigData::
~ConfigData(){
	delete webQueryDummy;
}

bool
ConfigData::
outputParam( const std::string &param )const
{
   return parameterMap.useParam( param );
}

PtrProviderRefTimes
ConfigData::
getReferenceTimeByDbId(const std::string &dbId_)
{
	string dbId((dbId_.empty()?defaultDbId:dbId_));
	ProviderRefTimesByDbId::const_iterator it=reftimes->find(dbId);
	if( it == reftimes->end() ) {
		it=reftimes->find(defaultDbId);
		if( it == reftimes->end() )
			return PtrProviderRefTimes(new ProviderRefTimeList());
	}

	return it->second;
}


void
ConfigData::
setReferenceTime(PtrProviderRefTimes reftime, const std::string &dbId_)
{
	string dbId((dbId_.empty()?defaultDbId:dbId_));

	ProviderRefTimesByDbId::iterator it=reftimes->find(dbId);

	if( it == reftimes->end() ) {
		(*reftimes)[dbId]=reftime;
	}else{
		it->second =  reftime;
	}
}


WciConnectionPtr
ConfigData::newWciConnection( const std::string &dbid, unsigned int timeoutInMilliSecound)
{
	if(!app) {
		cerr << "\n\n  ConfigData::newWciConnection:  SHIT app==nullptr\n\n";
		return WciConnectionPtr();
	}

	return app->newWciConnection(dbid, timeoutInMilliSecound);

}
}
