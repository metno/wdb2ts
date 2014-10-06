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

#include <math.h>
#include <metlibs/diField/diProjection.h>
#include <Logger4cpp.h>
#include <ProjectionTest.h>
#include <ProjectionHelper.h>

CPPUNIT_TEST_SUITE_REGISTRATION( MiProjectionTest );

using namespace std;
using namespace wdb2ts;

namespace {

int
pow10( int p ) {
	int r=1;
	for( int i=0; i<p; ++i )
		r *= 10;
	return r;
}


long double
asLong( double f, int des ){
	return roundl( f * pow10( des ) );
}


bool
equals( double a1, double a2, int des ) {
	return asLong( a1, des ) == asLong( a2, des );
}


}



MiProjectionTest::
MiProjectionTest()
{
	//NOOP
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
	WEBFW_USE_LOGGER( "handler" );
	WEBFW_SET_LOGLEVEL( log4cpp::Priority::ERROR );
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

//	cerr << "LAMBERT   : " << lambert << endl;
//	cerr << "GEOGRAPHIC: " << geographic << endl;

	float lat=61.0;
	float lon=7.0;
	CPPUNIT_ASSERT( lambert.xyconvert( geographic, lat, lon ) );

//	cerr << "lat = " << lat << " lon = " << lon << endl;

	double dlat=61.0;
	double dlon=7.0;
	CPPUNIT_ASSERT( lambert.convertFromGeographicToXY( dlat, dlon ) );
//	cerr << "LAMBERT: lat = " << dlat << " lon = " << dlon << endl;

	MiProjection obTran=ProjectionHelper::createProjection(-6.739, -16.039, 0.036, 0.036,
			"+proj=ob_tran +o_proj=longlat +lon_0=-24 +o_lat_p=23.5 +a=6367470.0 +no_defs" );

	MiProjection obTranProj( lcc );
//	cerr << "ob_tran: " << obTran << endl;
//	cerr << "ob_tran: proj from milib: " << obTran.createProjDefinition(false) << endl;

	lat=61.0;
	lon=7.0;
	CPPUNIT_ASSERT( obTran.xyconvert( geographic, lat, lon ) );

//	cerr << "ob_tran: milib: lat = " << dlat << " lon = " << dlon << endl;
	dlat=61.0;
	dlon=7.0;
	CPPUNIT_ASSERT( obTranProj.convertFromGeographicToXY( dlat, dlon ) );
//	cerr << "ob_tran:  proj: lat = " << dlat << " lon = " << dlon << endl;


	Projection diObTran( ob_tran, 1, 1 );
	MiProjection miObTran( ob_tran );
//	cerr << "diField: " << diObTran << endl;
	dlat=61.0;
	dlon=7.0;
	float flat=61.0;
	float flon=7.0;
	diObTran.convertFromGeographic( 1, &flon, &flat );
//	cerr << "diObTran: proj: lat = " << flat << " lon = " << flon << endl;
	miObTran.convertFromGeographicToXY( dlat, dlon );
//	cerr << "miObTran: proj: lat = " << dlat << " lon = " << dlon << endl;

	float gs[6]={-6.703, -16.003, 0.036, 0.036, -24, 66.5};
	double resx, resy;
	diObTran.set_mi_gridspec( 3, gs, resx, resy );
//	cerr << "diObTran: proj (mi) : " << diObTran << endl;
	flat=61.0;
	flon=7.0;
	diObTran.convertFromGeographic( 1, &flon, &flat );
//	cerr << "diObTran: proj (mi): lat = " << flat << " lon = " << flon << endl;


}

void
MiProjectionTest::
testObTran()
{
//	cerr << "\n\n\n ----- testObTran -------\n\n\n";
	MiProjection::ProjectionType gst=MiProjection::spherical_rotated;
	float gs[6]={-6.703, -16.003, 0.036, 0.036, -24, 66.5};
	const char *ob_tran="+proj=ob_tran +o_proj=longlat +lon_0=-24 +o_lat_p=23.5 +R=6371000 +no_defs";

	MiProjection obTranMi( gs, gst, ob_tran );
	MiProjection geographic;
	geographic.makeGeographic();

	CPPUNIT_ASSERT( MiProjection::geographic == geographic.getProjectionType() );

//	cerr << "GEOGRAPHIC: " << geographic << endl;
//	cerr << "ob_tran   : " << obTranMi << endl;

	double dlat, dlon;
	float flat, flon;

	Projection diObTran( ob_tran, 1, 1 );
	MiProjection miObTran( ob_tran );
//	cerr << "diField: " << diObTran << endl;
	dlat=61.0;
	dlon=7.0;
	flat=61.0;
	flon=7.0;
	diObTran.convertFromGeographic( 1, &flon, &flat );
//	cerr << "diObTran: proj: lat = " << flat << " lon = " << flon << endl;
	miObTran.convertFromGeographicToXY( dlat, dlon );
//	cerr << "miObTran: proj: lat = " << dlat << " lon = " << dlon << endl;

	double resx, resy;
	diObTran.set_mi_gridspec( 3, gs, resx, resy, false );
//	cerr << "diObTran: proj (mi) : " << diObTran << " resx: " << resx << " resy: " << resy << endl;
	Projection diObTranScale( "+proj=ob_tran +o_proj=longlat +lon_0=-24 +o_lat_p=23.5 +x_0=0.116989 +y_0=0.279305 +R=6371000 +no_defs", 1, 1 );

	flat=61.0;
	flon=7.0;
	diObTran.convertFromGeographic( 1, &flon, &flat );
//	cerr << "diObTran: proj (mi): lat = " << flat << " lon = " << flon << endl;

	flat=61.0;
	flon=7.0;
	diObTranScale.convertFromGeographic( 1, &flon, &flat );
//	cerr << "diObTranScale: (mi): lat = " << flat << " lon = " << flon << endl;


	flat=61.0;
	flon=7.0;

	CPPUNIT_ASSERT( obTranMi.xyconvert( geographic, flat, flon ) );

//	cerr << "obTranMi: mi       : lat = " << flat << " lon = " << flon << endl;
}


void
MiProjectionTest::
testVectorReprojection()
{
	WEBFW_USE_LOGGER( "handler" );
	WEBFW_SET_LOGLEVEL( log4cpp::Priority::WARN );
	//cerr << "\n\n\n ----- testVectorReprojection -------\n\n\n";

	float flat, flon;
	double dlat, dlon;
	float fU, fV;
	float fDD, fFF;
	double dU, dV;
	double dDD, dFF;
	double resx, resy;

	MiProjection geographic;
	geographic.makeGeographic();

	//Projection: ob_tran (spherical_rotated)
	//placename: proff grid
	MiProjection obtran = ProjectionHelper::createProjection(
					-6.739, -16.039, 0.036, 0.036,
					"+proj=ob_tran +o_proj=longlat +lon_0=-24 +o_lat_p=23.5 +a=6367470.0 +no_defs"
				);

	flat=68.1531;
	flon=14.6491;
	fU = 4.68000030517578;
	fV = 10.577000617981;

	CPPUNIT_ASSERT( geographic.convertToDirectionAndLengthMiLib( obtran, flat, flon, fU, fV, fDD, fFF, false) );

//	cerr << "obtran (miLib): dd: " << fDD << " ff: " << fFF << endl;
	dlat=68.1531;
	dlon=14.6491;
	dU = 4.68000030517578;
	dV = 10.577000617981;

	CPPUNIT_ASSERT( geographic.convertToDirectionAndLength( obtran, dlat, dlon, dU, dV, dDD, dFF, false) );
	CPPUNIT_ASSERT( equals( 239.178, dDD, 2 ) );
	CPPUNIT_ASSERT( equals( 11.5661, dFF, 2 ) );
	//cerr << "obtran  (proj): " << dDD << " ff: " << dFF << endl;

	//Projection: geographic
	//Placename: Ecmwf Grid 3
	MiProjection geo = ProjectionHelper::createProjection(
			-179.75, -90, 0.25, 0.25,
			"+proj=longlat +a=6367470.0 +towgs84=0,0,0 +no_defs" );

	flat=68.1531;
	flon=14.6491;
	fU = -4.248046875;
	fV = 1.44552612304688;
	CPPUNIT_ASSERT( geographic.convertToDirectionAndLengthMiLib( geo, flat, flon, fU, fV, fDD, fFF, false) );
	//cerr << "geographic (miLib): dd: " << fDD << " ff: " << fFF << endl;

	dlat=68.1531;
	dlon=14.6491;
	dU = -4.248046875;
	dV = 1.44552612304688;
	CPPUNIT_ASSERT( geographic.convertToDirectionAndLength( geo, dlat, dlon, dU, dV, dDD, dFF, false) );
	CPPUNIT_ASSERT( equals(108.792, dDD, 2 ) );
	CPPUNIT_ASSERT( equals(4.49, dFF, 2 ) );
	//cerr << "geographic (proj) : dd: " << dDD << " ff: " << dFF << endl;


	//Projection: lcc (lambert)
	//Placename: arome_norway_grid_scandinavia"
	MiProjection lambert =ProjectionHelper::createProjection(
			-897499, 7.22338e+06, 2500, 2500,
			"+proj=lcc +lon_0=15 +lat_1=63 +lat_2=63 +R=6.371e+06 +units=m +no_defs");

	flat=68.1531;
	flon=14.6491;
	fU = -4.99100017547607;
	fV = 5.70400047302246;
	CPPUNIT_ASSERT( geographic.convertToDirectionAndLengthMiLib( lambert, flat, flon, fU, fV, fDD, fFF, false) );

	//cerr << "lambert (miLib): dd: " << fDD << " ff: " << fFF << endl;

	dlat=68.1531;
	dlon=14.6491;
	dU = -4.99100017547607;
	dV = 5.70400047302246;
	CPPUNIT_ASSERT( geographic.convertToDirectionAndLength( lambert, dlat, dlon, dU, dV, dDD, dFF, false) );
	CPPUNIT_ASSERT( equals( 138.501, dDD, 2 ) );
	CPPUNIT_ASSERT( equals( 7.58, dFF, 2 ) );
	//cerr << "lambert (proj) : dd: " << dDD << " ff: " << dFF << endl;
}

void
MiProjectionTest::
testAtThePoles()
{
    const char *sPolar1="+proj=stere +lat_0=90 +lon_0=58 +lat_ts=60 +a=6371000 +units=m +no_defs";
    const char *sPpolar2="+proj=stere +lat_0=90 +lon_0=24 +lat_ts=60 +a=6371000 +units=m +no_defs";

    MiProjection geographic;
	MiProjection lambert( "+proj=lcc +lon_0=15 +lat_1=63 +lat_2=63 +R=6.371e+06 +units=m +no_defs" );
	MiProjection obTran( "+proj=ob_tran +o_proj=longlat +lon_0=-24 +o_lat_p=23.5 +a=6367470.0 +no_defs" );
	MiProjection lonLat( "+proj=longlat +a=6367470.0 +towgs84=0,0,0 +no_defs" );

	geographic.makeGeographic();

//	cerr << "------ testNorthPole -----------\n";
//	cerr << "OBTRAN    : " << obTran << endl;
//	cerr << "LAMBERT   : " << lambert << endl;
//	cerr << "GEOGRAPHIC: " << geographic << endl;

	double dlat;
	double dlon;
	double dU = -4.99100017547607;
	//double dV = 5.70400047302246;
	double dV = 6.70400047302246;
	double dDD, dFF;

	//Test north pole
	dlat=90;
	dlon=0;

	CPPUNIT_ASSERT( geographic.convertToDirectionAndLength( lambert, dlat, dlon, dU, dV, dDD, dFF, false) );
	CPPUNIT_ASSERT( equals( 143.333, dDD, 2 ) && equals(8.35785, dFF, 2) );
//	cerr << " --- lambert: DD: " << dDD << " FF: " << dFF << endl;

	CPPUNIT_ASSERT( geographic.convertToDirectionAndLength( obTran, dlat, dlon, dU, dV, dDD, dFF, false) );
	CPPUNIT_ASSERT( equals( 169.402, dDD, 2 ) && equals(8.35785, dFF, 2) );
//	cerr << " --- obTran: DD: " << dDD << " FF: " << dFF << endl;

	CPPUNIT_ASSERT( geographic.convertToDirectionAndLength( lonLat, dlat, dlon, dU, dV, dDD, dFF, false) );
	CPPUNIT_ASSERT( equals( 143.333, dDD, 2 ) && equals(8.35785, dFF, 2) );
//	cerr << " --- lonLat: DD: " << dDD << " FF: " << dFF << endl;

	//This is also at the north pole, and the result should be the same as above.
	dlat=90;
	dlon = 30;
	CPPUNIT_ASSERT( geographic.convertToDirectionAndLength( lambert, dlat, dlon, dU, dV, dDD, dFF, false) );
//	cerr << " --- lambert: DD: " << dDD << " FF: " << dFF << endl;
	CPPUNIT_ASSERT( equals( 143.333, dDD, 2 ) && equals(8.35785, dFF, 2) );


	CPPUNIT_ASSERT( geographic.convertToDirectionAndLength( obTran, dlat, dlon, dU, dV, dDD, dFF, false) );
//	cerr << " --- obTran: DD: " << dDD << " FF: " << dFF << endl;
	CPPUNIT_ASSERT( equals( 169.402, dDD, 2 ) && equals(8.35785, dFF, 2) );


	CPPUNIT_ASSERT( geographic.convertToDirectionAndLength( lonLat, dlat, dlon, dU, dV, dDD, dFF, false) );
//	cerr << " --- lonLat: DD: " << dDD << " FF: " << dFF << endl;
	CPPUNIT_ASSERT( equals( 143.333, dDD, 2 ) && equals(8.35785, dFF, 2) );


//	cerr << " ----- south pole ------\n";
	//Test south pole
	dlat = -90;
	dlon = 0;

	CPPUNIT_ASSERT( geographic.convertToDirectionAndLength( obTran, dlat, dlon, dU, dV, dDD, dFF, false) );
	CPPUNIT_ASSERT( equals( 257.696, dDD, 2 ) && equals(8.35785, dFF, 2) );
//	cerr << " --- obTran: DD: " << dDD << " FF: " << dFF << endl;

	CPPUNIT_ASSERT( geographic.convertToDirectionAndLength( lonLat, dlat, dlon, dU, dV, dDD, dFF, false) );
	CPPUNIT_ASSERT( equals( 143.333, dDD, 2 ) && equals(8.35785, dFF, 2) );
//	cerr << " --- lonLat: DD: " << dDD << " FF: " << dFF << endl;


}

void
MiProjectionTest::
testPolarStero()
{
	const char  *stero_proj="+proj=stere +lat_0=90 +lon_0=58 +lat_ts=60 +a=6371000 +units=m +no_defs";
	MiProjection geographic;
	MiProjection polarStereoProj( stero_proj );
	double dV, dU;
	double dlat, dlon;
	double direction, length;

	geographic.makeGeographic();

	dlat=63.65825;
	dlon=5.83059;
	dV=0.259999990463257; //x
	dU=0.100000001490116; //y

	CPPUNIT_ASSERT( geographic.convertToDirectionAndLength( polarStereoProj, dlat, dlon, dU, dV,direction, length) );
	CPPUNIT_ASSERT( equals( 148.868, direction, 2 ) );
	CPPUNIT_ASSERT( equals(0.278568 , length, 3 ) );

	dV=0; //x
	dU=0; //y
	CPPUNIT_ASSERT( geographic.convertToDirectionAndLength( polarStereoProj, dlat, dlon, dU, dV, direction, length) );
	CPPUNIT_ASSERT( equals( 0, direction, 2 ) );
	CPPUNIT_ASSERT( equals( 0, length, 3 ) );

	dlat=72.33712;
	dlon= 5.12818;
	dV=0.189999997615814; //x
	dU=0.0499999970197678; //y
	CPPUNIT_ASSERT( geographic.convertToDirectionAndLength( polarStereoProj, dlat, dlon, dU, dV, direction, length) );
	CPPUNIT_ASSERT( equals( 141.872, direction, 2 ) );
	CPPUNIT_ASSERT( equals(0.196469, length, 3 ) );
}
