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


#ifndef __WDB2TS_WDB_VALID_TIME_PERIOD_H__
#define __WDB2TS_WDB_VALID_TIME_PERIOD_H__

#include <boost/date_time/posix_time/posix_time.hpp>
#include <ptimeutil.h>
#include <contenthandlers/LocationForecastGml/GML/XmlBase.h>
#include <contenthandlers/LocationForecastGml/GMLContext.h>

namespace wdb2ts {



class TimePeriodTag : public XmlBase 
{
	boost::posix_time::ptime from;
	boost::posix_time::ptime to;
	GMLContext *gmlContext;
	std::string tag;
	
public:
	TimePeriodTag( GMLContext &context, const std::string &tag_,
			                 const boost::posix_time::ptime &from, 
			                 const boost::posix_time::ptime &to  ) 
		: gmlContext( &context ), tag( tag_ )
	{
		init( from, to );
	}
	
	virtual ~TimePeriodTag(){
	}
	
	void init( const boost::posix_time::ptime &from, const boost::posix_time::ptime &to ) {
		this->from = from;
		this->to = to ;
	}
	
	virtual void output( std::ostream &out, miutil::Indent &indent ) {
		using namespace miutil;
		this->out = &out;
		int id=gmlContext->idMap[tag];
		
		gmlContext->idMap[tag]++;
		
		out << indent.spaces() << "<" << tag << ">\n";
		indent.incrementLevel();
		out << indent.spaces() << "<gml:TimePeriod gml:id=\"" << sId( tag ) << "-" << id << "\">\n";
		indent.incrementLevel();
		out << indent.spaces() << "<gml:begin>" << isotimeString(from, true, true) << "</gml:begin>\n";
		out << indent.spaces() << "<gml:end>" << isotimeString(to, true, true) << "</gml:end>\n";
		indent.decrementLevel();
		out << indent.spaces() << "</gml:TimePeriod>\n";
		indent.decrementLevel();
		out << indent.spaces() << "</" << tag << ">\n";
	}
};

}


#endif 
