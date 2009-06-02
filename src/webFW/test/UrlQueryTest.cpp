#include <cppunit/config/SourcePrefix.h>
#include <exception.h>
#include <UrlQueryTest.h>
#include <ptimeutil.h>

CPPUNIT_TEST_SUITE_REGISTRATION( UrlQueryTest );

using namespace std;
using namespace webfw;

void
UrlQueryTest::
setUp()
{
	// NOOP
}

void
UrlQueryTest::
tearDown() 
{
	// NOOP
}

void 
UrlQueryTest::
testUrlQueryUnescape()
{
   CPPUNIT_ASSERT_MESSAGE( "UrlQuery::unescape 1",
                           UrlQuery::unescape("Dette er en string") == "Dette er en string" );
   
   CPPUNIT_ASSERT_MESSAGE( "UrlQuery::unescape 3",
                            UrlQuery::unescape("%20Dette er en string") == " Dette er en string" );
   
   CPPUNIT_ASSERT_MESSAGE( "UrlQuery::unescape 4",
                            UrlQuery::unescape("Dette er en string%20") == "Dette er en string " );

   CPPUNIT_ASSERT_MESSAGE( "UrlQuery::unescape 5",
                            UrlQuery::unescape("Dette%20er%20en%20string") == "Dette er en string" );

   CPPUNIT_ASSERT_MESSAGE( "UrlQuery::unescape 6",
                            UrlQuery::unescape("") == "" );

   CPPUNIT_ASSERT_MESSAGE( "UrlQuery::unescape 7",
                            UrlQuery::unescape("%20") == " " );

   CPPUNIT_ASSERT_MESSAGE( "UrlQuery::unescape 8",
   		                   UrlQuery::unescape("%3c") == "<" );
   
   CPPUNIT_ASSERT_MESSAGE( "UrlQuery::unescape 9",
     		                   UrlQuery::unescape("%3C") == "<" );

   CPPUNIT_ASSERT_THROW_MESSAGE( "UrlQuery::unescape 10",
        		                      UrlQuery::unescape("%3") == "<",
        		                      std::range_error);

   CPPUNIT_ASSERT_THROW_MESSAGE( "UrlQuery::unescape 10",
       		                      UrlQuery::unescape("%3k") == "<",
       		                      std::logic_error);

}

void 
UrlQueryTest::
testUrlQueryEscape()
{
	CPPUNIT_ASSERT_MESSAGE( "UrlQuery::escape 1",
	                           UrlQuery::escape("Dette er en string") == "Dette%20er%20en%20string" );
	   
	CPPUNIT_ASSERT_MESSAGE( "UrlQuery::escape 3",
	                         UrlQuery::escape(" Dette er en string") == "%20Dette%20er%20en%20string" );
	
	CPPUNIT_ASSERT_MESSAGE( "UrlQuery::escape 4",
	                         UrlQuery::escape("2009-04-29T08:23:00") == "2009%2d04%2d29T08%3a23%3a00" );
	
	CPPUNIT_ASSERT_MESSAGE( "UrlQuery::escape 5",
		                      UrlQuery::escape("max air temperature,min air presure") == "max%20air%20temperature%2cmin%20air%20presure" );

	CPPUNIT_ASSERT_MESSAGE( "UrlQuery::escape 6",
			                   UrlQuery::escape("/path/to?param=val") == "%2fpath%2fto%3fparam%3dval" );
	
	CPPUNIT_ASSERT_MESSAGE( "UrlQuery::escape 7",
			                   UrlQuery::escape("with\rhmm") == "with%0dhmm" );
}

void 
UrlQueryTest::
testUrlQueryEncode()
{
	using namespace miutil;
	
	string path("/my/path");
	string s;
	UrlQuery query;
	UrlQuery queryDecoded;
 	
	boost::posix_time::ptime dt = ptimeFromIsoString( "2009-04-29T08:23:00" );
	boost::posix_time::ptime resDt;
	
	query.setValue( "date", dt );
	
	s = query.encode( path );
	
	CPPUNIT_ASSERT_MESSAGE( "UrlQuery::encode 1",
	                         s  == "/my/path?date=2009%2d04%2d29T08%3a23%3a00Z" );

	queryDecoded.decode( s, true );
	CPPUNIT_ASSERT_MESSAGE( "UrlQuery::encode 2",
		                      queryDecoded.asPTime( "date" ) == dt );
	
	query.setValue( "param", "\"air temperature, max\",min air presure");
	s = query.encode( path );

	CPPUNIT_ASSERT_MESSAGE( "UrlQuery::encode 3",
	                         s  == "/my/path?date=2009%2d04%2d29T08%3a23%3a00Z;param=%22air%20temperature%2c%20max%22%2cmin%20air%20presure" );
	
	queryDecoded.decode( s, true );
	
	CPPUNIT_ASSERT_MESSAGE( "UrlQuery::encode 4",
		                      queryDecoded.asPTime( "date" ) == dt );

	CPPUNIT_ASSERT_MESSAGE( "UrlQuery::encode 5",
		                      queryDecoded.asString( "param" ) == "\"air temperature, max\",min air presure" );
}



void 
UrlQueryTest::
testUrlQuery()
{
	UrlQuery query;
	
	//Use & as separator between params
	query.decode("param=value with space&param2=value1&param3=value2");
	
	CPPUNIT_ASSERT_MESSAGE( "UrlQuery::decode 1",
			                   query.asString( "param" ) == "value with space" );
	CPPUNIT_ASSERT_MESSAGE( "UrlQuery::decode 2",
			                   query.asString( "param2" ) == "value1" );
	CPPUNIT_ASSERT_MESSAGE( "UrlQuery::decode 3",
			                   query.asString( "param3" ) == "value2" );

	//Use ; as separator between params
	query.decode("param=value with space;param2=value1;param3=value2");
		
	CPPUNIT_ASSERT_MESSAGE( "UrlQuery::decode 2.1",
			                   query.asString( "param" ) == "value with space" );
	CPPUNIT_ASSERT_MESSAGE( "UrlQuery::decode 2.2",
			                   query.asString( "param2" ) == "value1" );
	CPPUNIT_ASSERT_MESSAGE( "UrlQuery::decode 2.3",
			                   query.asString( "param3" ) == "value2" );

	
	//Use the hex escape value %20 (spaces) in the value. 
	query.decode("param=value%20with%20space;param2=value1;param3=value2");
			
	CPPUNIT_ASSERT_MESSAGE( "UrlQuery::decode 3.1",
			                   query.asString( "param" ) == "value with space" );
	CPPUNIT_ASSERT_MESSAGE( "UrlQuery::decode 3.2",
				                   query.asString( "param2" ) == "value1" );
	CPPUNIT_ASSERT_MESSAGE( "UrlQuery::decode 3.3",
			                   query.asString( "param3" ) == "value2" );
	
}


