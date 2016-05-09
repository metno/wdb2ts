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
#include <stdlib.h>
#include <boost/date_time/posix_time/ptime.hpp>
#include <configdata.h>
#include <Logger4cpp.h>

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
ConfigData()
    : throwNoData( false )
{
}

bool
ConfigData::
outputParam( const std::string &param )const
{
   return parameterMap.useParam( param );
}

}
