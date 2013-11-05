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


#ifndef __ProjectionTest_H__
#define __ProjectionTest_H__

#include <cppunit/extensions/HelperMacros.h>

/**
 * Testsuite for MiProjection.
 */
class MiProjectionTest : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE( MiProjectionTest );
	//CPPUNIT_TEST( testFullQuery );
	//CPPUNIT_TEST( testNullQuery );
	CPPUNIT_TEST( testLambert );
	CPPUNIT_TEST( testObTran );
	CPPUNIT_TEST( testVectorReprojection );
	CPPUNIT_TEST( testAtThePoles );
	CPPUNIT_TEST_SUITE_END();
public:
	
	MiProjectionTest();
	virtual ~MiProjectionTest();
	
	virtual void setUp();
	virtual void tearDown();

	void testLambert();
	void testObTran();
	void testVectorReprojection();
	void testAtThePoles();
private:

};

#endif
