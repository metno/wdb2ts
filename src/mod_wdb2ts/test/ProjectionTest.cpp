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
#include <ProjectionTest.h>
#include <ProjectionHelper.h>

CPPUNIT_TEST_SUITE_REGISTRATION( MiProjectionTest );

using namespace std;
using namespace wdb2ts;

MiProjectionTest::
MiProjectionTest()
{
	// NOOP
}

MiProjectionTest::
~MiProjectionTest()
{
	// NOOP
}

void
MiProjectionTest::
setUp()
{
	// NOOP
}

void
MiProjectionTest::
tearDown()
{
	// NOOP
}

void
MiProjectionTest::
testLambert()
{
	const char *lcc="+proj=lcc +lon_0=15 +lat_1=63 +lat_2=63 +R=6.371e+06 +units=m +no_defs";
	const char *ob_tran="+proj=ob_tran +o_proj=longlat +lon_0=-24 +o_lat_p=23.5 +a=6367470.0 +no_defs";

	MiProjection geographic;
	MiProjection lambert=ProjectionHelper::createProjection(-897499, 7.22338e+06, 2500, 2500,
				lcc );



	geographic.makeGeographic();
	CPPUNIT_ASSERT( MiProjection::lambert == lambert.getProjectionType() );
	CPPUNIT_ASSERT( MiProjection::geographic == geographic.getProjectionType() );

	cerr << "LAMBERT   : " << lambert << endl;
	cerr << "GEOGRAPHIC: " << geographic << endl;

	float lat=61.0;
	float lon=7.0;
	CPPUNIT_ASSERT( lambert.xyconvert( geographic, lat, lon ) );

	cerr << "lat = " << lat << " lon = " << lon << endl;

	double dlat=61.0;
	double dlon=7.0;
	CPPUNIT_ASSERT( lambert.convertFromGeographicToXY( dlat, dlon ) );
	cerr << "LAMBERT: lat = " << dlat << " lon = " << dlon << endl;

	MiProjection obTran=ProjectionHelper::createProjection(-6.739, -16.039, 0.036, 0.036,
			"+proj=ob_tran +o_proj=longlat +lon_0=-24 +o_lat_p=23.5 +a=6367470.0 +no_defs" );

	MiProjection obTranProj( lcc );
	cerr << "ob_tran: " << obTran << endl;
	cerr << "ob_tran: proj from milib: " << obTran.createProjDefinition(false) << endl;

	lat=61.0;
	lon=7.0;
	CPPUNIT_ASSERT( obTran.xyconvert( geographic, lat, lon ) );

	cerr << "ob_tran: milib: lat = " << dlat << " lon = " << dlon << endl;
	dlat=61.0;
	dlon=7.0;
	CPPUNIT_ASSERT( obTranProj.convertFromGeographicToXY( dlat, dlon ) );
	cerr << "ob_tran:  proj: lat = " << dlat << " lon = " << dlon << endl;


	Projection diObTran( ob_tran, 1, 1 );
	MiProjection miObTran( ob_tran );
	cerr << "diField: " << diObTran << endl;
	dlat=61.0;
	dlon=7.0;
	float flat=61.0;
	float flon=7.0;
	diObTran.convertFromGeographic( 1, &flon, &flat );
	cerr << "diObTran: proj: lat = " << flat << " lon = " << flon << endl;
	miObTran.convertFromGeographicToXY( dlat, dlon );
	cerr << "miObTran: proj: lat = " << dlat << " lon = " << dlon << endl;

	float gs[6]={-6.703, -16.003, 0.036, 0.036, -24, 66.5};
	double resx, resy;
	diObTran.set_mi_gridspec( 3, gs, resx, resy );
	cerr << "diObTran: proj (mi) : " << diObTran << endl;
	flat=61.0;
	flon=7.0;
	diObTran.convertFromGeographic( 1, &flon, &flat );
	cerr << "diObTran: proj (mi): lat = " << flat << " lon = " << flon << endl;


}

void
MiProjectionTest::
testObTran()
{
	cerr << "\n\n\n ----- testObTran -------\n\n\n";
	MiProjection::ProjectionType gst=MiProjection::spherical_rotated;
	float gs[6]={-6.703, -16.003, 0.036, 0.036, -24, 66.5};
	const char *ob_tran="+proj=ob_tran +o_proj=longlat +lon_0=-24 +o_lat_p=23.5 +R=6371000 +no_defs";

	MiProjection obTranMi( gs, gst, ob_tran );
	MiProjection geographic;
	geographic.makeGeographic();

	CPPUNIT_ASSERT( MiProjection::geographic == geographic.getProjectionType() );

	cerr << "GEOGRAPHIC: " << geographic << endl;
	cerr << "ob_tran   : " << obTranMi << endl;

	double dlat, dlon;
	float flat, flon;

	Projection diObTran( ob_tran, 1, 1 );
	MiProjection miObTran( ob_tran );
	cerr << "diField: " << diObTran << endl;
	dlat=61.0;
	dlon=7.0;
	flat=61.0;
	flon=7.0;
	diObTran.convertFromGeographic( 1, &flon, &flat );
	cerr << "diObTran: proj: lat = " << flat << " lon = " << flon << endl;
	miObTran.convertFromGeographicToXY( dlat, dlon );
	cerr << "miObTran: proj: lat = " << dlat << " lon = " << dlon << endl;

	double resx, resy;
	diObTran.set_mi_gridspec( 3, gs, resx, resy, false );
	cerr << "diObTran: proj (mi) : " << diObTran << " resx: " << resx << " resy: " << resy << endl;
	Projection diObTranScale( "+proj=ob_tran +o_proj=longlat +lon_0=-24 +o_lat_p=23.5 +x_0=0.116989 +y_0=0.279305 +R=6371000 +no_defs", 1, 1 );

	flat=61.0;
	flon=7.0;
	diObTran.convertFromGeographic( 1, &flon, &flat );
	cerr << "diObTran: proj (mi): lat = " << flat << " lon = " << flon << endl;

	flat=61.0;
	flon=7.0;
	diObTranScale.convertFromGeographic( 1, &flon, &flat );
	cerr << "diObTranScale: (mi): lat = " << flat << " lon = " << flon << endl;


	flat=61.0;
	flon=7.0;

	CPPUNIT_ASSERT( obTranMi.xyconvert( geographic, flat, flon ) );

	cerr << "obTranMi: mi       : lat = " << flat << " lon = " << flon << endl;
}


void
MiProjectionTest::
testVectorReprojection()
{
	cerr << "\n\n\n ----- testVectorReprojection -------\n\n\n";
	MiProjection::ProjectionType gst=MiProjection::spherical_rotated;
	float gs[6]={-6.703, -16.003, 0.036, 0.036, -24, 66.5};
	const char *ob_tran="+proj=ob_tran +o_proj=longlat +lon_0=-24 +o_lat_p=23.5 +a=6367470.0 +no_defs";
	const char *lcc="+proj=lcc +lon_0=15 +lat_1=63 +lat_2=63 +R=6.371e+06 +units=m +no_defs";
	MiProjection obTranMi( gs, gst, ob_tran );
	MiProjection geographic;
	MiProjection miLambert( lcc );
	geographic.makeGeographic();

	CPPUNIT_ASSERT( MiProjection::geographic == geographic.getProjectionType() );

	cerr << "GEOGRAPHIC: " << geographic << endl;
	cerr << "ob_tran MI: " << obTranMi << endl;

	float flat, flon;
	double dlat, dlon;
	double resx, resy;
	float fU, fV;
	float fDD, fFF;
	double dU, dV;
	double dDD, dFF;

	Projection	diObTranGS;
	Projection diObTran( ob_tran, 1, 1 );
	MiProjection miObTran( ob_tran );
	MiProjection miObTranGS( gs, MiProjection::spherical_rotated, ob_tran );


	cerr << "diObTran:   " << diObTran << endl;

	diObTranGS.set_mi_gridspec( 3, gs, resx, resy );
	cerr << "diObTranGS: " << diObTranGS << endl;

	dlat=61.0;
	dlon=7.0;
	flat=61.0;
	flon=7.0;

	diObTran.convertFromGeographic( 1, &flon, &flat );
	cerr << "diObTran: proj: lat = " << flat << " lon = " << flon << endl;
	miObTran.convertFromGeographicToXY( dlat, dlon );
	cerr << "miObTran: proj: lat = " << dlat << " lon = " << dlon << endl;


	flat=61.0;
	flon=7.0;
	diObTranGS.convertFromGeographic( 1, &flon, &flat );
	cerr << "diObTranGS: proj (mi): lat = " << flat << " lon = " << flon << endl;

	flat=61.0;
	flon=7.0;
	diObTranGS.convertFromGeographic( 1, &flon, &flat );
	cerr << "diObTranGS:          : lat = " << flat << " lon = " << flon << endl;

	flat=68.1531;
	flon=14.6491;
	fU = 4.68000030517578;
	fV = 10.577000617981;

	CPPUNIT_ASSERT( geographic.convertToDirectionAndLength( miObTranGS, flat, flon, fU, fV, fDD, fFF, false) );

	cerr << "miObTranGS: dd: " << fDD << " ff: " << fFF << endl;


	dlat=68.1531;
	dlon=14.6491;
	dU = 4.68000030517578;
	dV = 10.577000617981;

	CPPUNIT_ASSERT( geographic.convertToDirectionAndLength_( miObTran, dlat, dlon, dU, dV, dDD, dFF, false) );

	cerr << "miObTran: dd: " << dDD << " ff: " << dFF << endl;

	dlat=68.1531;
	dlon=14.6491;
	dU = 8.3120002746582;
	dV = 5.61900043487549;

	CPPUNIT_ASSERT( geographic.convertToDirectionAndLength_( miLambert, dlat, dlon, dU, dV, dDD, dFF, false) );

	cerr << "miLambert:  dd: " << dDD << " ff: " << dFF << endl;
	dlat=68.1531;
	dlon=14.6491;
	dU = 4.68000030517578;
	dV = 10.577000617981;


	CPPUNIT_ASSERT( geographic.directionAndLength( 1, &dlon, &dlat, &dU, &dV, miObTran, &dDD, &dFF ) );
	//geographic.directionAndLength( 1, &dlon, &dlat, &dU, &dV, miObTran, &dDD, &dFF );
	cerr << "miObTran: (hmm) dd: " << dDD << " ff: " << dFF << endl;

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
