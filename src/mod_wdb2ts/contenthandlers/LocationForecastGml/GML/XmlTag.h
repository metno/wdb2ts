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


#ifndef __WDB2TS_XML_TAG_H__
#define __WDB2TS_XML_TAG_H__

#include <contenthandlers/LocationForecastGml/GML/XmlBase.h>
#include <contenthandlers/LocationForecastGml/GMLContext.h>

namespace wdb2ts {



class XmlTag : public XmlBase 
{
	std::string indent;
	std::string tag;
	bool oneLine;
	GMLContext *gmlContext;
	std::string id;
	
public:
	
	XmlTag( GMLContext &context, const std::string &tag_, const std::string &id_="", bool oneLine_=false )
		: tag( tag_ ), oneLine( oneLine_ ), gmlContext( &context ), id( id_ ) {}
	
	virtual ~XmlTag(){
		if( out ) {
			if( ! oneLine )
				(*out) << indent;
			
			(*out) << "</" << tag << ">\n";
		}
	}
	
	virtual void output( std::ostream &out, miutil::Indent &indent_ ) {
		this->out = &out;
		this->indent = indent_.spaces();
		
		out << indent << "<" << tag;
		
		if( ! id.empty() ) {
			int iid=gmlContext->idMap[id];
			gmlContext->idMap[id]++;
			out << " gml:id=\"" << id << "-" << iid << "\"";
		}
			
		out << ">";
		
		if( !oneLine ) out << "\n";
	}
};

}


#endif 
