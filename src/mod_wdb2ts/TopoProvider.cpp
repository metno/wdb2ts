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
#include <ProviderList.h>
#include <TopoProvider.h>
#include <Logger4cpp.h>
#include <splitstr.h>
using namespace std;

namespace {

void
decode( const wdb2ts::config::ActionParam &conf, const std::string &prefix, wdb2ts::TopoProviderMap &providerMap, ostream &msg )
{
	for( wdb2ts::config::ActionParam::const_iterator it = conf.begin(); it!=conf.end(); ++it ) {
		string::size_type i=it->first.find( prefix );

		if( i != string::npos && i==0 ) {

			wdb2ts::ProviderItem item = wdb2ts::ProviderList::decodeItem( it->first );
			string provider = item.providerWithPlacename();
			provider.erase(0, prefix.size() );
			item = wdb2ts::ProviderList::decodeItem( it->second.asString() );
			msg <<  " --- topo provider alias: " << provider << " --> " << item.providerWithPlacename() << endl;
			providerMap[provider].push_back( item.providerWithPlacename() );
		}
	}
}


}


namespace wdb2ts {

TopoProviderMap
configureModelTopographyProvider( const wdb2ts::config::ActionParam &conf )
{
	stringstream msg;
	TopoProviderMap topoProviders;

	WEBFW_USE_LOGGER( "handler" );
	msg << "Configure model topo providers: " << endl;

	decode( conf, "model_topo_provider-", topoProviders, msg );

	WEBFW_LOG_INFO( msg.str() );

	return topoProviders;
}


/*
std::list<std::string>
configureTopographyProvider( const wdb2ts::config::ActionParam &conf )
{
	stringstream msg;
	TopoProviderMap topoProviders;

	WEBFW_USE_LOGGER( "handler" );
	msg << "Configure topography providers: " << endl;

	decode( conf, "topography_provider-", topoProviders, msg );

	WEBFW_LOG_INFO( msg.str() );

	return topoProviders;

}
*/

std::list<std::string>
configureTopographyProvider( const wdb2ts::config::ActionParam &conf )
{
	std::list<std::string> topoList;

	WEBFW_USE_LOGGER( "handler" );

	WEBFW_LOG_DEBUG( " ---@@@@@@@@@@@@@@ topo provider: configureTopographyProvider" );

	for( wdb2ts::config::ActionParam::const_iterator it = conf.begin(); it!=conf.end(); ++it ) {
		string::size_type i=it->first.find( "topography_provider");

		if( i != string::npos && i==0 ) {
			vector<string> providers = miutil::splitstr( it->second.asString(), ';' );

			for(vector<string>::size_type i=0; i<providers.size(); ++i  ) {
				wdb2ts::ProviderItem item = wdb2ts::ProviderList::decodeItem( providers[i] );
				WEBFW_LOG_DEBUG( " ---@@@@@@@@@@@@@@ topo provider " <<  item.providerWithPlacename() );
				topoList.push_back( item.providerWithPlacename() );
			}
		}
	}

	return topoList;
}




}




