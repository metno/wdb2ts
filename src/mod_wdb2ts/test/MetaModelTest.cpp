/*
    wdb - weather and water data storage

    Copyright (C) 2008 met.no

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

#include <fstream>
#include <math.h>
#include <Logger4cpp.h>
#include <MetaModelConf.h>
#include <MetaModelTest.h>
#include <ptimeutil.h>
#include <Config.h>

CPPUNIT_TEST_SUITE_REGISTRATION( MetaModelTest );

using namespace std;
using namespace wdb2ts;
namespace pt=boost::posix_time;

namespace {

}



MetaModelTest::
MetaModelTest()
{
	//NOOP
}

MetaModelTest::
~MetaModelTest()
{
	// NOOP
}

void
MetaModelTest::
setUp()
{


	WEBFW_USE_LOGGER( "handler" );
	WEBFW_SET_LOGLEVEL( log4cpp::Priority::ERROR );
}

void
MetaModelTest::
tearDown()
{
	// NOOP
}







void
MetaModelTest::
testNextRunSpecific()
{
	using namespace wdb2ts;
	config::ActionParam params;
	MetaModelConfList modelConfs;
	params["meta_model-proff"]=miutil::Value("name=LOCAL;nextrun=04:00,16:00");

	modelConfs=configureMetaModelConf(params);

	CPPUNIT_ASSERT(modelConfs.size()==1);
	CPPUNIT_ASSERT(modelConfs.begin()->first == "proff" );

	TimeDurationList tds=modelConfs.begin()->second.nextrun();

	CPPUNIT_ASSERT(tds.size()==2);
	TimeDurationList::const_iterator td=tds.begin();

	td=tds.begin();
	CPPUNIT_ASSERT(td->total_seconds()==(4*3600));
	td++;
	CPPUNIT_ASSERT(td->total_seconds()==(16*3600));

	MetaModelConf::setNowTimeForTest(pt::time_from_string("2016-05-24 14:27:13"));
	boost::posix_time::ptime t=modelConfs.begin()->second.findNextrun();
	CPPUNIT_ASSERT(t==pt::time_from_string("2016-05-24 16:00:00"));
}


void
MetaModelTest::
testNextRunRelativToLoadtime()
{
	using namespace wdb2ts;
	config::ActionParam params;
	MetaModelConfList modelConfs;
	params["meta_model-proff"]=miutil::Value("name=LOCAL;nextrun=+450");

	modelConfs=configureMetaModelConf(params);

	CPPUNIT_ASSERT(modelConfs.size()==1);
	CPPUNIT_ASSERT(modelConfs.begin()->first == "proff" );

	TimeDurationList tds=modelConfs.begin()->second.nextrun();

	CPPUNIT_ASSERT(tds.size()==1);
	TimeDurationList::const_iterator td=tds.begin();
	CPPUNIT_ASSERT(td->total_seconds() == 450);

	MetaModelConf::setNowTimeForTest(pt::time_from_string("2016-05-24 14:27:13"));
	boost::posix_time::ptime refTime=pt::time_from_string("2016-05-24 14:00:00");
	boost::posix_time::ptime t=modelConfs.begin()->second.findNextrun(refTime);
	CPPUNIT_ASSERT(t==(refTime+pt::seconds(450)));
}


void
MetaModelTest::testNextRunEvery()
{
	using namespace wdb2ts;
	config::ActionParam params;
	MetaModelConfList modelConfs;
	boost::posix_time::ptime dummy=pt::time_from_string("2016-05-24 14:00:00");

	params["meta_model-proff"]=miutil::Value("name=LOCAL;nextrun=*/450");

	modelConfs=configureMetaModelConf(params);
	CPPUNIT_ASSERT(modelConfs.size()==1);
	CPPUNIT_ASSERT(modelConfs.begin()->first == "proff" );

	TimeDurationList tds=modelConfs.begin()->second.nextrun();

	CPPUNIT_ASSERT(tds.size()==192);
	TimeDurationList::const_iterator td=tds.begin();
	int accSeconds=0;
	for( TimeDurationList::const_iterator td=tds.begin(); td!=tds.end(); ++td) {
		CPPUNIT_ASSERT(td->total_seconds() == accSeconds);
		accSeconds+=450;
	}

	MetaModelConf::setNowTimeForTest(pt::time_from_string("2016-05-24 14:27:13"));

	boost::posix_time::ptime t=modelConfs.begin()->second.findNextrun(dummy);
	CPPUNIT_ASSERT(t==pt::time_from_string("2016-05-24 14:30:00"));

	params["meta_model-proff"]=miutil::Value("name=LOCAL;nextrun=120/450");
	modelConfs=configureMetaModelConf(params);

	CPPUNIT_ASSERT(modelConfs.size()==1);
	CPPUNIT_ASSERT(modelConfs.begin()->first == "proff" );

	tds=modelConfs.begin()->second.nextrun();

	CPPUNIT_ASSERT(tds.size()==192);
	td=tds.begin();
	accSeconds=120;
	for( TimeDurationList::const_iterator td=tds.begin(); td!=tds.end(); ++td) {
		CPPUNIT_ASSERT(td->total_seconds() == accSeconds);
		accSeconds+=450;
	}
	t=modelConfs.begin()->second.findNextrun(dummy);
	CPPUNIT_ASSERT(t==pt::time_from_string("2016-05-24 14:32:00"));
}
