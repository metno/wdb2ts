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
 * @addtogroup mod_wdb2ts
 * @{
 */
/**
 * @file
 * Implementation of the UrlParamString class.
  */
#ifdef HAVE_CONFIG_H
#include <wdb2ts_config.h>
#endif
// CLASS
#include <UrlParamPolygon.h>
// PROJECT INCLUDES
//
// SYSTEM INCLUDES
#include <boost/regex.hpp>
#include <boost/assign/list_of.hpp>
#include <sstream>

using namespace std;

namespace {

void
creatPolygon( wdb2ts::LocationPointList &points, string &selectPolygon )
{
	wdb2ts::LocationPoint lp( *points.begin() );

	if( points.size() == 1 )
		points.push_back( lp ); //Must be a closed polygon, this is a point.
	else if( *points.rbegin() != lp )
		points.push_back( lp ); //Make it a closed polygon.

	ostringstream ost;
	ost.setf(ios::floatfield, ios::fixed);
	ost.precision(5);

	ost <<"POLYGON((";

	wdb2ts::LocationPointList::const_iterator it = points.begin();
	ost << it->longitude() << " " << it->latitude();

	for( ++it; it != points.end(); ++it )
		ost << "," << it->longitude() << " " << it->latitude();

	ost << "))";

	selectPolygon = ost.str();
}

bool
decodePolygon( const std::string &toDecode_, wdb2ts::LocationPointList &points, string &selectPart, string &errorMsg )
{
	try{
		wdb2ts::LocationPoint::decodePointList( toDecode_, points );
	}
	catch( const exception &ex) {
		errorMsg = ex.what();
		return false;
	}

	creatPolygon( points, selectPart );

	return true;
}



}




namespace wdb2ts {

UrlParamPolygon::
UrlParamPolygon( int protocol, const LocationPointList &DefValue )
    : value_( DefValue )
{
	if( ! DefValue.empty() ) {
		valid_ = true;
		creatPolygon( value_, selectPart_ );
		defSelectPart_ = selectPart_;
		defValue_ = value_;
	}

	defValid_ = valid_;
}

UrlParamPolygon::
UrlParamPolygon(  )
	: UrlParam( protocol ), defValid_( valid_ )
{
}
   
void
UrlParamPolygon::
clean()
{
	value_ = defValue_;
	valid_ = defValid_;
	selectPart_ = defSelectPart_;
}

void 
UrlParamPolygon::
decode( const std::string &toDecode_ )
{
	string msg;
	if( ! decodePolygon( toDecode_, value_, selectPart_, msg ) ) {
		valid_ = false;
		throw logic_error( msg.c_str() );
	}

	valid_ = true;}

} // namespace

/**
 * @}
 *
 * @}
 */
