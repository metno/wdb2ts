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
compresSpace(std::string &buf, bool newlineAsSpace)
{
    const char *space=" \t\r";
    std::string::size_type n1, n2;

    if(buf.length()==0)
	return;

	if( newlineAsSpace )
		space = " \t\r\n";

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

	    n1=buf.find_first_of(space, n1+1);
	}else
	{
	    buf.erase(n1);
	    n1=std::string::npos;
	}
    }
}
	    
