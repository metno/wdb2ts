#ifndef __URL_QUERY_TEST_H__
#define __URL_QUERY_TEST_H__

#include <cppunit/extensions/HelperMacros.h>
#include <UrlQuery.h>

class UrlQueryTest : public CppUnit::TestFixture
{
   CPPUNIT_TEST_SUITE( UrlQueryTest );
   CPPUNIT_TEST( testUrlQueryUnescape );
   CPPUNIT_TEST( testUrlQueryEscape );
   CPPUNIT_TEST( testUrlQueryEncode );
   CPPUNIT_TEST( testUrlQuery );
   CPPUNIT_TEST_SUITE_END();

   protected:
      void testUrlQueryUnescape();
      void testUrlQuery();
      void testUrlQueryEscape();
      void testUrlQueryEncode();
      
   public:
      void setUp();
      void tearDown(); 

};





#endif
