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
#include <transactor/LocationPointRead.h>
#include <wciHelper.h>
#include <PqTupleContainer.h>

using namespace std;

namespace wdb2ts {



LocationPointRead::
LocationPointRead( float latitude, float longitude,
        const ParamDefList &paramDefs,
        const ProviderList &providers,
        const ProviderRefTimeList &refTimeList,
        const boost::posix_time::ptime &to,
        LocationPointDataPtr locationPointData,
        int wciProtocol )
	: latitude_( latitude ), longitude_( longitude ),
	  paramDefs_( paramDefs ), providers_( providers ),refTimeList_( refTimeList ),
	  to_( to ), wciProtocol_( wciProtocol ),
	  locationPointData_( locationPointData )
{
}


LocationPointRead::
~LocationPointRead()
{
}

/*
 * 	   ost << "SELECT " << returnColoumns << " FROM wci.read(" << endl
		   << "   " << dataprovider.selectPart() << ", " << endl
		   << "   '" << sPointInterpolation << " POINT(" <<  longitude.value() << " " << latitude.value() << ")', " << endl
//       	<< "   'POINT(" <<  longitude.value() << " " << latitude.value() << ")', " << endl
		   << "   " << reftime.selectPart() << ", " << endl
		   << "   " << validtime.selectPart() << ", " << endl
		   << "   " << parameter.selectPart() << ", " << endl
		   << "   " << levelspec.selectPart() << ", " << endl
		   << "   " << dataversion.selectPart() << ", " << endl
		   << "   NULL::wci.returnfloat )" << endl;
 *
 */

void
LocationPointRead::
operator () ( argument_type &t )
{
	ostringstream q;
	boost::posix_time::ptime refTime;
	string validTime;
	bool disabled;
	int dataversion;

	WEBFW_USE_LOGGER( "wdb" );

	if( to_.is_special() )
		validTime = "NULL";
	else
		validTime = wciTimeSpec( wciProtocol_,
				                 boost::posix_time::ptime( boost::posix_time::neg_infin), to_ );


	for( ParamDefList::const_iterator itPar=paramDefs_.begin();
		itPar != paramDefs_.end();
		++itPar )
	{
		if( ! refTimeList_.providerReftimeDisabledAndDataversion(itPar->first,
		                                                         refTime,
		                                                         disabled,
		                                                         dataversion ) ) {
			WEBFW_LOG_WARN("No reference times found for provider '" << itPar->first << "'." <<
					        "Check that the provider is listed in provider_priority.");
			continue;
		}

		if( disabled ) {
		   WEBFW_LOG_WARN("Provider '" << itPar->first << "' disabled." );
		   continue;
		}

		if( dataversion < -1 )
		   dataversion = -1;

		q.str("");

		q << "SELECT " << wciReadReturnColoumns( wciProtocol_ ) << " FROM wci.read(" << endl
	      << "ARRAY['" << itPar->first << "'], " << endl
	      << "'nearest POINT(" << longitude_ << " " << latitude_ << ")', " << endl
	      << wciTimeSpec( wciProtocol_, refTime ) << ", " << endl
	      << validTime << ", " << endl
	      << wciValueParameter( wciProtocol_, itPar->second ) << ", " << endl
	      << "NULL, " << endl
	      << "ARRAY[" << dataversion << "], NULL::wci.returnfloat ) ORDER BY referencetime";

		WEBFW_LOG_DEBUG( "LocationPointRead: transactor: SQL ["	<< q.str() << "]" );
		pqxx::result  res = t.exec( q.str() );

		//miutil::container::PqContainer container( res );
		decodePData( paramDefs_, providers_, refTimeList_, res, false, *locationPointData_, wciProtocol_ );
	}
}

}
