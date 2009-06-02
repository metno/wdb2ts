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


#ifndef __WDB2TS_LOCATIONTAG_H__
#define __WDB2TS_LOCATIONTAG_H__

#include <IXmlTemplate.h>
#include <iostream>
#include <iomanip>

namespace wdb2ts {


class LocationTag : public IXmlTemplate 
{
	std::string indent;
	int alt;
	float lat, lon;
	std::ostream *out;
	
public:
	LocationTag() : out( 0 ) {}

	LocationTag( float latitude, float longitude, int altitude ) {
		init( latitude, longitude, altitude );
	}
	
	virtual ~LocationTag(){
		if( out )
			(*out) << indent << "</location>\n";
	}
	
	void init( float latitude, float longitude, int altitude ) {
		alt= altitude;
		lon = longitude ;
		lat = latitude;
	}
	
	virtual void output( std::ostream &out, const std::string &indent ) {
		this->out = &out;
		this->indent = indent;
		int oldPrec = out.precision();
		
	//	out.setf(std::ios::floatfield, std::ios::fixed);
		out.precision(4);
		
		out << indent << "<location altitude=\"" <<  alt << "\" " 
		    << "latitude=\"" <<  lat << "\" "
		    << "longitude=\"" << lon << "\">\n";
		
		out.precision( oldPrec );
	}
};

}


#endif 
