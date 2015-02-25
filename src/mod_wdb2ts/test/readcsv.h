
#ifndef SRC_MOD_WDB2TS_TEST_READCSV_H_
#define SRC_MOD_WDB2TS_TEST_READCSV_H_

#include <string>
#include <vector>
#include <list>

bool
readCSV( std::istream &in, std::list<std::vector<std::string> > &csv, char sep_ );


#endif
