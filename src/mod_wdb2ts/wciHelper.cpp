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

namespace {

string
timeStr( const boost::posix_time::ptime &t )
{
	if( t.is_pos_infinity()  )
		return "infinity";
	else if( t.is_neg_infinity() )
		return "epoch";
	else if( t.is_special() )
		return "";
	else
		return miutil::isotimeString( t, false, true );
}

}

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
wciTimeSpec( int wciProtocol,
		     const boost::posix_time::ptime &from,
		     const boost::posix_time::ptime &to_ )
{
	boost::posix_time::ptime to( to_ );
	string sFrom;
	string sTo;
	if( from.is_not_a_date_time() )
		return "NULL";

	if( to.is_not_a_date_time() )
		to = from;

	sFrom = timeStr( from );
	sTo = timeStr( to );

	if( sFrom.empty() )
		return "NULL";

	if( sTo.empty() ) {
		to = from;
		sTo = sFrom;
	}

	ostringstream o;

	if( wciProtocol == 1 ) {
		if( to == from )
			o << "('" << sFrom << "', '" << sTo << "', 'exact')";
		else
			o << "('" << sFrom << "', '" << sTo << "', 'inside')";
	}else if( wciProtocol >= 2 ) {
		if( to == from )
			o << "'exact " << sTo <<"'";
		else
			o << "'inside " << sFrom << " TO " << sTo << "'";
	} else {
		if( to == from )
			o << "'exact " << sTo <<"'";
		else
			o << "'inside " << sFrom << " TO " << sTo << "'";
	}

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

