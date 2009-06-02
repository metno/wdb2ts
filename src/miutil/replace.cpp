#include "replace.h"

std::string& 
miutil::
replace(std::string &str, const std::string &what, const std::string &with)
{
	std::string::size_type pos=str.find(what);
	
	while(pos!=std::string::npos){
		str.replace(pos, what.length(), with);
		pos=str.find(what, pos+with.length());
	}
	
	return str;
}
