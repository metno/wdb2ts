/*
 * MetaModelTest.h
 *
 *  Created on: May 23, 2016
 *      Author: borgem
 */

#ifndef SRC_MOD_WDB2TS_TEST_METAMODELTEST_H_
#define SRC_MOD_WDB2TS_TEST_METAMODELTEST_H_


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

#include <cppunit/extensions/HelperMacros.h>

/**
 * Testsuite for MetaModelTest.
 */
class MetaModelTest : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE( MetaModelTest );
	CPPUNIT_TEST( testNextRunSpecific );
	CPPUNIT_TEST( testNextRunRelativToLoadtime );
	CPPUNIT_TEST( testNextRunEvery );
	CPPUNIT_TEST_SUITE_END();
public:

	MetaModelTest();
	virtual ~MetaModelTest();

	virtual void setUp();
	virtual void tearDown();

	void testNextRunSpecific();
	void testNextRunRelativToLoadtime();
	void testNextRunEvery();
private:

};

#endif /* SRC_MOD_WDB2TS_TEST_METAMODELTEST_H_ */
