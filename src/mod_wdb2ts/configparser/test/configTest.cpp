#include <boost/assign/list_inserter.hpp>
#include <cppunit/config/SourcePrefix.h>
#include <exception.h>
#include <iostream>
#include <ptimeutil.h>
#include "configTest.h"
#include <src/mod_wdb2ts/configparser/RequestConf.h>
#include <src/mod_wdb2ts/configdata.h>

CPPUNIT_TEST_SUITE_REGISTRATION( ConfigTest );

using namespace std;
using namespace miutil;
using namespace boost::posix_time;

namespace {
	ptime pts(const std::string &s){
		ptime pt(time_from_string(s));
		return pt;
	}
}


void
ConfigTest::
setUp()
{
	// NOOP
}

void
ConfigTest::
tearDown() 
{
	// NOOP
}
      
void 
ConfigTest::
testTimePeriod()
{
   using namespace wdb2ts::config;
   using namespace boost::posix_time;
   using namespace boost::gregorian;

   TimePeriode tp1( time_duration( 22, 0, 0 ), time_duration( 3, 0, 0 ) );

   //Test inPeriod when the time period cross midnight.
   CPPUNIT_ASSERT_MESSAGE( "Timeperiod 1:1", tp1.inPeriode( time_duration( 23, 0, 0 ) ) );
   CPPUNIT_ASSERT_MESSAGE( "Timeperiod 1:2", tp1.inPeriode( time_duration( 22, 0, 0 ) ) );
   CPPUNIT_ASSERT_MESSAGE( "Timeperiod 1:3", tp1.inPeriode( time_duration( 3, 0, 0 ) ) );
   CPPUNIT_ASSERT_MESSAGE( "Timeperiod 1:4", tp1.inPeriode( time_duration( 2, 0, 0 ) ) );
   CPPUNIT_ASSERT_MESSAGE( "Timeperiod 1:5", !tp1.inPeriode( time_duration( 4, 0, 0 ) ) );
   CPPUNIT_ASSERT_MESSAGE( "Timeperiod 1:6", !tp1.inPeriode( time_duration( 21, 0, 0 ) ) );

   TimePeriode tp2( time_duration( 3, 0, 0 ), time_duration( 6, 0, 0 ) );

   //Test inPeriod when the time period is in the interval [0,23:59].
   CPPUNIT_ASSERT_MESSAGE( "Timeperiod 2:1", tp2.inPeriode( time_duration( 3, 0, 0 ) ) );
   CPPUNIT_ASSERT_MESSAGE( "Timeperiod 2:2", tp2.inPeriode( time_duration( 6, 0, 0 ) ) ) ;
   CPPUNIT_ASSERT_MESSAGE( "Timeperiod 2:3", !tp2.inPeriode( time_duration( 2, 0, 0 ) ) );
   CPPUNIT_ASSERT_MESSAGE( "Timeperiod 2:4", !tp2.inPeriode( time_duration( 7, 0, 0 ) ) );
   CPPUNIT_ASSERT_MESSAGE( "Timeperiod 2:5", tp2.inPeriode( time_duration( 4, 0, 0 ) ) );

   TimePeriode tp4( time_duration( 3, 0, 0 ), time_duration( 6, 0, 0 ) );

   //Test equal operator.
   CPPUNIT_ASSERT_MESSAGE( "Timeperiod 3:1", !(tp2 == tp1) );
   CPPUNIT_ASSERT_MESSAGE( "Timeperiod 3:2", tp2 == tp4 );

   //Test less than (<) operator.
   CPPUNIT_ASSERT_MESSAGE( "Timeperiod 4:1",  tp1<tp2 );
   TimePeriode tp3( time_duration( 4, 0, 0 ), time_duration( 6, 0, 0 ) );
   CPPUNIT_ASSERT_MESSAGE( "Timeperiod 4:2",  tp3<tp2 );

   //Test sorting of TimePeriods.
   UpdateGroup group("my");
   std::list<TimePeriode> expectedTimeList;

   boost::assign::push_back( expectedTimeList )(tp1)(tp3)(tp2);

   group.addTimePeriod( tp3 );
   group.addTimePeriod( tp1 );
   group.addTimePeriod( tp2 );
   std::list<TimePeriode> resultTimeList = group.getTimeList();

   CPPUNIT_ASSERT_MESSAGE( "Timeperiod 5:1",  resultTimeList.size() == expectedTimeList.size() );
   std::list<TimePeriode>::iterator it1=resultTimeList.begin();
   std::list<TimePeriode>::iterator it2=expectedTimeList.begin();
   for( ; it1 != resultTimeList.end(); ++it1, ++it2 ) {
      if( *it1 != *it2 )
         break;
   }
   CPPUNIT_ASSERT_MESSAGE( "Timeperiod 5:2",  it1 == resultTimeList.end() );

   //Test findTimePeriod
   UpdateGroup group1("my");
   group1.addTimePeriod( TimePeriode( time_duration( 22, 0, 0 ), time_duration( 1, 0, 0 ) ) );
   group1.addTimePeriod( TimePeriode( time_duration( 5, 0, 0 ), time_duration( 7, 0, 0 ) ) );
   group1.addTimePeriod( TimePeriode( time_duration( 9, 0, 0 ), time_duration( 11, 0, 0 ) ) );
   group1.addTimePeriod( TimePeriode( time_duration( 17, 0, 0 ), time_duration( 19 , 0, 0 ) ) );

   bool inPeriod;
   const TimePeriode *tp = group1.findTimePeriod( time_duration( 23, 0, 0 ), inPeriod );
   CPPUNIT_ASSERT_MESSAGE( "Timeperiod 6:1",  tp != 0 );
   CPPUNIT_ASSERT_MESSAGE( "Timeperiod 6:2",  *tp == TimePeriode( time_duration( 22, 0, 0 ), time_duration( 1, 0, 0 ) ) );
   CPPUNIT_ASSERT_MESSAGE( "Timeperiod 6:3", inPeriod );

   tp = group1.findTimePeriod( time_duration( 20, 0, 0 ), inPeriod );
   CPPUNIT_ASSERT_MESSAGE( "Timeperiod 6:4",  *tp == TimePeriode( time_duration( 22, 0, 0 ), time_duration( 1, 0, 0 ) ) );
   CPPUNIT_ASSERT_MESSAGE( "Timeperiod 6:5", !inPeriod );

   tp = group1.findTimePeriod( time_duration( 10, 0, 0 ), inPeriod );
   CPPUNIT_ASSERT_MESSAGE( "Timeperiod 6:6",  *tp == TimePeriode( time_duration( 9, 0, 0 ), time_duration( 11, 0, 0 ) ) );
   CPPUNIT_ASSERT_MESSAGE( "Timeperiod 6:7", inPeriod );

   tp = group1.findTimePeriod( time_duration( 8, 0, 0 ), inPeriod );
   CPPUNIT_ASSERT_MESSAGE( "Timeperiod 6:8",  *tp == TimePeriode( time_duration( 9, 0, 0 ), time_duration( 11, 0, 0 ) ) );
   CPPUNIT_ASSERT_MESSAGE( "Timeperiod 6:9", ! inPeriod );

   //Test maping of reftime to datetime clampet to nearest to in TimePeriode in
   //the future.
   TimePeriode  tpReftime( time_duration( 22, 0, 0 ), time_duration( 2, 0, 0 ) );
   ptime fromTime;
   CPPUNIT_ASSERT_MESSAGE( "Timeperiod 7:1",
                           (tpReftime.refto( time_from_string( "2011-05-12 23:00:00"), fromTime ) ==
                           time_from_string( "2011-05-13 2:00:00" )) &&
                           fromTime == time_from_string( "2011-05-12 22:00:00" )  );

   CPPUNIT_ASSERT_MESSAGE( "Timeperiod 7:2",
                           (tpReftime.refto( time_from_string( "2011-05-12 1:00:00"), fromTime ) ==
                           time_from_string( "2011-05-12 2:00:00" )) &&
                           fromTime == time_from_string( "2011-05-11 22:00:00" ));

   CPPUNIT_ASSERT_MESSAGE( "Timeperiod 7:3",
                           tpReftime.refto( time_from_string( "2011-05-12 22:00:00"), fromTime ) ==
                           time_from_string( "2011-05-13 2:00:00" )  );

   CPPUNIT_ASSERT_MESSAGE( "Timeperiod 7:4",
                           tpReftime.refto( time_from_string( "2011-05-12 2:00:00"), fromTime ) ==
                           time_from_string( "2011-05-12 2:00:00" )  );

   CPPUNIT_ASSERT_MESSAGE( "Timeperiod 7:5",
                           tpReftime.refto( time_from_string( "2011-05-12 21:00:00"), fromTime ) ==
                           time_from_string( "2011-05-13 2:00:00" )  );

   CPPUNIT_ASSERT_MESSAGE( "Timeperiod 7:6",
                           tpReftime.refto( time_from_string( "2011-05-12 3:00:00"), fromTime ) ==
                           time_from_string( "2011-05-13 2:00:00" )  );

   tpReftime = TimePeriode( time_duration( 8, 0, 0 ), time_duration( 12, 0, 0 ) );
   CPPUNIT_ASSERT_MESSAGE( "Timeperiod 7:7",
                           (tpReftime.refto( time_from_string( "2011-05-12 7:00:00"), fromTime ) ==
                            time_from_string( "2011-05-12 12:00:00" )) &&
                            fromTime == time_from_string( "2011-05-12 08:00:00") );

   CPPUNIT_ASSERT_MESSAGE( "Timeperiod 7:8",
                           (tpReftime.refto( time_from_string( "2011-05-12 10:00:00"), fromTime ) ==
                            time_from_string( "2011-05-12 12:00:00" )) &&
                            fromTime == time_from_string( "2011-05-12 08:00:00") );
   CPPUNIT_ASSERT_MESSAGE( "Timeperiod 7:9",
                           (tpReftime.refto( time_from_string( "2011-05-12 14:00:00"), fromTime ) ==
                            time_from_string( "2011-05-13 12:00:00" ) ) &&
                            fromTime == time_from_string( "2011-05-13 08:00:00") );

   CPPUNIT_ASSERT_MESSAGE( "Timeperiod 7:10",
                           tpReftime.refto( time_from_string( "2011-05-12 8:00:00"), fromTime ) ==
                           time_from_string( "2011-05-12 12:00:00" )  );

   CPPUNIT_ASSERT_MESSAGE( "Timeperiod 7:11",
                           tpReftime.refto( time_from_string( "2011-05-12 12:00:00"), fromTime ) ==
                           time_from_string( "2011-05-12 12:00:00" )  );
}

void
ConfigTest::testExpire(){
	wdb2ts::config::ActionParam theConf;
	ptime refTime(pts("2016-05-04 09:40:38"));

	//Test default configuration.
	// - ref is NearestModelTimeResolution
	// - model_resolution = 3600 seconds
	// - expire_rand = 0
	// - expire is undef, ie INT_MAX
	wdb2ts::ExpireConfig conf=wdb2ts::ExpireConfig::readConf(theConf);

	CPPUNIT_ASSERT(conf.ref_ == wdb2ts::ExpireConfig::NearestModelTimeResolution);
	CPPUNIT_ASSERT(conf.modelTimeResolution_ == 3600);
	CPPUNIT_ASSERT(conf.expireRand_ == 0);
	CPPUNIT_ASSERT(conf.expire_ == INT_MAX);
	ptime expire = conf.expire(refTime);

	CPPUNIT_ASSERT(expire == pts("2016-05-04 10:00:00"));

	//Test that we get an random addition at most 120 seconds into the future from refTime
	theConf["expire_rand"]=Value(120);
	conf=wdb2ts::ExpireConfig::readConf(theConf);
	CPPUNIT_ASSERT( conf.expireRand_==120);

	for( int i=0; i<100; ++i) {
		expire = conf.expire(refTime);
		CPPUNIT_ASSERT( expire>=pts("2016-05-04 10:00:00") && expire<pts("2016-05-04 10:02:00"));
	}

	//Test clear the config and set a model_resolution to 450 seconds.
	theConf.clear();
	theConf["model_resolution"]=Value(450);

	conf=wdb2ts::ExpireConfig::readConf(theConf);

	CPPUNIT_ASSERT(conf.ref_ == wdb2ts::ExpireConfig::NearestModelTimeResolution);
	CPPUNIT_ASSERT(conf.modelTimeResolution_ == 450);
	CPPUNIT_ASSERT(conf.expireRand_ == 0);
	CPPUNIT_ASSERT(conf.expire_ == INT_MAX);
	expire = conf.expire(refTime);

	CPPUNIT_ASSERT(expire == pts("2016-05-04 09:45:00"));

	//Test expire, the reference should be set to Now and expire should be
	//refTime + conf.expire_
	theConf["expire"]=Value(60);
	conf=wdb2ts::ExpireConfig::readConf(theConf);
	CPPUNIT_ASSERT(conf.ref_ == wdb2ts::ExpireConfig::Now);
	CPPUNIT_ASSERT(conf.expire_ == 60);
	CPPUNIT_ASSERT(conf.modelTimeResolution_ == 450);
	CPPUNIT_ASSERT(conf.expireRand_ == 0);

	expire = conf.expire(refTime);

	CPPUNIT_ASSERT(expire == pts("2016-05-04 09:41:38"));
}
