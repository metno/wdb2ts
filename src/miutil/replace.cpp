#include "replace.h"

std::string& 
miutil::
replace(std::string &str, const std::string &what, const std::string &with, int nToReplace )
{
	int nCount = nToReplace;
	std::string::size_type pos=str.find(what);
	
	while( pos!=std::string::npos ){
		if( nToReplace >0 && nCount == 0 )
			break;

		--nCount;
		str.replace(pos, what.length(), with);
		pos=str.find(what, pos+with.length());
	}
	
	return str;
}
