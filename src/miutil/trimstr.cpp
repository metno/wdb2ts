#include "trimstr.h"

void
miutil::
trimstr(std::string &str, ETrimStrWhere where, const char *trimset)
{
	std::string::size_type pos;
	std::string::size_type len;

	if( str.length()==0 )
		return;

	if( where==TRIMFRONT || where==TRIMBOTH ){  //Trim front
		pos = str.find_first_not_of(trimset);

		if( pos==std::string::npos )
			str.erase();
		else if( pos>0 )
	    	str.erase(0, pos);
	}

	len = str.length();

	if( len>0 && ( where==TRIMBACK || where==TRIMBOTH ) ) {  //Trim end
		pos = str.find_last_not_of(trimset);
	
		if(pos==std::string::npos)
			str.erase();
		else if( pos < ( len-1 ) )
			str.erase( pos+1, len-pos-1 );
   }
}

