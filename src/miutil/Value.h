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

#ifndef __MIUTIL_VALUE_H__
#define __MIUTIL_VALUE_H__

#include <iostream>
#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/lexical_cast.hpp>
#include <stdexcept>


namespace miutil {

/**
 * Value is a helper class to hold a string representation
 * of a value. It support int, float, double, boost::posix_time::ptime
 * and string. 
 */
class Value {
	std::string value_;
	bool        defined_;
	
public:
	Value();
	Value( const Value &v);
	Value( float f, int desimals=5 );
	Value( double d, int desimals=5 );
	Value( int i );
	Value( const std::string &value );
	Value( const boost::posix_time::ptime &pt );
	
	Value& operator=( const Value &v);
	Value& operator=( const std::string &s);
	Value& operator=( float f );
	Value& operator=( double d );
	Value& operator=( int f );
	Value& operator=( const boost::posix_time::ptime &pt );
	
	bool defined() const { return defined_; }

	/**
	 * Use as<type>(), where type is a numeric including float, double etc.
	 *
	 * @throw std::logic_error if the value is undefined or can't be  converted to
	 *    type T.
	 */
	template <typename T>
	T as()const {
		if( !defined_)
			throw std::logic_error("The value is undefined!");
		try {
			return boost::lexical_cast<T>(value_);
		}
		catch( const boost::bad_lexical_cast &ex ) {
			throw std::logic_error(ex.what());
		}
	}

	/**
	 * Use getAs<type>(defValue), where type is a numeric including float, double etc.
	 *
	 * Return defValue if the value is undefuned or can't be converted to type T.
	 */
	template <typename T>
	T as(const T &defaultValue)const{
		try {
			return as<T>();
		}
		catch( const std::exception &e /* ignored */) {
			return defaultValue;
		}
	}
	
	/**
	 * @throws std::logic_error if the this Value is undefined.
	 */
	std::string asString()const;


	/**
	 * Return the defValue if the value does not exist.
	 */
	std::string asString( const std::string &defValue )const;
	
   /**
  	 * @throws std::logic_error if undefined or can't be converted to ptime.
  	 */
   boost::posix_time::ptime asPTime( )const;
   
   /**
    * Return the defValue if the value does not exist or can't be converted to bool.
    */
   boost::posix_time::ptime asPTime( const boost::posix_time::ptime &defValue )const;
   
   /**
    * @throws std::logic_error if undefined or can't be converted to bool.
    */
   bool asBool()const;

   /**
    * Return the defValue if the value does not exist or can't be converted to bool.
    */
   bool asBool(bool defVal)const;
   friend std::ostream& operator<<(std::ostream& output, const Value& v);
};


/**
 * Specializations for std::string, boost::posix_time::ptime and bool.
 */
template <>
inline std::string Value::as<std::string>()const {
	return asString();
}

template <>
inline std::string Value::as<std::string>(const std::string &defVal)const {
	return asString(defVal);
}

template <>
inline boost::posix_time::ptime Value::as<boost::posix_time::ptime>()const {
	return asPTime();
}

template <>
inline boost::posix_time::ptime Value::as<boost::posix_time::ptime>(const boost::posix_time::ptime &defVal)const {
	return asPTime(defVal);
}

template <>
inline bool Value::as<bool>( const bool &defVal) const {
	return asBool(defVal);
}

template <>
inline bool Value::as<bool>()const {
	return asBool();
}


std::ostream& operator<<(std::ostream& output, const Value& v);

}

#endif 
