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


#include <metlibs/diField/diProjection.h>
#include <SymbolTest.h>
#include <SymbolGenerator.h>

CPPUNIT_TEST_SUITE_REGISTRATION( SymbolTest );

using namespace std;
using namespace wdb2ts;

SymbolTest::
SymbolTest()
{
	// NOOP
}

SymbolTest::
~SymbolTest()
{
	// NOOP
}

void
SymbolTest::
setUp()
{
	// NOOP
}

void
SymbolTest::
tearDown()
{
	// NOOP
}

void
SymbolTest::
testRain()
{

}

#if 0
void
WciWebQueryTest::
testNullQuery()
{
}

void
WciWebQueryTest::
testLevelQuery()
{
	WebQuery wq;
	string prefix = "long=10;lat=60;";
	ostringstream q;

	q << prefix << "level=2m";
	
	wq = WebQuery::decodeQuery( q.str() );
	CPPUNIT_ASSERT( "exact 2 height above ground" == wq.getLevel().toWdbLevelspec() );

	q.str("");
	q << prefix << "level=2 m";
	wq = WebQuery::decodeQuery( q.str() );
	CPPUNIT_ASSERT( "exact 2 height above ground" == wq.getLevel().toWdbLevelspec() );

	q.str("");
	q << prefix << "level=2%20m";
	wq = WebQuery::decodeQuery( q.str() );
	CPPUNIT_ASSERT( "exact 2 height above ground" == wq.getLevel().toWdbLevelspec() );

	q.str("");
	q << prefix << "level=-2m";
	wq = WebQuery::decodeQuery( q.str() );
	CPPUNIT_ASSERT( "exact 2 depth" == wq.getLevel().toWdbLevelspec() );

	q.str("");
	q << prefix << "level=-2 m";
	wq = WebQuery::decodeQuery( q.str() );
	CPPUNIT_ASSERT( "exact 2 depth" == wq.getLevel().toWdbLevelspec() );
}
#endif
