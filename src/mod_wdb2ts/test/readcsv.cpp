#include <iostream>
#include <memory>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <sstream>
#include "readcsv.h"

using namespace std;
//using namespace boost;
//using namespace boost::algorithm;

namespace {
void
split( vector<string> &vals, const string &line, char sep_)
{
	string::const_iterator it = line.begin();
	ostringstream buf;
	char ch;
	bool inString=false;
	vals.clear();

	while( it != line.end() ) {
		ch = *it;
		if( ch == '\\' ) {
			if( ++it == line.end() )
				continue;
			buf << ch << *it;
		} else if( inString && ch != '"' ) {
			buf << ch;
		} else if( ch == '"') {
			inString = ! inString;
		} else if( ch == sep_ ) {
			vals.push_back( boost::trim_copy( buf.str() ) );
			buf.str("");
		} else {
			buf << ch;
		}
		++it;
	}
	vals.push_back( boost::trim_copy( buf.str() ) );
}
}

bool
readCSV( std::istream &in, std::list<std::vector<std::string> > &csv, char sep_ )
{
	string line;
	vector<string> vals;
	int n=-1;

	csv.clear();

	while( getline( in, line ) ) {
		split( vals, line, sep_ );

		if( n < 0 )
			n=vals.size();
		else if( n > -1 && vals.size() != n ) {
			if( vals.size() && vals[0].length()==0) //Skip empty lines.
				continue;
			return false;
		}

		csv.push_back( vals );
	}

	return true;
}

