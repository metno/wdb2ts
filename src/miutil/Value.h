#ifndef __MIUTIL_VALUE_H__
#define __MIUTIL_VALUE_H__

#include <iostream>
#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>
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
	 * @throws std::logic_error if the this Value is undefined.
	 */
	std::string asString()const;
	
	/**
	 * Return the defValue if the param dos not exist.
	 */
	std::string asString( const std::string &defValue )const;
	
	/**
	 * @throws std::logic_error if undefined.
	 * @throws std::bad_cast if the param value can not be converted to 
	 *     an float.
	 */
   float asFloat( )const;
   
   /**
    * Return the defValue if the param dos not exist.
    * @throws std::bad_cast if the param value can not be converted to 
    *     an float.
    */
   float asFloat( float defValue )const;

	/**
	 * @throws std::logic_error if undefined.
	 * @throws std::bad_cast if the param value can not be converted to 
	 *     an float.
	 */
   double asDouble( )const;
   
   /**
    * Return the defValue if the param dos not exist.
    * @throws std::bad_cast if the param value can not be converted to 
    *     an float.
    */
   double asDouble( double defValue )const;
   
   /**
    * Return the defValue if the param dos not exist.
    * 
  	 * @throws std::logic_error if undefined.
  	 */
   int asInt()const;
   
   /**
    * Return the defValue if the param dos not exist.
    * @throws std::logic_error if the param value can not be converted to 
    */
   int asInt(int defValue )const;
  
   /**
  	 * @throws std::logic_error if undefined.
  	 */
   boost::posix_time::ptime asPTime( )const;
   
   /**
    * Return the defValue if the param dos not exist.
    * 
    * @throws std::logic_error if the param value can not be converted to 
    *     an ptime.
    */
   boost::posix_time::ptime asPTime( const boost::posix_time::ptime &defValue )const;
   
   friend std::ostream& operator<<(std::ostream& output, const Value& v);
};

std::ostream& operator<<(std::ostream& output, const Value& v);

}

#endif 
