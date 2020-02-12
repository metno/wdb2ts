#include <cppunit/config/SourcePrefix.h>
#include <exception.h>
#include <iostream>
#include <string>
#include <ptimeutil.h>
#include "EnableTimePeriod.h"
#include "ptimeutilTest.h"
#include "metfunctions.h"


CPPUNIT_TEST_SUITE_REGISTRATION( PtimeUtilTest );

using namespace std;
using namespace miutil;
using namespace boost::posix_time;

using std::string;

void
PtimeUtilTest::
setUp()
{
	// NOOP
}

void
PtimeUtilTest::
tearDown() 
{
	// NOOP
}
      
void 
PtimeUtilTest::
testPtimeFromIsoString()
{
	ptime pt;
	ptime ptTest;
	
	pt = time_from_string("2008-10-18 18:33:22");
	ptTest = ptimeFromIsoString( "2008-10-18 18:33:22" );
	//cerr << pt << " -> (pt) " << ptTest << endl;
	CPPUNIT_ASSERT_MESSAGE( "2008-10-18 18:33:22", pt == ptTest );
	
	pt = time_from_string("2008-10-18 18:33:00");
	ptTest = ptimeFromIsoString( "2008-10-18 18:33" );
	//cerr << pt << " -> (pt) " << ptTest << endl;
	CPPUNIT_ASSERT_MESSAGE( "2008-10-18 18:33", pt == ptTest );
	
	pt = time_from_string("2008-10-18 18:00:00");
	ptTest = ptimeFromIsoString( "2008-10-18 18" );
	//cerr << pt << " -> (pt) " << ptTest << endl;
	CPPUNIT_ASSERT_MESSAGE( "2008-10-18 18", pt == ptTest );
	
	pt = time_from_string("2008-10-18 00:00:00");
	ptTest = ptimeFromIsoString( "2008-10-18" );
	//cerr << pt << " -> (pt) " << ptTest << endl;
	CPPUNIT_ASSERT_MESSAGE( "2008-10-18", pt == ptTest );

	pt = time_from_string("2008-10-18 18:33:22");
	ptTest = ptimeFromIsoString( "2008-10-18T18:33:22" );
	//cerr << pt << " -> (pt) " << ptTest << endl;
	CPPUNIT_ASSERT_MESSAGE( "2008-10-18 18:33:22", pt == ptTest );
		
	pt = time_from_string("2008-10-18 18:33:00");
	ptTest = ptimeFromIsoString( "2008-10-18T18:33" );
	//cerr << pt << " -> (pt) " << ptTest << endl;
	CPPUNIT_ASSERT_MESSAGE( "2008-10-18T18:33", pt == ptTest );
	
	pt = time_from_string("2008-10-18 18:00:00");
	ptTest = ptimeFromIsoString( "2008-10-18T18" );
	//cerr << pt << " -> (pt) " << ptTest << endl;
	CPPUNIT_ASSERT_MESSAGE( "2008-10-18 18", pt == ptTest );
	
	pt = time_from_string("2008-10-18 18:33:22");
	ptTest = ptimeFromIsoString( "20081018T183322" );
	//cerr << pt << " -> (pt) " << ptTest << endl;
	CPPUNIT_ASSERT_MESSAGE( "2008-10-18 18:33:22", pt == ptTest );

	pt = time_from_string("2008-10-18 18:33:22");
	ptTest = ptimeFromIsoString( "20081018183322" );
	//cerr << pt << " -> (pt) " << ptTest << endl;
	CPPUNIT_ASSERT_MESSAGE( "2008-10-18 18:33:22", pt == ptTest );
		
	pt = time_from_string("2008-10-18 18:33:00");
	ptTest = ptimeFromIsoString( "200810181833" );
	//cerr << pt << " -> (pt) " << ptTest << endl;
	CPPUNIT_ASSERT_MESSAGE( "2008-10-18 18:33", pt == ptTest );
		
	pt = time_from_string("2008-10-18 18:00:00");
	ptTest = ptimeFromIsoString( "2008101818" );
	//cerr << pt << " -> (pt) " << ptTest << endl;
	CPPUNIT_ASSERT_MESSAGE( "2008-10-18 18", pt == ptTest );
		
	pt = time_from_string("2008-10-18 00:00:00");
	ptTest = ptimeFromIsoString( "20081018" );
	//cerr << pt << " -> (pt) " << ptTest << endl;
	CPPUNIT_ASSERT_MESSAGE( "2008-10-18", pt == ptTest );

	
	
	pt = time_from_string("2008-10-18 18:00:00");
	ptTest = ptimeFromIsoString( "2008-10-18T18:00:00+00" );
	//cerr << pt << " -> (pt) " << ptTest << endl;
	CPPUNIT_ASSERT_MESSAGE( "2008-10-18 18:00:00+00", pt == ptTest );

	pt = time_from_string("2008-10-18 18:00:00");
	ptTest = ptimeFromIsoString( "2008-10-18T18:00:00-00" );
	//cerr << pt << " -> (pt) " << ptTest << endl;
	CPPUNIT_ASSERT_MESSAGE( "2008-10-18 18:00:00-00", pt == ptTest );
	
	pt = time_from_string("2008-10-18 16:00:00");
	ptTest = ptimeFromIsoString( "2008-10-18T18:00:00+02" );
	//cerr << pt << " -> (pt) " << ptTest << endl;
	CPPUNIT_ASSERT_MESSAGE( "2008-10-18 16:00:00", pt == ptTest );
	
	pt = time_from_string("2008-10-18 16:00:00");
	ptTest = ptimeFromIsoString( "2008-10-18T18:00:00+0200" );
	//cerr << pt << " -> (pt) " << ptTest << endl;
	CPPUNIT_ASSERT_MESSAGE( "2008-10-18 18:00:00+0200", pt == ptTest );
		
	pt = time_from_string("2008-10-18 15:30:00");
	ptTest = ptimeFromIsoString( "2008-10-18T18:00:00+0230" );
	//cerr << pt << " -> (pt) " << ptTest << endl;
	CPPUNIT_ASSERT_MESSAGE( "2008-10-18 18:00:00+0230", pt == ptTest );

	pt = time_from_string("2008-10-18 20:00:00");
	ptTest = ptimeFromIsoString( "2008-10-18T18:00:00-02" );
	//cerr << pt << " -> (pt) " << ptTest << endl;
	CPPUNIT_ASSERT_MESSAGE( "2008-10-18 16:00:00-02", pt == ptTest );
	
	pt = time_from_string("2008-10-18 20:00:00");
	ptTest = ptimeFromIsoString( "2008-10-18T18:00:00-0200" );
	//cerr << pt << " -> (pt) " << ptTest << endl;
	CPPUNIT_ASSERT_MESSAGE( "2008-10-18 18:00:00-0200", pt == ptTest );
		
	pt = time_from_string("2008-10-18 20:30:00");
	ptTest = ptimeFromIsoString( "2008-10-18T18:00:00-0230" );
	//cerr << pt << " -> (pt) " << ptTest << endl;
	CPPUNIT_ASSERT_MESSAGE( "2008-10-18 18:00:00-0230", pt == ptTest );
	
	pt = time_from_string("2007-04-01 09:51:04");
   string rfc("Sun, 01 Apr 2007 09:51:04 GMT");
   ptTest = ptimeFromIsoString( rfc );
	//cerr << pt << " -> (pt) " << ptTest << endl;
	CPPUNIT_ASSERT_MESSAGE( "Sun, 1 Apr 2007 09:51:04 GMT", pt == ptTest );

}
void
PtimeUtilTest::
geologicalLocalTime()
{
   namespace m=miutil;
   namespace b=boost::posix_time;

   b::ptime pt=time_from_string("2007-04-01 09:51:04");
   b::ptime rt;

   rt = m::geologicalLocalTime( pt, 7.4 );
   CPPUNIT_ASSERT_MESSAGE( "utc 0", rt == time_from_string("2007-04-01 09:51:04") );

   rt = m::geologicalLocalTime( pt, 7.6 );
   CPPUNIT_ASSERT_MESSAGE( "utc +1", rt == time_from_string("2007-04-01 10:51:04") );

   rt = m::geologicalLocalTime( pt, -7.6 );
   CPPUNIT_ASSERT_MESSAGE( "utc -1", rt == time_from_string("2007-04-01 08:51:04") );

   rt = m::geologicalLocalTime( pt, 85 );
   CPPUNIT_ASSERT_MESSAGE( "utc +6", rt == time_from_string("2007-04-01 15:51:04") );

   rt = m::geologicalLocalTime( pt, -85 );
   CPPUNIT_ASSERT_MESSAGE( "utc -6", rt == time_from_string("2007-04-01 03:51:04") );
}

void 
PtimeUtilTest::
enableTimePeriod()
{
	using namespace boost::posix_time;
	using namespace boost::gregorian;
	ptime refTime = time_from_string("2014-06-06 10:35:45");
	EnableTimePeriod p;

	CPPUNIT_ASSERT_MESSAGE("All times", p.isEnabled( time_from_string("2014-10-06 10:35:45") ) );

	p = EnableTimePeriod::parse( "May-1/Aug-1");

	date_period dp = p.enabledPeriod( refTime.date() );
	CPPUNIT_ASSERT_MESSAGE("Unexpected, periode is null.", ! dp.is_null() );
	CPPUNIT_ASSERT( dp.begin() == date(2014, May, 1) && dp.last() == date(2014, Jul, 31) );

	CPPUNIT_ASSERT( ! p.isEnabled( time_from_string("2014-04-30 10:35:45"), refTime.date() ) );
	CPPUNIT_ASSERT( p.isEnabled( time_from_string("2014-05-01 10:35:45"), refTime.date() ) );
	CPPUNIT_ASSERT( ! p.isEnabled( time_from_string("2014-08-01 10:35:45"), refTime.date() ) );
	CPPUNIT_ASSERT( p.isEnabled( time_from_string("2014-06-01 10:35:45"), refTime.date() ) );


	p = EnableTimePeriod::parse( "Aug-1/May-1");

	dp = p.enabledPeriod( refTime.date() );

	//cerr << "Date period: " << dp << endl;
	CPPUNIT_ASSERT( dp.begin() == date(2014, Aug, 1) && dp.last() == date(2015, Apr, 30) );



}

void
PtimeUtilTest::
testRFC1123()
{
	ptime pt = time_from_string("2007-04-01 09:51:04");
   string rfc("Sun, 01 Apr 2007 09:51:04 GMT");

   
   ptime ptTest=rfc1123date( rfc );
   //cerr << pt << " -> (pt) " << ptTest << endl;
   CPPUNIT_ASSERT_MESSAGE( "Sun, 1 Apr 2007 09:51:04 GMT", pt == ptTest );
   
   string testRfc=rfc1123date( pt );
   //cerr << pt << " -> (pt) " << testRfc << endl;
   CPPUNIT_ASSERT_MESSAGE( "2007-04-01 09:51:04", rfc == testRfc );
}

void PtimeUtilTest::testToBeaufort(){
   string name;
   CPPUNIT_ASSERT(toBeaufort(0.17, name)==string("0") && name == string("Stille"));
	CPPUNIT_ASSERT(toBeaufort(0.24, name)==string("0") && name == string("Stille"));
	CPPUNIT_ASSERT(toBeaufort(0.25, name)==string("1") && name == string("Flau vind"));

	//cerr << "Code: " << toBeaufort(1.54, name) << " name: " << name << "\n";
	CPPUNIT_ASSERT(toBeaufort(1.0, name)==string("1") && name == string("Flau vind"));
	CPPUNIT_ASSERT(toBeaufort(1.54, name)==string("1") && name == string("Flau vind"));
	CPPUNIT_ASSERT(toBeaufort(1.55, name)==string("2") && name == string("Svak vind"));

	CPPUNIT_ASSERT(toBeaufort(2.0, name)==string("2") && name == string("Svak vind"));
	CPPUNIT_ASSERT(toBeaufort(3.34, name)==string("2") && name == string("Svak vind"));
	CPPUNIT_ASSERT(toBeaufort(3.35, name)==string("3") && name == string("Lett bris"));

	CPPUNIT_ASSERT(toBeaufort(4.0, name)==string("3") && name == string("Lett bris"));
	CPPUNIT_ASSERT(toBeaufort(5.44, name)==string("3") && name == string("Lett bris"));
	CPPUNIT_ASSERT(toBeaufort(5.45, name)==string("4") && name == string("Laber bris"));

	CPPUNIT_ASSERT(toBeaufort(6.0, name)==string("4") && name == string("Laber bris"));
	CPPUNIT_ASSERT(toBeaufort(7.94, name)==string("4") && name == string("Laber bris"));
	CPPUNIT_ASSERT(toBeaufort(7.95, name)==string("5") && name == string("Frisk bris"));

	CPPUNIT_ASSERT(toBeaufort(9.0, name)==string("5") && name == string("Frisk bris"));
	CPPUNIT_ASSERT(toBeaufort(10.74, name)==string("5") && name == string("Frisk bris"));
	CPPUNIT_ASSERT(toBeaufort(10.75, name)==string("6") && name == string("Liten kuling"));

	CPPUNIT_ASSERT(toBeaufort(12.0, name)==string("6") && name == string("Liten kuling"));
	CPPUNIT_ASSERT(toBeaufort(13.84, name)==string("6") && name == string("Liten kuling"));
	CPPUNIT_ASSERT(toBeaufort(13.85, name)==string("7") && name == string("Stiv kuling"));

	CPPUNIT_ASSERT(toBeaufort(14.0, name)==string("7") && name == string("Stiv kuling"));
	CPPUNIT_ASSERT(toBeaufort(17.14, name)==string("7") && name == string("Stiv kuling"));
	CPPUNIT_ASSERT(toBeaufort(17.15, name)==string("8") && name == string("Sterk kuling"));

	CPPUNIT_ASSERT(toBeaufort(19.0, name)==string("8") && name == string("Sterk kuling"));
	CPPUNIT_ASSERT(toBeaufort(20.74, name)==string("8") && name == string("Sterk kuling"));
	CPPUNIT_ASSERT(toBeaufort(20.75, name)==string("9") && name == string("Liten storm"));

	CPPUNIT_ASSERT(toBeaufort(23.0, name)==string("9") && name == string("Liten storm"));
	CPPUNIT_ASSERT(toBeaufort(24.44, name)==string("9") && name == string("Liten storm"));
	CPPUNIT_ASSERT(toBeaufort(24.45, name)==string("10") && name == string("Full storm"));

	CPPUNIT_ASSERT(toBeaufort(26.0, name)==string("10") && name == string("Full storm"));
	CPPUNIT_ASSERT(toBeaufort(28.44, name)==string("10") && name == string("Full storm"));
	CPPUNIT_ASSERT(toBeaufort(28.45, name)==string("11") && name == string("Sterk storm"));

	CPPUNIT_ASSERT(toBeaufort(30.0, name)==string("11") && name == string("Sterk storm"));
	CPPUNIT_ASSERT(toBeaufort(32.64, name)==string("11") && name == string("Sterk storm"));
	CPPUNIT_ASSERT(toBeaufort(32.65, name)==string("12") && name == string("Orkan"));
	CPPUNIT_ASSERT(toBeaufort(33.0, name)==string("12") && name == string("Orkan"));






}
