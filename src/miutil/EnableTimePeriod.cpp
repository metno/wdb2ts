/*
    wdb - weather and water data storage

    Copyright (C) 2007 met.no

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

#include <vector>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include "EnableTimePeriod.h"

using namespace std;

namespace miutil {

namespace {
int
getMonth( const string &month )
{
	const char *months[]={"jan","feb","mar","apr","may","jun","jul","aug","sep","oct","nov","dec", 0};

	for( int i=0; months[i]; ++i ) {
		if( strcasecmp( months[i], month.c_str() ) == 0 )
			return i+1;
	}
	return 0;
}
}


EnableTimePeriod::
EnableTimePeriod( int dayFrom, int monthFrom,  int dayTo, int monthTo)
	: fromDay( dayFrom ), fromMonth( monthFrom ), toDay( dayTo ), toMonth( monthTo )
{

}

EnableTimePeriod
EnableTimePeriod::
parse( const std::string &duration_ )
{
	using namespace boost::gregorian;
	using namespace boost::algorithm;

	typedef std::vector<string> Array;
	int m1,m2,d1,d2;
	string duration( trim_copy( duration_ ) );
	std::string number;
	Array aDuration;
	Array fromSpec, toSpec;

	if( boost::algorithm::to_lower_copy( duration ) == "all" )
		return EnableTimePeriod(); //Enable all times.

	split( aDuration, duration, boost::is_any_of("/") );

	if( aDuration.size()  != 2 )
		return EnableTimePeriod( -1 ); //Invalid specification

	split( fromSpec, trim_copy<const string>( aDuration[0] ), boost::is_any_of("-") );
	split( toSpec, trim_copy<const string>( aDuration[1] ), boost::is_any_of("-") );

	if( fromSpec.size() != 2  || toSpec.size() != 2 )
		return EnableTimePeriod( -1 ); //Invalid specification

	try {
		m1 = getMonth( trim_copy( fromSpec[0] ) );
		m2 = getMonth( trim_copy( toSpec[0] ) );
		d1 = boost::lexical_cast<int>( trim_copy( fromSpec[1] ) );
		d2 = boost::lexical_cast<int>( trim_copy( toSpec[1] ) );

		if( d1<1 || d1>31 || d2<1 || d2>31 ||
			m1<1 || m1>12 || m2<1 || m2>12)
			return EnableTimePeriod( -1 ); //Invalid specification
	}
	catch (const boost::bad_lexical_cast &e) {
		return EnableTimePeriod( -1 ); //Invalid specification
	}

	//cerr << "Date: " << duration_ << " (" << m1 << " - " << d1 << ")/(" << m2 << " - " << d2 << ")\n";
	return EnableTimePeriod( d1, m1, d2, m2 );
}

boost::gregorian::date_period
EnableTimePeriod::
enabledPeriod( const boost::gregorian::date &refTime )const
{
	using namespace boost::gregorian;
	int year=refTime.year();
	int tDay, fDay;

	bool outer=false;

	if( fromDay < 0 ) //Invalid, disable.
		return date_period( date( pos_infin ), date( neg_infin ) );
	else if( fromDay == 0 ) //All days is in the periode.
		return date_period( date( neg_infin ), date( pos_infin ) );

	if( fromMonth > toMonth || ( fromMonth == toMonth && fromDay > toDay) )
		outer = true;

	//If the given days is greater than the last day in month, "snap" to the last day.

	if( outer ) {
		date dTo( year, toMonth, 1);
		date dFrom( year+1, fromMonth, 1);
		dTo = dTo.end_of_month();
		dFrom = dFrom.end_of_month();
		tDay = toDay>dTo.day()?dTo.day().as_number():toDay;
		fDay = fromDay>dFrom.day()?dFrom.day().as_number():fromDay;

		return date_period( date( year, fromMonth, fDay ), date(year+1, toMonth, tDay) );
	}else {
		date dTo( year, toMonth, 1);
		date dFrom( year, fromMonth, 1);
		dTo = dTo.end_of_month();
		dFrom = dFrom.end_of_month();
		tDay = toDay>dTo.day()?dTo.day().as_number():toDay;
		fDay = fromDay>dFrom.day()?dFrom.day().as_number():fromDay;

		return date_period( date( year, fromMonth, fDay ), date(year, toMonth, tDay) );
	}
};

bool
EnableTimePeriod::
isEnabled( const boost::posix_time::ptime &t, const boost::gregorian::date &refTime)const
{
	using namespace boost::gregorian;

	if( ! valid() )
		return false;

	date_period p = enabledPeriod( refTime );

	if( p.is_null() )
		return true;

	if( p.contains(t.date() ) )
		return true;
	else
		return false;
}


}
