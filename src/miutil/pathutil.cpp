/*
 * pathutil.cpp
 *
 *  Created on: Sep 14, 2017
 *      Author: borgem
 */

#include "trimstr.h"
#include "pathutil.h"
#include "compresspace.h"

namespace miutil {

std::string
dirname(const std::string &path_) {
	using namespace std;
	string path(path_);
	miutil::trimstr(path, miutil::TRIMFRONT);
	miutil::trimstr(path, miutil::TRIMBACK, " /\r\n\t");
	string::size_type i = path.find_last_of("/");

	if( i == string::npos ) {
		if( path[0] == '/')
			return "/";
		else
			return ".";
	} else {
		return path.substr(0, i);
	}
}

std::string
basename(const std::string &path_){
	using namespace std;
	string path(path_);
	miutil::trimstr(path, miutil::TRIMFRONT);
	miutil::trimstr(path, miutil::TRIMBACK, " /\r\n\t");
	string::size_type i = path.find_last_of("/");

	if(i==string::npos)
		return path;
	else
		return path.substr(i+1);
}

std::string
fixPath(const std::string &path_, bool pathSepAtEnd=true) {
	using namespace std;
	string path(miutil::trimstrCopy(path_));

	if( path.empty())
		return "";

	if( !pathSepAtEnd )
		miutil::trimstr(path, miutil::TRIMBACK, "/");
	else
		path += "/";

	miutil::compres(path, "/"); //Remove duplicates of '/'.
	return path;
}



}



