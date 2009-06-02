#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <iostream>
#include "mkdir.h"

namespace {
	bool mkdir_(const std::string &path);
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

  
namespace {
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
 
 