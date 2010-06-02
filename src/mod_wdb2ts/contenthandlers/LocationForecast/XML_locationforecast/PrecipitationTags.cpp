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
#include <PrecipitationTags.h>
#include <iostream>

namespace wdb2ts {

using namespace std;

void 
PrecipitationTags::
init( float value ) 
{
	this->value = value;
}
		
	
void 
PrecipitationTags::
output( std::ostream &out, const std::string &indent ) 
{
	//out.setf(ios::floatfield, ios::fixed);
	//out.precision(1);
	std::string description;
	std::string minMaxProb;



	if( value == FLT_MAX )
		return;

	if( min != FLT_MAX && max != FLT_MAX ) {
	   std::ostringstream o;
	   o.precision( out.precision() );
	   o.flags( out.flags() );
	   o << " minvalue=\"" << min << "\" maxvalue=\"" << max << "\"";

	   if( prob >= 0 )
	      o << " probability=\"" << prob << "\"";

	   minMaxProb = o.str();
	}

	out << indent 
	    << "<precipitation unit=\"mm\" value=\"" << value << "\"" << minMaxProb << "/>\n";
}

}
