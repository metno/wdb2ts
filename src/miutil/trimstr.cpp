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

