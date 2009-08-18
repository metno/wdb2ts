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


#ifndef __WDB2TS_WDB_FORECASTS_TAG_H__
#define __WDB2TS_WDB_FORECASTS_TAG_H__

#include <stdio.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <ptimeutil.h>
#include <contenthandlers/LocationForecastGml/GML/XmlBase.h>
#include <contenthandlers/LocationForecastGml/GMLContext.h>

namespace wdb2ts {



class WdbForecastsTag : public XmlBase 
{
	std::string indent;
	boost::posix_time::ptime created;
	std::string schema;
	GMLContext *gmlContext;
public:
	WdbForecastsTag() : gmlContext( 0 )
	{
	}
	
	WdbForecastsTag( GMLContext &context, const boost::posix_time::ptime &created, const std::string &schema )
		: gmlContext( &context )
	{
		init( created, schema );
	}

	
	virtual ~WdbForecastsTag(){
		if( out )
			(*out) << indent << "</mox:Forecasts>\n";
	}

	void init( const boost::posix_time::ptime &created, const std::string &schema ) {
		this->created = created;
		this->schema = schema ;
	}
	
	virtual void output( std::ostream &out, miutil::Indent &indent_ ) {
		using namespace miutil;
		char buf[16];
		this->out = &out;
		this->indent = indent_.spaces();
		
		boost::gregorian::date date=created.date();
		int y = date.year();
		int m = date.month();
		int d = date.day();
		
		sprintf( buf, "%4d%02d%02d", y, m, d );
		
		out << indent << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n" 
		    << indent << "<mox:Forecasts\n"
		    << indent << "   gml:id=\"pt0-" << buf << "\"\n"  
		    << indent << "   " << "xmlns:mox=\"http://wdb.met.no/wdbxml\"\n"
		    << indent << "   " << "xmlns:metno=\"http://api.met.no\"\n"
/* FIXME:
 * Reverted to the previous declaration of the namespace. The reason is to be compatible with
 * readers tha is allready using the mox format.
 * The implementation should be version dependent.
 *		    << indent << "   " << "xmlns:gml=\"http://www.opengis.net/gml/3.2\"\n"
*/
		    << indent << "   " << "xmlns:gml=\"http://www.opengis.net/gml\"\n"
		    << indent << "   " << "xmlns:xlink=\"http://www.w3.org/1999/xlink\"\n"
		    << indent << "   " << "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://wdb.met.no/wdbxml/schema/products.xsd\">\n";
	}
};

}


#endif 
