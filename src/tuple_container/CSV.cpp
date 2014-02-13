/*
 * CSValues.h
 *
 *  Created on: Sep 12, 2013
 *      Author: borgem
 */

#include <memory>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include "CSV.h"


using namespace std;
using namespace boost;
using namespace boost::algorithm;
using namespace miutil::container;

namespace {
string makeFnut( const Field &f ) {
	if( f.is_null() )
		return "";

	if( f.is_number() )
		return f.c_str();
	else
		return "\""+f.c_str()+"\"";
}
}

SimpleTupleContainer*
readCSV( std::istream &in, char sep_ )
{
	string line;
	vector<string> vals;
	SimpleTupleContainer *tuples=0;
	string sep;
	sep = sep_;


	while( getline( in, line ) ) {
		split( vals, line, is_any_of( sep ) );

		if( !tuples ) {
			try {
				tuples = new SimpleTupleContainer( vals );
			}
			catch( const bad_alloc  &ex) {
				return 0;
			}
		} else {
			tuples->add( vals );
		}
	}

	return tuples;

}

bool
writeCSV( std::ostream &out, const ITupleContainer &container, char sep  )
{
	int n;
	vector<string>::const_iterator itNames;
	vector<string> names = container.columnNames();
	n = names.size();

	if( names.empty() )
		return false;

	//Write the header to the stream.
	itNames = names.begin();
	out << *itNames;
	for( ++itNames; itNames != names.end(); ++itNames )
		out << sep << *itNames;
	out << endl;


	for( boost::shared_ptr<IIterator> it( container.iterator() ); it->hasNext(); /*empty*/) {
		ITuple &tuple = it->next();
		int i=0;
		out << makeFnut( tuple.at( i ) );
		for( ++i; i < n; ++i )
			out << sep << makeFnut( tuple.at( i ) );
		out << endl;
	}

	return true;
}

