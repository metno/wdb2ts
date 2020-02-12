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

#ifndef __CONFIGDATA_H__
#define __CONFIGDATA_H__

#include <time.h>
#include <string>
#include <iostream>
#include <map>
#include <boost/shared_ptr.hpp>
#include "WebQuery.h"
#include "EnableTimePeriod.h"
#include "ConfigUtils.h"
#include "Metric.h"


namespace wdb2ts {

class Wdb2TsApp;

struct ExpireConfig {
	typedef enum { Now, NearestModelTimeResolution }Reference;
	Reference ref_;
	int modelTimeResolution_;
	int expireRand_;
	int expire_;
	mutable unsigned int seed_;
	ExpireConfig( const Reference &ref, int modelResolutionInSeconds, int randOffset, int expire):
		ref_(ref), modelTimeResolution_( modelResolutionInSeconds), expireRand_(randOffset),
		expire_(expire), seed_(time(0)){}
	ExpireConfig():ref_(NearestModelTimeResolution),modelTimeResolution_(3600),expireRand_(0),expire_(0),seed_(time(0)){}

	boost::posix_time::ptime expire(const boost::posix_time::ptime &ref=boost::posix_time::second_clock::universal_time())const;
	static ExpireConfig readConf(const wdb2ts::config::ActionParam &conf );
	friend std::ostream &operator<<(std::ostream &o, const ExpireConfig &conf);
};

struct ConfigData {

   ConfigData(const ConfigData &);
   ConfigData& operator=(const ConfigData &);
   OutputParams parameterMap;
   WebQuery     *webQueryDummy;
   const WebQuery &webQuery;
   std::string  url;
   bool throwNoData;
   std::string requestedProvider;
   miutil::EnableTimePeriod thunder;
   ExpireConfig expireConf;
private:
   PtrProviderRefTimesByDbId reftimes;
public:
   bool isForecast;
   std::string defaultDbId;
   int altitude;
   wdb2ts::Wdb2TsApp *app;
   miutil::Metric dbMetric;
   miutil::Metric decodeMetric;
   miutil::Metric validateMetric;
   miutil::Metric requestMetric;
   miutil::Metric encodeMetric;

   ConfigData();
   ConfigData(const WebQuery  &webQuery, wdb2ts::Wdb2TsApp *app);
   ~ConfigData();
   void setReferenceTimes(PtrProviderRefTimesByDbId ref) { reftimes=ref;}
   bool outputParam( const std::string &param )const;
   PtrProviderRefTimes getReferenceTimeByDbId(const std::string &dbId);
   PtrProviderRefTimesByDbId getRefererenceTimes()const{ return reftimes;}
   void setReferenceTime(PtrProviderRefTimes reftime, const std::string &dbId="");
   WciConnectionPtr newWciConnection( const std::string &dbid="", unsigned int timeoutInMilliSecound=2000 );
};

typedef boost::shared_ptr<ConfigData> ConfigDataPtr;

}


#endif /* CONFIGDATA_H_ */
