/*
 * IDataWrapper.h
 *
 *  Created on: Sep 11, 2013
 *      Author: borgem
 */

#ifndef __miutil_container_SimpleTupleContainer_h__
#define __miutil_container_SimpleTupleContainer_h__

#include "ITupleContainer.h"
#include <string>
#include <map>
#include <vector>
#include <list>
#include <stdexcept>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/trim.hpp>

namespace miutil {
namespace container {


class SimpleTupleContainer;

class SimpleTuple :
		public virtual ITuple
{
protected:
	friend class SimpleTupleContainer;
	SimpleTuple();
	SimpleTuple& operator=( const SimpleTuple& );
	const SimpleTupleContainer *container;
	std::vector<Field> values;
	mutable std::list<SimpleTuple>::const_iterator iter;



public:
	//SimpleTuple( const SimpleTuple *t);
	SimpleTuple( const SimpleTuple &tuple );
	SimpleTuple( const std::vector<Field> &values_,
				 const SimpleTupleContainer *container,
				 std::list<SimpleTuple>::const_iterator it=std::list<SimpleTuple>::const_iterator()
				 );
	virtual ~SimpleTuple();
	virtual int size()const;
	virtual Field at( int index )const;
	virtual int getIndex( const std::string &fieldName )const;
};

class SimpleIterator: public virtual IIterator
{
	friend class SimpleTupleContainer;
	mutable bool init;
	std::list<SimpleTuple> &container;
	mutable std::list<SimpleTuple>::iterator it;
	SimpleIterator( std::list<SimpleTuple> &container );
public:

	virtual bool hasNext()const;

	/**
	 *
	 * @throws std::range_error if the iterator
	 *  is NOT initialized, ie hasNext is not called. Or
	 *  when next is called after hasNext has returned false.
	 */
	virtual ITuple& next()const;

};

class SimpleTupleContainer :
		public ITupleContainer
{
protected:
	friend class SimpleTuple;
	std::list<SimpleTuple>  tuples;
	std::map<std::string, int> fieldNames;
	std::string trim( const std::string &s, const char *any_of="\"" );

public:
	SimpleTupleContainer( const std::list<std::string> &names );
	SimpleTupleContainer( const std::vector<std::string> &names );

	bool add( const std::vector<std::string> &values );

	virtual int numberOfColumns() const;
	virtual std::vector<std::string> columnNames()const;

	virtual IIterator* iterator()const;
};

}
}

#endif
