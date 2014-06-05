/*
 * IDataWrapper.h
 *
 *  Created on: Sep 11, 2013
 *      Author: borgem
 */

#ifndef __miutil_container_PqTupleContainer_h__
#define __miutil_container_PqTupleContainer_h__


#include <string>
#include <map>
#include <set>
#include <vector>
#include <list>
#include <stdexcept>
#include <pqxx/pqxx>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/trim.hpp>
#include "ITupleContainer.h"

namespace miutil {
namespace container {


class PqContainer;

class PqTuple :
		public virtual ITuple
{
protected:
	friend class PqIterator;
	friend class PqContainer;
	PqTuple();
	PqTuple& operator=( const PqTuple &rhs );

	const PqContainer &container;
	const pqxx::result &result;
	mutable pqxx::result::const_iterator it;


	PqTuple( const PqTuple & );
	PqTuple( const PqContainer &container, const pqxx::result &result );

public:

	~PqTuple();

	virtual Field at( const std::string &fieldName ) const ;

	virtual int size() const ;
	virtual Field at( int index ) const;
	virtual int getIndex( const std::string &fieldName ) const;
};

class PqIterator : public virtual IIterator
{
protected:
	friend class PqContainer;
	mutable PqTuple tuple;
	mutable bool    init;
//	pqxx::result &result;
//	pqxx::result::iterator it;
	PqIterator( const PqContainer &container, const pqxx::result &result );

public:
	virtual ~PqIterator();

	bool hasNext()const;
	ITuple& next()const;
};


class PqContainer :
		public virtual ITupleContainer
{
protected:
	struct NumericTypes : public std::set<std::string>
	{
		NumericTypes();
	};

	friend class PqTuple;
	const pqxx::result &result;
	pqxx::transaction_base *transaction;
	//static  NumericTypes numericTypes;
	static  std::set<std::string> numericTypes;
	bool isInitialized;
	std::map<int,std::string> cNames;
	std::vector<bool> cIsNumber;

	void initColumnNames();
	void initColumnIsNumber();


public:

	PqContainer( const pqxx::result &result, pqxx::transaction_base *transaction=0 );

	virtual int numberOfColumns() const;
	virtual std::vector<std::string> columnNames()const;

	std::vector<std::string> getFieldNames()const;
	virtual IIterator* iterator()const;
};

}
}

#endif
