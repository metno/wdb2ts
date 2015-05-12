#include <iostream>
#include <cmath>
#include <float.h>
#include <proj_api.h>

using namespace std;

double
angle( double u, double v )
{
	double ret = 90 - std::atan2(v, u) * RAD_TO_DEG;

	if (ret < 0)
		ret += 360;
	if (360 < ret)
		ret -= 360;

	return ret;
}

double
angle2( double u_, double v_, bool turn=false )
{
	double fTurn;
	double length;
	double direction;

	if( ! turn )
		fTurn = 180;

	length = std::sqrt(u_*u_ + v_*v_);

	if( length>0.0001) {
		direction = angle( u_, v_ ) - fTurn;
		cerr << " ( " << direction << " ) ";
		if( direction > 360 ) direction -=360;
		else if( direction < 0   ) direction +=360;
	} else {
		direction = 0;
		length = 0;
	}

	return direction;
}

double
direction( double u, double v, bool turn=false )
{
	double deg=180./3.141592654;
	double fTurn;
	double length;
	double direction;

	if( turn )
		fTurn = 90.0;
	else
		fTurn = 270.0;

	length = sqrt(u*u + v*v);

	if( length>0.0001) {
		direction = fTurn - deg*atan2( v, u );
		if( direction > 360 ) direction -=360;
		if( direction < 0   ) direction +=360;
	} else {
		direction = 0;
	}
	return direction;
}

double
myatan( double y, double x )
{
	if( x == 0 ) {
		if( y > 0 )
			return M_PI/2;
		else
			return 270*DEG_TO_RAD;
	}

	double ret = atan( y/x );

	if( x < 0 )
		ret += M_PI;

	if( ret < 0 )
		ret += 2*M_PI;
	else if ( ret > (2*M_PI) )
		ret -= 2*M_PI;

	return ret;
}

double
myatan2( double y, double x )
{
	double ret = atan2( y, x );

	if( ret < 0 )
		ret += 2*M_PI;
	else if ( ret > (2*M_PI) )
		ret -= 2*M_PI;

	return ret;
}




int
main( int argn, char **argv )
{
	double y=5.0;
	double x=5.0;

	cerr << "tan:   " << myatan(y,x)*RAD_TO_DEG << endl;
	cerr << "atan2: " << myatan2( y, x )*RAD_TO_DEG << endl;
	cerr << "-----------------------------------------\n";

	cerr << "tan:   " << myatan( -y, x )*RAD_TO_DEG << endl;
	cerr << "atan2: " << myatan2( -y, x )*RAD_TO_DEG << endl;
	cerr << "-----------------------------------------\n";

	cerr << "tan:   " << myatan( y, -x )*RAD_TO_DEG << endl;
	cerr << "atan2: " << myatan2( y, -x )*RAD_TO_DEG << endl;
	cerr << "-----------------------------------------\n";

	cerr << "tan:   " << myatan( -y, -x )*RAD_TO_DEG << endl;
	cerr << "atan2: " << myatan2( -y, -x )*RAD_TO_DEG << endl;
	cerr << "-----------------------------------------\n";

	cerr << "tan:   " << myatan( y, 0 )*RAD_TO_DEG << endl;
	cerr << "atan2: " << myatan2( y, 0 )*RAD_TO_DEG << endl;
	cerr << "-----------------------------------------\n";

	cerr << "tan:   " << myatan( -y, 0 )*RAD_TO_DEG << endl;
	cerr << "atan2: " << myatan2( -y, 0 )*RAD_TO_DEG << endl;
	cerr << "-----------------------------------------\n";

	cerr << "tan:   " << myatan( HUGE_VAL, HUGE_VAL )*RAD_TO_DEG << endl;
	cerr << "atan2: " << myatan2( HUGE_VAL, HUGE_VAL )*RAD_TO_DEG << endl;
	cerr << "-----------------------------------------\n";

	cerr << "tan:   " << myatan( HUGE_VAL, x )*RAD_TO_DEG << endl;
	cerr << "atan2: " << myatan2( HUGE_VAL, x )*RAD_TO_DEG << endl;
	cerr << "-----------------------------------------\n";

	cerr << "tan:   " << myatan( 0.000000001, 0 )*RAD_TO_DEG << endl;
	cerr << "atan2: " << myatan2( 0.000000001, 0 )*RAD_TO_DEG << endl;
	cerr << "-----------------------------------------\n";

	cerr << "tan:   " << myatan( -0.000000001, 0 )*RAD_TO_DEG << endl;
	cerr << "atan2: " << myatan2( -0.000000001, 0 )*RAD_TO_DEG << endl;
	cerr << "-----------------------------------------\n";

	cerr << "DBL_MIN: " << DBL_MIN << endl;
	cerr << "-DBL_MIN: " << -DBL_MIN << endl;

	cerr << "====================================================================\n";
	x = 3;
	y = 9;

	cerr << "1: " << atan2( y, x ) << " 2: " << (M_PI/2 - atan2( x, y )) << endl;
	cerr << "1: " << atan2( -y, x ) << " 2: " << (M_PI/2 - atan2( x, -y )) << endl;
	cerr << "1: " << atan2( y, -x ) << " 2: " << (M_PI/2 - atan2( -x, y )) << endl;
	cerr << "1: " << atan2( -y, -x )+2*M_PI << " 2: " << (M_PI/2 - atan2( -x, -y))/*-2*M_PI*/ << endl;

	cerr << "direction: " << direction( x, y )  << "  angle2: " << angle2( x, y ) << " diff: " <<   angle2( x, y ) - direction( x, y ) << endl;
	cerr << "direction: " << direction( -x, y ) << "  angle2: " << angle2( -x, y ) << " diff: " << angle2( -x, y )- direction( -x, y ) << endl;
	cerr << "direction: " << direction( x, -y ) << "  angle2: " << angle2( x, -y ) << " diff: " << angle2( x, -y )- direction( x, -y ) << endl;
	cerr << "direction: " << direction( -x, -y )<< "  angle2: " << angle2( -x, -y ) << " diff: " << angle2( -x, -y )- direction( -x, -y ) << endl;

}


