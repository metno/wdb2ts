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

#include <boost/date_time/posix_time/posix_time.hpp>
#include <ptimeutil.h>
#include <transactor/ProviderRefTime.h>
#include <wdb2tsProfiling.h>

DECLARE_MI_PROFILE;

using namespace std;

namespace wdb2ts {

ProviderRefTime::
ProviderRefTime( const ProviderRefTimeList &refTimes_, 
         		  const string &provider_,
         		  const string &reftimespec_  )
	: refTimes( refTimes_ ), provider( provider_ ),
	  reftimespec( reftimespec_ ),
	  refTimesResult( new ProviderRefTimeList() )
{
}

ProviderRefTime::
~ProviderRefTime()
{
}

void 
ProviderRefTime::
operator () ( argument_type &t )
{
	ostringstream ost;
	*refTimesResult=refTimes;

	ost << "SELECT * FROM wci.browse( ARRAY['" << provider <<"'], NULL,"<< reftimespec 
	    << ",NULL,NULL,NULL,NULL,NULL::wci.browseplace)";

	cerr << "ProviderReftimes (Transactor): query: " << ost.str() << endl;
	
	USE_MI_PROFILE;
	MARK_ID_MI_PROFILE("ProviderRefTimes (Transactor)");
	
	try {
		pqxx::result  res = t.exec( ost.str() );
	      
		if( res.size() == 0 )
			return;
	        
		for( pqxx::result::const_iterator it=res.begin(); it != res.end(); ++it  ) 
			if( ! it.at("referencetimeto").is_null() )
				(*refTimesResult)[ ProviderItem( provider, it.at("placename").c_str() ).providerWithPlacename() ] =
					ProviderTimes( miutil::ptimeFromIsoString( it.at("referencetimeto").c_str() ),
							         boost::posix_time::second_clock::universal_time()   
				                );
	}
	catch( ... ) {
		MARK_ID_MI_PROFILE("ProviderRefTimes (Transactor)");
		throw;
	}
	
	MARK_ID_MI_PROFILE("ProviderRefTimes (Transactor)");
}

}

