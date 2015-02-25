
#ifndef SRC_MOD_WDB2TS_TEST_READCSV_H_
#define SRC_MOD_WDB2TS_TEST_READCSV_H_

#include <string>
#include <vector>
#include <list>

std::list<std::vector<std::string> >
readCSV( std::istream &in, char sep_ );


#endif
