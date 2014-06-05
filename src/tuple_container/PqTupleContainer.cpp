/*
 * IDataWrapper.h
 *
 *  Created on: Sep 11, 2013
 *      Author: borgem
 */

#include "PqTupleContainer.h"
#include <string>
#include <map>
#include <vector>
#include <list>
#include <stdexcept>
#include <iostream>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/assign.hpp>

using namespace std;
//class PqContainer;


namespace miutil {
namespace container {

//PqContainer::NumericTypes PqContainer::numericTypes ;
std::set<std::string> PqContainer::numericTypes =
		boost::assign::list_of("smallint")("integer")("bigint" )("decimal")("numeric")("real")
		("double precision")("serial")("bigserial")("money" )("boolean");


PqContainer::
NumericTypes::
NumericTypes()
{

	//*this = boost::assign::list_of()
}

int nTuples;

PqTuple::
PqTuple( const PqTuple &tuple )
	:container( tuple.container), result( tuple.result ), it( tuple.it )
{
	//cerr << "PgTuple:: CTOR copy\n";
	//++nTuples;
}

PqTuple::
PqTuple( const PqContainer &container, const pqxx::result &result_)
	: container( container ), result( result_ )
{
	//cerr << "PgTuple:: CTOR ( const pqxx::result &result_ )\n";
	//++nTuples;
}


PqTuple::
~PqTuple()
{
	//cerr << "PgTuple:: DTOR\n";
	//++nTuples;
}

int
PqTuple::
size()const
{
	return it.size();
}

Field
PqTuple::
at( const std::string &fieldName )const
{
	//return Field( it.at( fieldName ).c_str(),  it.at( fieldName ).is_null() );
	return at( getIndex( fieldName ) );
}

Field
PqTuple::
at( int index )const
{
	if( index < 0 || index >= result.columns() ) {
		ostringstream ost;
		ost << "Index '" << index << "' out of range [0," <<  result.columns()  << ">.";
		throw range_error( ost.str() );
	}

	return Field( it.at( index ).c_str(), it.at( index ).is_null(), container.cIsNumber[index] );
}

int
PqTuple::
getIndex( const std::string &fieldName )const
{

	try {
		return result.column_number( fieldName );
	}
	catch( const exception &ex ){
		throw range_error( "Unknown field name: '" + fieldName + "'. (" +ex.what()+")" );
	}
	throw range_error( "Unknown field name: '" + fieldName + "'." );
}


PqIterator::
PqIterator( const PqContainer &container, const pqxx::result &result )
: tuple( container, result ), init( false )
{
}

PqIterator::
~PqIterator()
{
}

bool
PqIterator::
hasNext()const
{
	if( ! init ) {
		init = true;
		tuple.it = tuple.result.begin();
	} else if( tuple.it != tuple.result.end() ){
		++tuple.it;
	}

	return tuple.it != tuple.result.end();
}

ITuple&
PqIterator::
next()const
{
	if( ! init )
		throw std::range_error("Iterator NOT initialized.");
	else if( tuple.it == tuple.result.end() )
		throw std::range_error("Iterator, past the end of the result set.");
	else
		return tuple;
}



PqContainer::
PqContainer( const pqxx::result &result, pqxx::transaction_base *transaction )
	: result( result ), transaction( transaction  ), isInitialized( false )
{
	try {
		initColumnNames();
		initColumnIsNumber();
		isInitialized = true;
	}
	catch( const std::exception &e ) {
		cerr << "EXCEPTION: " << e.what() << endl;
		isInitialized = false;
	}
}

void
PqContainer::
initColumnNames()
{
	int n = result.columns();

	for( int i=0; i<n; ++i )
		cNames[i]= result.column_name( i ) ;
}

void
PqContainer::
initColumnIsNumber()
{
	int n = result.columns();
	cIsNumber.clear();
	cIsNumber.reserve( n );

	if( n == 0 )
		return;

	if( ! transaction ) {
		for( int i=0; i < n; ++i )
			cIsNumber.push_back( false );
		return;
	}

	ostringstream ost;
	int i=0;

	ost << "SELECT " << result.column_type( i ) << "::regtype";

	for( ++i; i < n; ++i )
		ost << ", " << result.column_type( i ) << "::regtype";

	pqxx::result r = transaction->exec( ost.str() );
	pqxx::result::const_iterator it = r.begin();

 	if( it == r.end() )
 		throw std::logic_error( string("Failed to get SQL types: ")+ ost.str() +"." );

	for( i = 0; i<n; ++i ) {
//		cerr << "  " <<  result.column_type( i ) << " - " << it->at(i).c_str() << " numeric: "
//				<< ( numericTypes.count(it.at(i).c_str() ) != 0?"true":"false") << "." << endl;
		cIsNumber.push_back( numericTypes.count( it.at(i).c_str() ) != 0 ) ;
	}
}

int
PqContainer::
numberOfColumns() const
{
	return result.columns();
}

std::vector<std::string>
PqContainer::
columnNames()const
{
	std::vector<std::string> res;
	int n = result.columns();

	res.reserve( n );

	for( int i=0; i<n; ++i )
		res.push_back( result.column_name( i ) );

	return res;
}


IIterator*
PqContainer::
iterator()const
{
	return new PqIterator( *this, result );
}

}
}

