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

#include <sstream>
#include <splitstr.h>
#include <ptimeutil.h>
#include <Value.h>

using namespace std;

namespace miutil {

Value::
Value()
	: defined_( false )
{
}

Value::
Value( const Value &v)
	: value_( v.value_ ), defined_( v.defined_ )
{
}

Value::
Value( float f, int desimals )
	: defined_( false )
{
	ostringstream ost;
	ost.setf(ios::floatfield, ios::fixed);
	ost.precision(desimals);
	
	ost << f;
	value_ = ost.str();
	defined_ = true;
}

Value::
Value( double d, int desimals )
	: defined_( false )
{
	ostringstream ost;
	ost.setf(ios::floatfield, ios::fixed);
	ost.precision( desimals );
	
	ost << d;
	value_ = ost.str();
	defined_ = true;
}

Value::
Value( int i )
{
	ostringstream ost;
	ost << i;
	value_ = ost.str();
	defined_ = true;
}

Value::
Value( const boost::posix_time::ptime &pt )
{
	value_ = isotimeString( pt, true, true );
	defined_ = true;
}


Value::
Value( const std::string &value )
	:value_( value ), defined_(true)	
{
	
}


Value& 
Value::
operator=( const Value &rhs)
{
	value_ = rhs.value_;
	defined_ = rhs.defined_;
	
	return *this;
}

Value& 
Value::
operator=( const std::string &s)
{
	value_ = s;
	defined_ = true;
	return *this;
}

Value& 
Value::
operator=( float f )
{
	Value val( f );
	*this = val;
	return *this;
}

Value& 
Value::
operator=( double d )
{
	Value val( d );
	*this = val;
	return *this;
}


Value& 
Value::
operator=( int f )
{
	Value val( f );
	*this = val;
	return *this;
}

Value&
Value::
operator=( const boost::posix_time::ptime &pt )
{
	Value val( pt );
	*this = val;
	return *this;
}

std::string 
Value::
asString( )const
{
	if( ! defined_ )
		throw logic_error("Value is undefined.");
	
	return value_;
}


std::string
Value::
asString( const std::string &defValue )const
{
	if( ! defined_ )
		return defValue;
		
	return value_;
	
}

float
Value::
asFloat( )const
{
	if( ! defined_ )
		throw logic_error("Value is undefined.");
			
	try {
		return boost::lexical_cast<float>(value_);
	}
	catch( ... ) {
		throw logic_error("Value '"+value_+"' not convertibel to float!");
	}
}


float 
Value::
asFloat( float defValue )const
{
	if( ! defined_ )
		return defValue;
		
	try {
		if( value_.empty() )
			return defValue;
		
		return boost::lexical_cast<float>(value_);
	}
	catch( ... ) {
		throw logic_error("Value '"+value_+"' not convertibel to float!");
   }
}

double
Value::
asDouble( )const
{
	if( ! defined_ )
		throw logic_error("Value is undefined.");
			
	try {
		return boost::lexical_cast<double>(value_);
	}
	catch( ... ) {
		throw logic_error("Value '"+value_+"' not convertibel to double!");
	}
}


double 
Value::
asDouble( double defValue )const
{
	if( ! defined_ )
		return defValue;
		
	try {
		if( value_.empty() )
			return defValue;
		
		return boost::lexical_cast<double>(value_);
	}
	catch( ... ) {
		throw logic_error("Value '"+value_+"' not convertibel to double!");
   }
}

int
Value::
asInt( )const
{
	if( ! defined_ )
		throw logic_error("Value is undefined.");
		
	try {
		return boost::lexical_cast<int>(value_);
	}
	catch( ... ) {
		throw logic_error("Value '"+value_+"' not convertibel to int!");
   }
}

int
Value::
asInt( int defValue )const
{
	if( ! defined_ )
		return defValue;
	
	try {
		if( value_.empty() )
			return defValue;
	
		return boost::lexical_cast<int>(value_);
	}
	catch( ... ) {
		throw logic_error("Value '"+value_+"' not convertibel to int!");
	}
}

boost::posix_time::ptime 
Value::
asPTime( )const
{
	if( ! defined_ )
		throw logic_error("Value is undefined.");
	
	if( value_.empty() )
		throw logic_error("Value has no value.");
	
	try {
		return miutil::ptimeFromIsoString( value_ );
	}
	catch( logic_error &ex) {
		throw logic_error("Value '"+value_+"' not a valid time! ("+ex.what()+")");
	}
	catch( ... ) {
		throw logic_error("Value '"+value_+"' not a valid time!");
	}
}


boost::posix_time::ptime 
Value::
asPTime( const boost::posix_time::ptime &defValue )const
{
	try {
		return asPTime( );
	}
	catch( ... ) {
		return defValue;
	}
}

std::ostream& 
operator<<(std::ostream& output, const Value& v)
{
	if( ! v.defined_ )
		output << "'undefined'";
	else
		output << v.value_;
	
	return output;
}

} 


