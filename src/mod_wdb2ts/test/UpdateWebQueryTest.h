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


#ifndef __UPDATE_WEB_QUERY_TEST_H__
#define __UPDATE_WEB_QUERY_TEST_H__

#include <cppunit/extensions/HelperMacros.h>

/**
 * Testsuite for WciWebQuery.
 */
class UpdateWebQueryTest : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE( UpdateWebQueryTest );
	CPPUNIT_TEST( testReferenceTimeAndVersion );
	CPPUNIT_TEST( testVersionOnly );
	CPPUNIT_TEST( testEnableDisable );
	CPPUNIT_TEST_SUITE_END();
public:
	
	UpdateWebQueryTest();
	virtual ~UpdateWebQueryTest();
	
	virtual void setUp();
	virtual void tearDown();
	
	void testReferenceTimeAndVersion();
	void testVersionOnly();
	void testEnableDisable();

private:

};

#endif /*WCIWEBQUERYTEST_H_*/