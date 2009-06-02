#include <iostream>
#include <sstream>

#include "trimstr.h"
#include "splitstr.h"

using namespace std;

namespace {
	int   
	count(const std::string &str, char sep);
}


std::vector<std::string> 
miutil::
splitstr(const std::string &str, char sep)
{
 	ostringstream ost;
  	char          ch=0;
  	bool          inString=false;
 	int i=0;
  	int n=count(str, sep);
  	vector<string> data;
  	string elem;
  	
  	data.resize(n+1);
  	std::string::const_iterator it=str.begin();

  	for(;it!=str.end(); it++){
    	if(*it=='"' && ch!='\\'){
    		ost << *it;
      	ch=*it;
      	inString=!inString;
      	continue;
    	}
    
    	ch=*it;
    	
    	if(inString){
    		ost<<*it;
      	continue;
    	}
 
    	if(*it!=sep){
      	ost << *it;
    	}else{
      	elem=ost.str();
      	trimstr(elem);
      	data[i]=elem;
      	i++;
      	ost.str("");
    	}
  	}

	elem=ost.str();
	trimstr(elem);
  	data[i]=elem;
  	
  	return data;
}

namespace {
	int   
	count(const std::string &str, char sep)
	{
  		string::const_iterator it;
  		char                   ch=0;
  		int                    cnt=0;
 		bool                   inString=false;

  		for(it=str.begin();it!=str.end(); it++){
    		if(*it=='"' && ch!='\\')
      		inString=!inString;
    		else if(*it==sep && !inString)
      		cnt++;

    		ch=*it;
  		}
  
  		return cnt;
	}
}
