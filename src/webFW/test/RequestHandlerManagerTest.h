#ifndef __REQUESTHANDLERMANAGERTEST_H__
#define __REQUESTHANDLERMANAGERTEST_H__

#include <cppunit/extensions/HelperMacros.h>
#include <DefaultRequestHandlerManager.h>

class RequestHandlerManagerTest : public CppUnit::TestFixture
{
   CPPUNIT_TEST_SUITE( RequestHandlerManagerTest );
   CPPUNIT_TEST( testDecodePath );
   CPPUNIT_TEST( testFindRequestHandlerPath );
   CPPUNIT_TEST( testDefaultRequestHandlerPath );
   CPPUNIT_TEST( testRemoveRequestHandlerPath );
   CPPUNIT_TEST( testRemoveDefaultRequestHandlerPath );
   CPPUNIT_TEST( testRequestHandler );
   CPPUNIT_TEST_SUITE_END();

   protected:
      void testDecodePath();
      void testFindRequestHandlerPath();
      void testDefaultRequestHandlerPath();
      void testRemoveRequestHandlerPath();
      void testRemoveDefaultRequestHandlerPath();
      void testRequestHandler();
      
   public:
      void setUp();
      void tearDown(); 

};





#endif
