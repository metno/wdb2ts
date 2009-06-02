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


#ifndef __WDB2TS_TIMETAG_H__
#define __WDB2TS_TIMETAG_H__

#include <boost/date_time/posix_time/posix_time.hpp>
#include <ptimeutil.h>
#include <IXmlTemplate.h>

namespace wdb2ts {



class TimeTag : public IXmlTemplate 
{
	std::string indent;
	boost::posix_time::ptime from;
	boost::posix_time::ptime to;
	
public:
	TimeTag(  const boost::posix_time::ptime &from, const boost::posix_time::ptime &to  ) {
		init( from, to );
	}
	
	virtual ~TimeTag(){
		if( out )
			(*out) << indent << "</time>\n";
	}
	
	void init( const boost::posix_time::ptime &from, const boost::posix_time::ptime &to ) {
		this->from = from;
		this->to = to ;
	}
	
	virtual void output( std::ostream &out, const std::string &indent ) {
		using namespace miutil;
		this->out = &out;
		this->indent = indent;
		
		out << indent << "<time datatype=\"forecast\" " 
		    << "from=\"" << isotimeString(from, true, true) << "\" "
		    << "to=\"" << isotimeString(to, true, true) << "\">\n";
	}
};

}


#endif 
