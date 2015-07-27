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

#include <fstream>
#include <math.h>
#include <Logger4cpp.h>
#include <ProjectionTest.h>
#include <ProjectionHelper.h>
#include <readcsv.h>
#include <ptimeutil.h>

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

struct XYComponents {
	boost::posix_time::ptime time;
	float x;
	float y;
	XYComponents( const boost::posix_time::ptime &t, float x_, float y_ )
	:time( t ), x( x_ ), y( y_ ){}
};


bool
readVectorFile( const std::string &file, std::list<XYComponents> &xy )
{
	list<vector<string> > windCsv;
	ifstream windFile( file.c_str() );
	float val;
	boost::posix_time::ptime t;

	if( ! windFile.is_open() )
		return false;

	if( ! readCSV( windFile, windCsv, '|' ) )
		return false;

	map< boost::posix_time::ptime, map<string, float> > data;

	//Collect all components.
	for( list<vector<string> >::const_iterator it=windCsv.begin();
		  it != windCsv.end(); ++it ) {
		if( it->size() < 4 )
			continue;
		t = miutil::ptimeFromIsoString( (*it)[1] );
		if( t.is_special() )
			continue;
		try {
			val = boost::lexical_cast<float>( (*it)[3] );
			data[t][(*it)[0]]=val;
		}
		catch( ... ) {
			continue;
		}
	}

	for( map< boost::posix_time::ptime, map<string, float> >::iterator it=data.begin();
			it != data.end(); ++it ) {
		if( it->second.size() != 2 )
			continue;

		xy.push_back( XYComponents( it->first, it->second["x wind"], it->second["y wind"]) );
	}

	return xy.size() != 0;
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
	MiProjection miLambert( lcc );

	double dlat=61.0;
	double dlon=7.0;

	CPPUNIT_ASSERT( miLambert.convertFromGeographicToXY( dlat, dlon ) );
	CPPUNIT_ASSERT( equals( dlat, 8132156.888, 3) && equals( dlon, -430412.0, 0 ) );

	dlat=-61.0;
	dlon=7.0;
	CPPUNIT_ASSERT( miLambert.convertFromGeographicToXY( dlat, dlon ) );
   CPPUNIT_ASSERT( equals( dlat, -26747158.49, 2) && equals( dlon, -4792203.251, 3 ) );

	dlat=61.0;
	dlon=-7.0;
	CPPUNIT_ASSERT( miLambert.convertFromGeographicToXY( dlat, dlon ) );
   CPPUNIT_ASSERT( equals( dlat, 8306373.233, 3) && equals( dlon, -1163676.581, 3 ) );


   dlat=-61.0;
   dlon=-7.0;
   CPPUNIT_ASSERT( miLambert.convertFromGeographicToXY( dlat, dlon ) );
   CPPUNIT_ASSERT( equals( dlat, -24807434.59, 2) && equals( dlon, -12956369.1, 1 ) );
}

void
MiProjectionTest::
testObTran()
{
	const char *ob_tran="+proj=ob_tran +o_proj=longlat +lon_0=-24 +o_lat_p=23.5 +R=6371000 +no_defs";
	double dlat, dlon;
	float flat, flon;

	MiProjection miObTran( ob_tran );

	dlat=0.0;
	dlon=0.0;
	CPPUNIT_ASSERT( miObTran.convertFromGeographicToXY( dlat, dlon ) );
	CPPUNIT_ASSERT( equals( dlat, -0.99320, 5) && equals( dlon, 0.84041, 5 ) );
	CPPUNIT_ASSERT( miObTran.convertFromXYToGeographic( dlat, dlon ) );
	CPPUNIT_ASSERT( equals( dlat, 0.00, 2) && equals( dlon, 0.00, 2 ) );

	dlat=61.0;
	dlon=7.0;
	CPPUNIT_ASSERT( miObTran.convertFromGeographicToXY( dlat, dlon ) );
	CPPUNIT_ASSERT( equals( dlat, -0.03235, 5) && equals( dlon, 0.25250, 5 ) );
	CPPUNIT_ASSERT( miObTran.convertFromXYToGeographic( dlat, dlon ) );
	CPPUNIT_ASSERT( equals( dlat, 61.00, 2) && equals( dlon, 7.00, 2 ) );

	dlat=-61.0;
	dlon=7.0;
	CPPUNIT_ASSERT( miObTran.convertFromGeographicToXY( dlat, dlon ) );
	CPPUNIT_ASSERT( equals( dlat, -0.81810, 5) && equals( dlon, 2.76768, 5 ) );
	CPPUNIT_ASSERT( miObTran.convertFromXYToGeographic( dlat, dlon ) );
	CPPUNIT_ASSERT( equals( dlat, -61.00, 2) && equals( dlon, 7.00, 2 ) );

	dlat=61.0;
	dlon=-7.0;
	CPPUNIT_ASSERT( miObTran.convertFromGeographicToXY( dlat, dlon ) );
	CPPUNIT_ASSERT( equals( dlat, -0.07649, 5) && equals( dlon, 0.142644, 5 ) );
	CPPUNIT_ASSERT( miObTran.convertFromXYToGeographic( dlat, dlon ) );
	CPPUNIT_ASSERT( equals( dlat, 61.00, 2) && equals( dlon, -7.00, 2 ) );

	dlat=-61.0;
	dlon=-7.0;
	CPPUNIT_ASSERT( miObTran.convertFromGeographicToXY( dlat, dlon ) );
	CPPUNIT_ASSERT( equals( dlat, -0.88502, 5) && equals( dlon, 2.91585, 5 ) );
	CPPUNIT_ASSERT( miObTran.convertFromXYToGeographic( dlat, dlon ) );
	//cerr << "miObTran: proj: lat = " << setprecision( 10 ) << dlat << " lon = " << setprecision( 10 ) << dlon << endl;
	CPPUNIT_ASSERT( equals( dlat, -61.00, 2) && equals( dlon, -7.00, 2 ) );
}


void
MiProjectionTest::
testVectorReprojection()
{
	WEBFW_USE_LOGGER( "handler" );
	WEBFW_SET_LOGLEVEL( log4cpp::Priority::WARN );

	double dlat, dlon;
	double dU, dV;
	double dDD, dFF;

	MiProjection geographic;
	MiProjection obtran( "+proj=ob_tran +o_proj=longlat +lon_0=-24 +o_lat_p=23.5 +a=6367470.0 +no_defs" );

	dlat=68.1531;
	dlon=14.6491;
	dU = 4.68000030517578;
	dV = 10.577000617981;
	CPPUNIT_ASSERT( geographic.convertToDirectionAndLength( obtran, dlat, dlon, dU, dV, dDD, dFF, false) );
	CPPUNIT_ASSERT( equals( 239.178, dDD, 2 ) );
	CPPUNIT_ASSERT( equals( 11.5661, dFF, 2 ) );

	//Projection: geographic
	//Placename: Ecmwf Grid 3
	MiProjection geo(	"+proj=longlat +a=6367470.0 +towgs84=0,0,0 +no_defs" );

	dlat=68.1531;
	dlon=14.6491;
	dU = -4.248046875;
	dV = 1.44552612304688;
	CPPUNIT_ASSERT( geographic.convertToDirectionAndLength( geo, dlat, dlon, dU, dV, dDD, dFF, false) );
	CPPUNIT_ASSERT( equals(108.792, dDD, 2 ) );
	CPPUNIT_ASSERT( equals(4.49, dFF, 2 ) );

	//Projection: lcc (lambert)
	//Placename: arome_norway_grid_scandinavia"
	MiProjection lambert( "+proj=lcc +lon_0=15 +lat_1=63 +lat_2=63 +R=6.371e+06 +units=m +no_defs" );
	dlat=68.1531;
	dlon=14.6491;
	dU = -4.99100017547607;
	dV = 5.70400047302246;
	CPPUNIT_ASSERT( geographic.convertToDirectionAndLength( lambert, dlat, dlon, dU, dV, dDD, dFF, false) );
	CPPUNIT_ASSERT( equals( 138.501, dDD, 2 ) );
	CPPUNIT_ASSERT( equals( 7.58, dFF, 2 ) );
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

	//test at the North Pole.
	double dlat;
	double dlon;
	double dU = -4.99100017547607;
	double dV = 6.70400047302246;
	double dDD, dFF;

	dlat=90;
	dlon=0;
	CPPUNIT_ASSERT( geographic.convertToDirectionAndLength( lambert, dlat, dlon, dU, dV, dDD, dFF, false) );
	CPPUNIT_ASSERT( equals( 143.333, dDD, 2 ) && equals(8.35785, dFF, 2) );

	CPPUNIT_ASSERT( geographic.convertToDirectionAndLength( obTran, dlat, dlon, dU, dV, dDD, dFF, false) );
	CPPUNIT_ASSERT( equals( 169.402, dDD, 2 ) && equals(8.35785, dFF, 2) );

	CPPUNIT_ASSERT( geographic.convertToDirectionAndLength( lonLat, dlat, dlon, dU, dV, dDD, dFF, false) );
	CPPUNIT_ASSERT( equals( 143.333, dDD, 2 ) && equals(8.35785, dFF, 2) );

	//This is also at the north pole, and the result should be the same as above.
	dlat=90;
	dlon = 30;
	CPPUNIT_ASSERT( geographic.convertToDirectionAndLength( lambert, dlat, dlon, dU, dV, dDD, dFF, false) );
	CPPUNIT_ASSERT( equals( 143.333, dDD, 2 ) && equals(8.35785, dFF, 2) );

	CPPUNIT_ASSERT( geographic.convertToDirectionAndLength( obTran, dlat, dlon, dU, dV, dDD, dFF, false) );
	CPPUNIT_ASSERT( equals( 169.402, dDD, 2 ) && equals(8.35785, dFF, 2) );

	CPPUNIT_ASSERT( geographic.convertToDirectionAndLength( lonLat, dlat, dlon, dU, dV, dDD, dFF, false) );
	CPPUNIT_ASSERT( equals( 143.333, dDD, 2 ) && equals(8.35785, dFF, 2) );

	//Test south pole
	dlat = -90;
	dlon = 0;

	CPPUNIT_ASSERT( geographic.convertToDirectionAndLength( obTran, dlat, dlon, dU, dV, dDD, dFF, false) );
	CPPUNIT_ASSERT( equals( 257.696, dDD, 2 ) && equals(8.35785, dFF, 2) );

	CPPUNIT_ASSERT( geographic.convertToDirectionAndLength( lonLat, dlat, dlon, dU, dV, dDD, dFF, false) );
	CPPUNIT_ASSERT( equals( 143.333, dDD, 2 ) && equals(8.35785, dFF, 2) );
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

void
MiProjectionTest::
testIrland()
{
	MiProjection geographic;
	MiProjection irlandProj("+proj=lcc +lat_0=53.5 +lon_0=5 +lat_1=53.5 +lat_2=53.5 +no_defs +R=6.371e+06" );
	std::list<XYComponents> xyWind;
	string windFile( string(TESTDIR_MOD_WDB2TS) + "/wind_components.csv");

	CPPUNIT_ASSERT( readVectorFile( windFile, xyWind ) );

	double latitude=53.4214;
	double longitude=-8.2700;
	double u, v;
	double direction, speed;

	for( std::list<XYComponents>::iterator it=xyWind.begin(); it != xyWind.end(); ++it ) {
		u = it->x;
		v = it->y;
		geographic.convertToDirectionAndLength( irlandProj, latitude, longitude, u, v, direction, speed );
		cerr << "( " << u << " , " << v << " ) speed: " << speed << " direction: " << direction << endl;
	}

	return;

	u=1000; v=0;
	geographic.convertToDirectionAndLength( irlandProj, latitude, longitude, u, v, direction, speed );
	cerr << "( " << u << " , " << v << " ) speed: " << speed << " direction: " << direction << endl;

	u=0; v=1000;
	geographic.convertToDirectionAndLength( irlandProj, latitude, longitude, u, v, direction, speed );
	cerr << "( " << u << " , " << v << " ) speed: " << speed << " direction: " << direction << endl;

	u=1000; v=1000;
	geographic.convertToDirectionAndLength( irlandProj, latitude, longitude, u, v, direction, speed );
	cerr << "( " << u << " , " << v << " ) speed: " << speed << " direction: " << direction << endl;

	u=0.0001; v=0.00000001;
	geographic.convertToDirectionAndLength( irlandProj, latitude, longitude, u, v, direction, speed );
	cerr << "( " << u << " , " << v << " ) speed: " << speed << " direction: " << direction << endl;

	u=0.00000001; v=0.0001;
	geographic.convertToDirectionAndLength( irlandProj, latitude, longitude, u, v, direction, speed );
	cerr << "( " << u << " , " << v << " ) speed: " << speed << " direction: " << direction << endl;

	u=0.0001; v=0.0001;
	geographic.convertToDirectionAndLength( irlandProj, latitude, longitude, u, v, direction, speed );
	cerr << "( " << u << " , " << v << " ) speed: " << speed << " direction: " << direction << endl;


	return;
	for( std::list<XYComponents>::iterator it=xyWind.begin(); it != xyWind.end(); ++it ) {
		u = it->x;
		v = it->y;
		geographic.convertToDirectionAndLength( irlandProj, latitude, longitude, u, v, direction, speed );
		cerr << "( " << u << " , " << v << " ) speed: " << speed << " direction: " << direction << endl;
	}
}


