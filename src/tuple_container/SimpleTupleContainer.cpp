/*
 * IDataWrapper.h
 *
 *  Created on: Sep 11, 2013
 *      Author: borgem
 */

#include "SimpleTupleContainer.h"
#include <string>
#include <map>
#include <vector>
#include <list>
#include <stdexcept>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/trim.hpp>

using namespace std;

namespace miutil {
namespace container {


SimpleTuple::
SimpleTuple( const SimpleTuple & t)
	: container( t.container ), values( t.values ), iter( t.iter )
{
	nTuples++;
}

SimpleTuple::
~SimpleTuple()
{
	nTuples--;
	//cerr << "DETOR: SimpleTuple\n";
}


SimpleTuple::
SimpleTuple( const std::vector<Field> &values_,
		     const SimpleTupleContainer *container,
		     std::list<SimpleTuple>::const_iterator it )
	: container( container ), values( values_ ), iter( it )
{
	nTuples++;
}


int
SimpleTuple::
size()const
{
	return container->fieldNames.size();
}

Field
SimpleTuple::
at( int index )const
{
	if( index < 0  || index >= values.size() )
		throw std::range_error("Index out of range");

	return values[index];
}

int
SimpleTuple::
getIndex( const std::string &fieldName )const
{
	std::map<std::string, int>::const_iterator it = container->fieldNames.find( fieldName );
	if( it == container->fieldNames.end() ) {
		throw std::range_error("Unknown field name");
	}
	return it->second;
}

SimpleIterator::
SimpleIterator( std::list<SimpleTuple> &container_ )
	: init( false ), container( container_ ),
	  it( container.end() )
{
}

bool
SimpleIterator::
hasNext()const
{
	if( ! init ) {
		it = container.begin();
		init = true;
	} else if( it != container.end() ){
		++it;
	}

	return it != container.end();
}

ITuple&
SimpleIterator::
next()const
{
	if( ! init )
		throw std::range_error("Iterator NOT initialized.");
	else if(  it == container.end() )
		throw std::range_error("Iterator past the end.");
	else
		return *it;
}


std::string
SimpleTupleContainer::
trim( const std::string &s, const char *any_of )
{
	return boost::algorithm::trim_copy_if( s, boost::algorithm::is_any_of( any_of) );
}

SimpleTupleContainer::
SimpleTupleContainer( const std::list<std::string> &names )
{
	std::list<std::string>::const_iterator it=names.begin();
	for( int i=0; it != names.end(); ++it, ++i )
		fieldNames[ trim( *it ) ]=i;
}

SimpleTupleContainer::
SimpleTupleContainer( const std::vector<std::string> &names )
{
	for( std::vector<std::string>::size_type i=0; i < names.size(); ++i )
		fieldNames[ trim( names[i] ) ] = i;
}

int
SimpleTupleContainer::
numberOfColumns() const
{
	return fieldNames.size();
}

std::vector<std::string>
SimpleTupleContainer::
columnNames()const
{
	vector<string> ret( fieldNames.size() );

	for( map<string, int>::const_iterator it=fieldNames.begin();
		 it != fieldNames.end(); ++it )
		ret[it->second] = it->first;

	return ret;
}

bool
SimpleTupleContainer::
add( const std::vector<std::string> &values )
{
	string buf;
	bool isNumber;
	if( values.size() != fieldNames.size() )
		return false;

	std::vector<Field> val;
	val.reserve( values.size() );
	std::vector<std::string>::const_iterator it=values.begin();

	for(  ;it != values.end(); ++it ) {
		buf = boost::trim_copy( *it );
		isNumber = true;

		if( ! buf.empty() && buf[0]=='"')
			isNumber = false;

		val.push_back( Field( buf, (buf.empty()?true:false), isNumber  ) );
	}

	tuples.push_back( SimpleTuple( val, this ) );
	return true;
}

IIterator*
SimpleTupleContainer::
iterator()const
{
	return new SimpleIterator( const_cast<std::list<SimpleTuple>&>(tuples) );
}

}
}
