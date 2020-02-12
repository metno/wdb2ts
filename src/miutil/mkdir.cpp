/*
    wdb - weather and water data storage

    Copyright (C) 2007 met.no

    Contact information:
    Norwegian Meteorological Institute
    Box 43 Blindern
    0313 OSLO
    NORWAY
    E-mail: wdb@met.no

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
    MA  02110-1301, USA
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <iostream>
#include <memory>
#include <list>
#include "geterrstr.h"
#include "trimstr.h"
#include "splitstr.h"
#include "mkdir.h"

namespace {
	bool mkdir_(const std::string &path);
	std::list<std::string> splitPath(const std::string &path_);
	std::list<std::string> checkPaths(const std::list<std::string> &paths);
}

using namespace std;



bool
dnmi::file::
mkdir(const std::string &newdir, const std::string &path_ )
{
	struct stat sbuf;
	string path(path_);
	string dirlist(newdir);
	string dir;
	 	
	 	
	while(!dirlist.empty() && dirlist[0]=='/')
		dirlist.erase(0, 1);
	 	
	if(dirlist.empty())
		return false;
	 	
	while(!path.empty() && path[path.length()-1]=='/')
		path.erase(path.length()-1);
	
	if(path.empty())
		path=".";
	
	//We first check if the directory allready exist.	
	if(stat(string(path+"/"+dirlist).c_str(), &sbuf)==0){
		// cerr << "mkdir: Path (E): " << string(path+"/"+dirlist) ;
		
		if(S_ISDIR(sbuf.st_mode))
			return true;
		else
			return false;
	}
		
	 	
	if(stat(path.c_str(), &sbuf) !=0)
		return false;
		
	if(!S_ISDIR(sbuf.st_mode))
		return false;
	
	string::size_type i=dirlist.find("/");		
	
	for( ; i!=string::npos; i=dirlist.find("/")){
		dir=dirlist.substr(0, i);
		dirlist.erase(0, i+1);
		
		if(dir.empty()) 
			continue;
			
		path+="/"+dir;
		
		if(!mkdir_(path))
			return false;
	}
    	
	if(!dirlist.empty()){
		path+="/"+dirlist;
		
		if(!mkdir_(path))
			return false;
	}
			
	return true;
}	


void dnmi::file::checkAndMkDirs(const std::string &path)
{
	using namespace std;

	list<string> paths=splitPath(path);

	if( paths.empty())
		return;

	//Return a list of paths that must be created.
	paths = checkPaths( paths );

	for( auto &p: paths) {
		if( ! mkdir_(p) ) {
			throw  logic_error(string("Cant create or check the path '") +p + ". Reason: " + miutil::geterrstr(errno));
		}
	}
}


namespace {

	//Returns and emty string if it fail. The caller
	//can check errno for the reason, see man 2 getcwd.
	std::string getCWD(){
		using namespace std;

		int n=128;
		do {
			unique_ptr<char> buf(new char[n]);
			char *d=getcwd(buf.get(), n);

			if( !d ) {
				if( errno == ERANGE) {
					n *= 2; //Double the buffer size, and try again;
				} else {
					return "";
				}
			} else {
				return d;
			}
		} while(true);

		return "";
	}


	/**
	 *
	 */
	std::pair<bool, bool> pathExistAndIsDir( const std::string &path) {
		using namespace std;
		struct stat sbuf;

		if(stat(path.c_str(), &sbuf) !=0) {
			if( errno == ENOENT || errno == ENOTDIR) {
				return pair<bool,bool>(false, false);
			} else {
				throw std::logic_error(string("Error: path: '")+path+"': " + miutil::geterrstr(errno));
			}
		} else if(S_ISDIR(sbuf.st_mode)) {
			return pair<bool,bool>(true, false);
		} else {
			return pair<bool,bool>(false, true);
		}
	}

	std::list<std::string> checkPaths(const std::list<std::string> &paths) {
		using namespace std;
		list<string>::const_iterator it=paths.begin();

		for(; it!=paths.end(); ++it) {
			pair<bool,bool> exist = pathExistAndIsDir(*it);

			if(exist.first)
				continue;
			else if( exist.second ) //The path exists, but is not a directory.
				throw logic_error(string("Path '")+*it + "' exist, but is not a directory.");
			else //The path do not exist.
				break;
		}
		return list<string>(it, paths.end());
	}


	std::list<std::string> splitPath(const std::string &path_)
	{
		using namespace miutil;
		using namespace std;

		std::list<std::string> ret;
		string path(path_);

		trimstr(path);

		if(path.empty())
			return std::list<std::string>();

		vector<string> pe=splitstr(path,'/','"');

		if(path[0]!='/') {
			path = getCWD();
			if( path.empty()) {
				cerr << "getcwd failed: " << miutil::geterrstr(errno) << "\n";
				return std::list<std::string>();
			}
		} else {
			path="";
		}

		for( string &e : pe ) {
			if( e.empty())
				continue;
			path += "/"+e;
			ret.push_back(path);
		}

		return ret;
	}


	bool 
	mkdir_(const std::string& path){
		struct stat sbuf;
			
		//cerr << "mkdir: " << path << endl;
			
		if(mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)!=0){
			if(errno==EEXIST){
				if(stat(path.c_str(), &sbuf) !=0)
					return false;
		
				if(!S_ISDIR(sbuf.st_mode))
					return false;
			}else{
				return false;
			}
		}
			
		return true;
	}
 }
