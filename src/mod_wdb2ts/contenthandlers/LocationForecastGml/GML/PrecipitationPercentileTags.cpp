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


#include <float.h>
#include <math.h>
#include <PrecipitationPercentileTags.h>
#include <iostream>

namespace wdb2ts {

using namespace std;

	
void 
PrecipitationPercentileTags::
output( std::ostream &out, const std::string &indent ) 
{
	//out.setf(ios::floatfield, ios::fixed);
	//out.precision(1);
	int oldPrec = out.precision();
	float value;

	value = le.PRECIP_PERCENTILE_10( hoursBack ); 
	if( value != FLT_MAX )
		out << indent << "<probability type=\"exact\" parameter=\"precipitation\""
		    << " percentile=\"10\" unit=\"mm\" value=\"" << value << "\"/>\n";
		
	value = le.PRECIP_PERCENTILE_25( hoursBack );
	if( value != FLT_MAX )
		out << indent << "<probability type=\"exact\" parameter=\"precipitation\""
			 << " percentile=\"25\" unit=\"mm\" value=\"" << value << "\"/>\n";
		
	value = le.PRECIP_PERCENTILE_50( hoursBack ); 
	if( value != FLT_MAX )
		out << indent << "<probability type=\"exact\" parameter=\"precipitation\""
		    << " percentile=\"50\" unit=\"mm\" value=\"" << value << "\"/>\n";
		
	value = le.PRECIP_PERCENTILE_75( hoursBack ); 
	if( value != FLT_MAX )
		out << indent << "<probability type=\"exact\" parameter=\"precipitation\""
		    << " percentile=\"75\" unit=\"mm\" value=\"" << value << "\"/>\n";

	value = le.PRECIP_PERCENTILE_90( hoursBack ); 
	if( value != FLT_MAX )
		out << indent << "<probability type=\"exact\" parameter=\"precipitation\""
		    << " percentile=\"90\" unit=\"mm\" value=\"" << value << "\"/>\n";

	out.precision( 0 );
	
	value = le.PRECIP_PROBABILITY_0_1_MM( hoursBack ); 
	if( value != FLT_MAX )
		out << indent << "<probability type=\"lowerlimit\" parameter=\"precipitation\""
		    << " percentile=\"0.1mm\" unit=\"percent\" value=\"" << value << "\"/>\n";
	value = le.PRECIP_PROBABILITY_0_2_MM( hoursBack );
	
	if( value != FLT_MAX )
		out << indent << "<probability type=\"lowerlimit\" parameter=\"precipitation\""
		    << " percentile=\"0.2mm\" unit=\"percent\" value=\"" << value << "\"/>\n";
	
	value = le.PRECIP_PROBABILITY_0_5_MM( hoursBack ); 
	if( value != FLT_MAX )
		out << indent << "<probability type=\"lowerlimit\" parameter=\"precipitation\""
		    << " percentile=\"0.5mm\" unit=\"percent\" value=\"" << value << "\"/>\n";
	
	value = le.PRECIP_PROBABILITY_1_0_MM( hoursBack ); 
	if( value != FLT_MAX )
		out << indent << "<probability type=\"lowerlimit\" parameter=\"precipitation\""
		    << " percentile=\"1.0mm\" unit=\"percent\" value=\"" << value << "\"/>\n";
	
	value = le.PRECIP_PROBABILITY_2_0_MM( hoursBack ); 
	if( value != FLT_MAX )
		out << indent << "<probability type=\"lowerlimit\" parameter=\"precipitation\""
		    << " percentile=\"2.0mm\" unit=\"percent\" value=\"" << value << "\"/>\n";
	
	value = le.PRECIP_PROBABILITY_5_0_MM( hoursBack ); 
	if( value != FLT_MAX )
		out << indent << "<probability type=\"lowerlimit\" parameter=\"precipitation\""
			 << " percentile=\"5.0mm\" unit=\"percent\" value=\"" << value << "\"/>\n";
	
	out.precision( oldPrec );
}

}
