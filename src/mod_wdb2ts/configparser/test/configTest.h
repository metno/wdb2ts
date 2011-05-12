#ifndef __CONFIG_TEST_H__
#define __CONFIG_TEST_H__

#include <cppunit/extensions/HelperMacros.h>

class ConfigTest : public CppUnit::TestFixture
{
   CPPUNIT_TEST_SUITE( ConfigTest );
   CPPUNIT_TEST( testTimePeriod );
   CPPUNIT_TEST_SUITE_END();

   protected:
      void testTimePeriod();
      
   public:
      void setUp();
      void tearDown(); 

};





#endif
