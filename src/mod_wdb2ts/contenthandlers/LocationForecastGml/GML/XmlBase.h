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

#ifndef __WDB2TS_XMLBASE_H__
#define __WDB2TS_XMLBASE_H__

#include <ostream>
#include <ctype.h>
#include <sstream>
#include <string>
#include <Indent.h>

namespace wdb2ts {

class XmlBase
{
protected:
	std::ostream *out;
	
public:
	XmlBase(): out( 0 ) {}
	virtual ~XmlBase(){}
	
	/**
	 * Create an id out of an tag with camel case.
	 * Ignore the namespace part.
	 * 
	 * ex. 
	 * 	mox:issueTime => it ( mox: ignored).
	 */
	static std::string sId( const std::string &tag_ )  
	{
		std::string tag;
		std::ostringstream ost;
		char ch;
		std::string::size_type i=0;
		
		i = tag_.find_first_of(":");
		
		if( i != std::string::npos )
			tag = tag_.substr( i+1 );
		else 
			tag = tag_;
		
		i = 0;
		
		if( tag.empty() )
			return "";
		
		do {
			ch = tag[i];
			ost << static_cast<char>( tolower( ch ) );
			i++;
			
			if( i >= tag.length() )
				break;
			
			i = tag.find_first_of( "ABCDEFGHIJKLMNOPQRSTUVWXYZ", i );
		} while( i != std::string::npos );
			
		return ost.str();
	}
	
	virtual void output( std::ostream &out, miutil::Indent &indent ){};
};

}

#endif 
