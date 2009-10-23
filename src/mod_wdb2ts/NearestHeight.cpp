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


#include <sstream>
#include <Logger4cpp.h>
#include <splitstr.h>
#include <NearestHeight.h>
#include <ProviderList.h>

using namespace std;

namespace wdb2ts {

void
NearestHeight::
decode( const wdb2ts::config::ActionParam &conf, const std::string &prefix, wdb2ts::NearestHeights &nearestHeights, ostream &msg )
{
	for( wdb2ts::config::ActionParam::const_iterator it = conf.begin(); it!=conf.end(); ++it ) {
		string::size_type i=it->first.find( prefix );

		if( i != string::npos && i==0 ) {

			wdb2ts::ProviderItem item = wdb2ts::ProviderList::decodeItem( it->first );
			string provider = item.providerWithPlacename();
			provider.erase(0, prefix.size() );

			if( provider.empty() )
				continue;

			msg << "Nearest height: " << provider << endl;
			//Set the modelTopoProvider to the same as the provider as default.
			nearestHeights[provider].modelTopoProvider_ = provider;

			vector<string> params = miutil::splitstr( it->second.asString(), ',', '\'');

			for( vector<string>::size_type i=0; i < params.size(); ++i ) {
				vector<string> values = miutil::splitstr( params[i], ':', '\'');
				string paramName = values[0];
				string providerName;

				if( paramName.empty() )
					continue;

				if( values.size() > 1 ) {
					wdb2ts::ProviderItem itemTmp = wdb2ts::ProviderList::decodeItem( values[1] );
					providerName = itemTmp.providerWithPlacename();
				} else {
					providerName = provider;
				}

				if( paramName == "RENAME" )
					nearestHeights[provider].renameTo_ = providerName;
				else if( paramName == "MODEL.TOPOGRAPHY" && ! providerName.empty()  )
					nearestHeights[provider].modelTopoProvider_ = providerName;
				else if( paramName == "TOPOGRAPHY" && ! providerName.empty()  )
					nearestHeights[provider].topoProvider_ = providerName;
				else
					nearestHeights[provider].paramWithProvider[paramName] = providerName;
			}

			msg << "       topoProvider: " << nearestHeights[provider].topoProvider() << endl
				<< "  modelTopoProvider: " << nearestHeights[provider].modelTopoProvider() << endl
				<< "             rename: ";

			if( nearestHeights[provider].rename() )
				msg << "(true) " << nearestHeights[provider].renameTo() << endl;
			else
				msg << "(false)" << endl;

			msg << "             params:";

			for( std::map< std::string, std::string>::iterator it = nearestHeights[provider].paramWithProvider.begin();
			     it != nearestHeights[provider].paramWithProvider.end();
			     ++it )
				msg << " " << it->first << ":" << it->second;

			msg << endl;
		}
	}
}

NearestHeights
NearestHeight::
configureNearestHeight( const wdb2ts::config::ActionParam &conf )
{
	stringstream msg;
	NearestHeights nearestHeights;

	WEBFW_USE_LOGGER( "handler" );
	msg << "Configure nearest height : " << endl;

	decode( conf, "nearest_height-", nearestHeights, msg );

	WEBFW_LOG_INFO( msg.str() );

	return nearestHeights;
}


void
NearestHeight::
processNearestHeightPoint( WciConnectionPtr dbcon,
                           int          realHeight,
                           LocationData &locationData,
                           const NearestHeights &nearestHeights,
                           PtrProviderRefTimes reftimes,
                           const ParamDefList &paramDefs )
{
}


}




