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

#ifndef __QueryMakerTest_h__
#define __QueryMakerTest_h__

// CPPUNIT defines
#include <cppunit/extensions/HelperMacros.h>

#include <ConfigParser.h>

using namespace wdb2ts::config;

/**
 * @addtogroup wdb2ts
 * @{
 * @addtogroup test
 * @{
 */
/**
 * @file
 * This file contains the QueryMakerTest class definition.
 */



/**
 * Testsuite for QueryMaker, as used by mod_wdb2ts
 */
class QueryMakerTest : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE( QueryMakerTest );
	CPPUNIT_TEST( test );
	CPPUNIT_TEST( test2 );
//	CPPUNIT_TEST( testUrlParamDataProviderOne );
//	CPPUNIT_TEST( testUrlParamDataProviderMany );
//	CPPUNIT_TEST( testUrlParamTimeSpecEmpty );
//	CPPUNIT_TEST( testUrlParamTimeSpecOne );
//	CPPUNIT_TEST( testUrlParamTimeSpecTwo );
	CPPUNIT_TEST_SUITE_END();
	
	std::string testdir;

	Config *loadConfig( const std::string &file );

public:
	QueryMakerTest();
	virtual ~QueryMakerTest();
	
	virtual void setUp();
	virtual void tearDown();
	
	// UrlParamDataProvider
	void test();
	void test2();
//	void testUrlParamDataProviderOne();
//	void testUrlParamDataProviderMany();
//	// UrlParamTimeSpec
//	void testUrlParamTimeSpecEmpty();
//	void testUrlParamTimeSpecOne();
//	void testUrlParamTimeSpecTwo();
	
};


/**
 * @}
 *
 * @}
 */

#endif /*URLPARAMTEST_H_*/
