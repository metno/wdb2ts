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


#ifndef __WDB2TS_POINT_TAG_H__
#define __WDB2TS_POINT_TAG_H__

#include <contenthandlers/LocationForecastGml/GML/XmlBase.h>
#include <contenthandlers/LocationForecastGml/GMLContext.h>
#include <iostream>
#include <iomanip>
#include <Indent.h>

namespace wdb2ts {


class PointTag : public XmlBase 
{
	int alt;
	float lat, lon;
	std::ostream *out;
	GMLContext *gmlContext;
	std::string tag;
	
public:
	PointTag() : out( 0 ), gmlContext( 0 ) {}

	PointTag( GMLContext &context, const std::string &tag_,
			    float latitude, float longitude, int altitude ) 
		: gmlContext( &context ), tag( tag_ ) 
	{
		init( latitude, longitude, altitude );
	}
	
	virtual ~PointTag(){
	}
	
	void init( float latitude, float longitude, int altitude ) {
		alt= altitude;
		lon = longitude ;
		lat = latitude;
	}
	
	virtual void output( std::ostream &out, miutil::Indent &indent ) {
		this->out = &out;
		int oldPrec = out.precision();
		
		int id=gmlContext->idMap[tag];
		gmlContext->idMap[tag]++;
		
	//	out.setf(std::ios::floatfield, std::ios::fixed);
		out.precision(4);
		
		out << indent.spaces() << "<" << tag << ">\n";
		indent.incrementLevel();
		out << indent.spaces() << "<gml:Point gml:id=\"" << sId( tag ) << "-" << id << "\" srsName=\"urn:ogc:def:crs:epsg:4326\">\n";
		indent.incrementLevel();
		out << indent.spaces() << "<gml:pos>" << lon << " " << lat << " " << alt << "</gml:pos>\n";
		indent.decrementLevel();
		out << indent.spaces() << "</gml:Point>\n";
		indent.decrementLevel();
		out << indent.spaces() << "</" << tag << ">\n";
		out.precision( oldPrec );
	}
};

}


#endif 
