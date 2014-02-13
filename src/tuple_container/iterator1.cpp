//============================================================================
// Name        : iterator.cpp
// Author      : Børge Moe
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include "CSV.h"


using namespace std;
using namespace miutil::container;

namespace miutil {
namespace container {
	int nTuples=0;
}
}
std::ostream&
print( std::ostream &out, const ITupleContainer &container )
{
	cerr << " ----- print ------ \n";

	int i=0;

	for( auto_ptr<IIterator> it( container.iterator() ); it->hasNext(); ++i ) {
		ITuple& tuple = it->next();

		out << tuple.at( "validtimefrom" ).c_str() << " - "
			<< tuple.at( "validtimeto" ).c_str() << ": '"
			<< tuple.at( "valueparametername" ).c_str() << "' "
		    << tuple.at( "value" ).as<double>()  << " "
		    <<tuple.at( "valueparameterunit" ).c_str() << endl;

//		if( i>3 )
//			break;
	}

	return out;
}


int
main()
{
	string fileName("/home/borgem/projects/test-data.csv");
	ifstream ist( fileName.c_str() );

	if( ! ist.is_open() ) {
		cerr << "Could not open: " << fileName << endl;
		return 1;
	}

	SimpleTupleContainer *simpleContainer = readCSV( ist );

	if( ! simpleContainer ) {
		cerr << "Could not read the CSV file: " << fileName << endl;
		return false;
	}

	vector<string> fieldNames = simpleContainer->columnNames();

	cerr << "Fieldnames: " << endl;
	cerr << "---------- " << endl;

	for( unsigned int i=0; i < fieldNames.size(); ++i )
		cerr << fieldNames[i] << endl;
	cerr << endl;


	print( cerr, *simpleContainer );

	delete simpleContainer;

//	cerr << "nTuples: " << nTuples << endl;
}

