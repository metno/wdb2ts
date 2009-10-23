/*
    wdb - weather and water data storage

    Copyright (C) 2008 met.no

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


#include <sstream>
#include <ptimeutil.h>
#include <Logger4cpp.h>
#include <wciHelper.h>


using namespace std;

namespace wdb2ts {

string
wciReadReturnColoumns( int wciProtocol )
{
	if( wciProtocol < 3 )
		return "value, dataprovidername, placename, referencetime, validfrom, validto, "
		       "valueparametername, valueparameterunit, levelparametername, "
		       "levelunitname, levelfrom, levelto, dataversion, astext(placegeometry) as point";

	return "value, dataprovidername, placename, referencetime, validtimefrom, validtimeto, "
           "valueparametername, valueparameterunit, levelparametername, "
           "levelunitname, levelfrom, levelto, dataversion, astext(placegeometry) as point";
}

string
wciTimeSpec( int wciProtocol, const boost::posix_time::ptime &reftimespec )
{
	if( reftimespec.is_special() )
		return "NULL";

	string sReftime = miutil::isotimeString( reftimespec, false, true );
	ostringstream o;

	if( wciProtocol == 1 )
		o << "('" << sReftime << "', '" << sReftime << "', 'exact')";
	else if( wciProtocol == 2 )
		o << "'exact " << sReftime <<"'";
	else if( wciProtocol > 2 )
		o << "'exact " << sReftime <<"'";
	else
		o << "'exact " << sReftime <<"'";

	return o.str();
}


string
wciLevelSpec( int wciProtocol, const wdb2ts::ParamDef &paramDef )
{
	ostringstream ost;

	if( wciProtocol == 1 ) {
		ost << "(" << paramDef.levelfrom() << ", " << paramDef.levelto()
			<< ", '" << paramDef.levelparametername() << "', 'exact')";
	} else if( wciProtocol == 2 ) {
		if( paramDef.levelfrom() == paramDef.levelto() )
			ost << "'exact  " << paramDef.levelfrom() << " " << paramDef.levelparametername() << "'";
		else
			ost << "'inside " << paramDef.levelfrom() << " TO " << paramDef.levelto() << " "
	            << paramDef.levelparametername() << "'";
	} else if( wciProtocol > 2 ) {
		if( paramDef.levelfrom() == paramDef.levelto() )
			ost << "'exact  " << paramDef.levelfrom() << " " << paramDef.levelparametername() << "'";
		else
			ost << "'inside " << paramDef.levelfrom() << " TO " << paramDef.levelto() << " "
				<< paramDef.levelparametername() << "'";
	} else {
		if( paramDef.levelfrom() == paramDef.levelto() )
			ost << "'exact  " << paramDef.levelfrom() << " " << paramDef.levelparametername() << "'";
		else
			ost << "'inside " << paramDef.levelfrom() << " TO " << paramDef.levelto() << " "
			    << paramDef.levelparametername() << "'";
	}

	return ost.str();
}

string
wciValueParameter( int wciProtocol, const std::list<wdb2ts::ParamDef> &paramDefs )
{
	ostringstream ost;
	std::list<wdb2ts::ParamDef>::const_iterator it = paramDefs.begin();

	if( it == paramDefs.end() )
		return "NULL";

	ost << "ARRAY['" << it->valueparametername() << "'";
	++it;

	for( ; it != paramDefs.end(); ++it )
		ost << ",'" << it->valueparametername() << "'";

	ost << "]";
	return ost.str();
}


}

