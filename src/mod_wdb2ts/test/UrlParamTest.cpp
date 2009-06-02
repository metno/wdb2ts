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

/**
 * @addtogroup wdb2ts
 * @{
 * @addtogroup test
 * @{
 */
/**
 * @file
 * This file contains the UrlParamTest class implementation
 */

// CLASS INCLUDES
#include <UrlParamTest.h>
// PROJECT INCLUDES
#include <UrlParamDataProvider.h>
#include <UrlParamTimeSpec.h>
// SYSTEM INCLUDES
//

using namespace std;
using namespace wdb2ts;

CPPUNIT_TEST_SUITE_REGISTRATION( UrlParamTest );

UrlParamTest::UrlParamTest()
{
	// NOOP
}

UrlParamTest::~UrlParamTest()
{
	// NOOP
}

void UrlParamTest::setUp()
{
	// NOOP
}

void UrlParamTest::tearDown()
{
	// NOOP
}


void UrlParamTest::testUrlParamDataProviderEmpty()
{
	UrlParamDataProvider testParam;
	testParam.decode("");
	string wciParam = testParam.selectPart();
	string expResult = "NULL";
	
	CPPUNIT_ASSERT_EQUAL(  expResult, wciParam );
}

void UrlParamTest::testUrlParamDataProviderOne()
{
	UrlParamDataProvider testParam;
	testParam.decode("hirlam");
	string wciParam = testParam.selectPart();
	string expResult = "ARRAY[ 'hirlam' ]";
	
	CPPUNIT_ASSERT_EQUAL( expResult, wciParam );
}

void UrlParamTest::testUrlParamDataProviderMany()
{
	UrlParamDataProvider testParam;
	testParam.decode("hirlam,eps,bracknell,washington");
	string wciParam = testParam.selectPart();
	string expResult = "ARRAY[ 'hirlam', 'eps', 'bracknell', 'washington' ]";
	
	CPPUNIT_ASSERT_EQUAL( expResult, wciParam );
}


void UrlParamTest::testUrlParamTimeSpecEmpty()
{
	UrlParamTimeSpec testParam;
	CPPUNIT_ASSERT_THROW ( testParam.decode(""), std::logic_error );
	/*
	UrlParamTimeSpec testParam;
	testParam.decode("");
	string wciParam = testParam.selectPart();
	string expResult = "NULL";
	
	CPPUNIT_ASSERT_EQUAL(  expResult, wciParam );
	*/
}

void UrlParamTest::testUrlParamTimeSpecOne()
{
	UrlParamTimeSpec testParam;
	testParam.decode("2008-05-17T12:00:00");
	string wciParam = testParam.selectPart();
	string expResult = "('2008-05-17T12:00:00', '2008-05-17T12:00:00', 'exact')";
	
	CPPUNIT_ASSERT_EQUAL( expResult, wciParam );
}

void UrlParamTest::testUrlParamTimeSpecTwo()
{
	UrlParamTimeSpec testParam;
	testParam.decode("2008-05-17T12:00:00,2008-05-17T23:00:00");
	string wciParam = testParam.selectPart();
	string expResult = "('2008-05-17T12:00:00', '2008-05-17T23:00:00', 'exact')";
	
	CPPUNIT_ASSERT_EQUAL( expResult, wciParam );
}

/**
 * @}
 *
 * @}
 */
