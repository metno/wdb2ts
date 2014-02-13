/*
 * IDataWrapper.h
 *
 *  Created on: Sep 11, 2013
 *      Author: borgem
 */

#ifndef __miutil_container_ITupleContainer_h__
#define __miutil_container_ITupleContainer_h__

#include <stdio.h>
#include <string>
#include <map>
#include <vector>
#include <list>
#include <stdexcept>
#include <iostream>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <ptimeutil.h>

namespace miutil {
namespace container {

extern int nTuples;

class Field
{
	std::string val;
	bool isNumber;
	bool isNull;
public:

	Field( const std::string &val_, bool isNull_, bool isNumber=false ):
		val( val_ ), isNumber( isNumber ), isNull( isNull_ ) {
		if( val.size() >= 2 && val[0]=='\"' && val[val.length()-1]=='\"' ) {
			val = val.substr( 1, val.length()-2 );
			isNumber = true;
		}
	}

	template <typename T> T as( const T &defaultValue ) const
	{
		if( isNull )
			return defaultValue;

		//std::cerr << "as '" << val << "' (" << defaultValue << ")\n";
		return boost::lexical_cast<T>( val );
	}

	template <typename T> T as() const
	{
		//std::cerr << "as '" << val << "'\n";
		return boost::lexical_cast<T>( val );
	}


	bool is_null()const { return isNull; }
	bool is_number()const { return isNumber; }
	std::string c_str() const { return val; }
};

template <>
boost::posix_time::ptime
inline Field::as<boost::posix_time::ptime>() const
{
	try {
		//std::cerr << "\nField::as to ptime specialization. '" << val << "'\n" << std::endl;
		return miutil::ptimeFromIsoString( val );
	}
	catch( ... ) {
		throw boost::bad_lexical_cast();
	}
}

//template <>
//float
//inline Field::as<float>() const
//{
//	float f;
//	if( is_null() )
//		throw boost::bad_lexical_cast();
//
//	if( sscanf(val.c_str(), "%f", &f ) != 1 )
//		throw boost::bad_lexical_cast();
//
//	return f;
//}
//
//template <>
//int
//inline Field::as<int>() const
//{
//	int i;
//	if( is_null() )
//		throw boost::bad_lexical_cast();
//
//	if( sscanf(val.c_str(), "%d", &i ) != 1 )
//		throw boost::bad_lexical_cast();
//
//	return i;
//}


class ITuple {
public:
	virtual ~ITuple(){}

	virtual Field at( const std::string &fieldName ) const {
		return at( getIndex( fieldName) );
	}

	virtual int size() const = 0;
		virtual Field at( int index ) const = 0;
	virtual int getIndex( const std::string &fieldName ) const = 0;

};

class IIterator {
public:
	virtual ~IIterator(){};

	virtual bool hasNext()const=0;

	/**
	 *
	 * @throws std::range_error if the iterator
	 *  is NOT initialized, ie hasNext is not called. Or
	 *  when next is called after hasNext has returned false.
	 */
	virtual ITuple& next()const=0;
};


class ITupleContainer {
public:
	virtual ~ITupleContainer(){}

	virtual int numberOfColumns() const=0;
	virtual std::vector<std::string> columnNames()const=0;

	std::string trim( const std::string &s, const char *any_of="\"" ){
		return boost::algorithm::trim_copy_if( s, boost::algorithm::is_any_of( any_of) );
	}

	virtual IIterator* iterator()const =0;

};
}
}

#endif
