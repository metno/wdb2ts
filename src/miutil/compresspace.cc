#include "compresspace.h"

/**
 * cmprspace komprimerer alle space slik at de bare opptar
 * en space. Dersom det er space før \n fjernes disse. Dersom
 * det er space i starten fjernes disse. TAB og CR erstattes med SPACE.
 * Som space regnes SPACE, TAB og CR. Ved retur vil buf kun bestå av
 * ord separert med kun en SPACE 
 *
 * Eks.
 *    "dette er  en \t  string\tmed space     \n"
 *    blir komprimert til. "dette er en string med space\n"
 */

void 
miutil::
compresSpace(std::string &buf)
{
    const char *space=" \t\r";
    std::string::size_type n1, n2;

    if(buf.length()==0)
	return;
    
    n1=buf.find_first_of(space);

    while(n1!=std::string::npos)
    {
	n2=buf.find_first_not_of(space, n1);
	    
	if(n2!=std::string::npos)
	{
	    if(buf[n2]=='\n'|| (n1>0 && buf[n1-1]=='\n') || n1==0)
		buf.erase(n1, n2-n1);
	    else 
	    {
		if((n2-n1)>1)
		    buf.erase(n1+1, (n2-n1)-1);
		
		if(buf[n1]!=' ')
		    buf[n1]=' ';
	    }

	    n1=buf.find_first_of(space, n2);
	}else
	{
	    buf.erase(n1);
	    n1=std::string::npos;
	}
    }
}
	    
