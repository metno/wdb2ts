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
#include <ParamDef.h>
#include <DbManager.h>
#include <transactor/Topography.h>
#include <transactor/LocationPointRead.h>


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
processNearestHeightPoint( const LocationPointList &locationPoints,
			               const boost::posix_time::ptime &to,
		                   LocationPointDataPtr data,
		                   int altitude,
		                   PtrProviderRefTimes refTimes,
		                   const ProviderList &providerPriority,
		                   const ParamDefList &params,
		                   const NearestHeights &nearestHeights,
		                   int wciProtocol,
		                   WciConnectionPtr wciConnection
				          )
{
	ParamDefPtr itParam;
	ParamDef topoParam;
	ParamDef modelTopoParam;
	LocationPointList topoLocations;
	LocationPointList modelTopoLocations;
	boost::posix_time::ptime dataRefTime;
	boost::posix_time::ptime modelTopoRefTime;
	boost::posix_time::ptime topoRefTime;


	if( nearestHeights.empty() || locationPoints.empty() )
		return;

	WEBFW_USE_LOGGER( "handler" );

	for( NearestHeights::const_iterator it=nearestHeights.begin();
		 it != nearestHeights.end();
		++it )
	{
		if( it->second.topoProvider().empty() ) {
			WEBFW_LOG_WARN( "Nearest height: No TOPOGRAPHY provider defined in 'nearest_height-"<< it->first << "'.");
			continue;
		}

		if( ! findParam( itParam, params, "TOPOGRAPHY", it->second.topoProvider() ) ) {
			WEBFW_LOG_WARN( "Nearest height: No parameter definition for TOPOGRAPHY, provider '" << it->second.topoProvider() << "'.");
			continue;
		}

		topoParam = *itParam;

		if( ! findParam( itParam, params, "MODEL.TOPOGRAPHY", it->second.modelTopoProvider() ) ) {
			WEBFW_LOG_WARN( "Nearest height: No parameter definition for MODEL.TOPOGRAPHY, provider '" << it->second.modelTopoProvider() << "'.");
			continue;
		}

		modelTopoParam = *itParam;

		if( ! refTimes->providerReftime( it->first, dataRefTime ) ) {
			WEBFW_LOG_WARN( "Nearest height: No reference times found for provider '" << it->first << "'. Check that the provider is listed in provider_priority.");
			continue;
		}

		if( ! refTimes->providerReftime( it->second.topoProvider(), topoRefTime ) ) {
			WEBFW_LOG_INFO( "Nearest height: No reference times found for TOPOGRAPHY, provider '" << it->second.topoProvider() << "'. This is expected.");
			topoRefTime = boost::posix_time::ptime();
		}

		if( ! refTimes->providerReftime( it->second.modelTopoProvider(), modelTopoRefTime ) ) {
			WEBFW_LOG_INFO( "Nearest height: No reference times found for MODEL.TOPOGRAPHY, provider '" << it->second.modelTopoProvider() << "'. This is NOT unususal.");
			modelTopoRefTime = boost::posix_time::ptime();
		}

		try{
			Topography topographyTransactor( locationPoints.begin()->latitude(),
					                         locationPoints.begin()->longitude(),
					                         topoParam,
					                         it->second.topoProvider(),
					                         topoRefTime,
					                         false,
					                         wciProtocol );

			wciConnection->perform( topographyTransactor );
			topoLocations = topographyTransactor.result();

			if( topoLocations.size() == 0 ) {
				WEBFW_LOG_WARN( "Nearest height: No location heights found for TOPOGRAPHY, provider '" << it->second.topoProvider() << "'.");
				continue;
			}

			Topography modelTopographyTransactor( locationPoints.begin()->latitude(), locationPoints.begin()->longitude(),
    											  modelTopoParam,
												  it->second.modelTopoProvider(),
												  modelTopoRefTime,
												  true,
												  wciProtocol );

			wciConnection->perform( modelTopographyTransactor );
			modelTopoLocations = modelTopographyTransactor.result();

			if( modelTopoLocations.size() == 0 ) {
				WEBFW_LOG_WARN( "Nearest height: No location heights found for MODEL.TOPOGRAPHY, provider '" << it->second.topoProvider() << "'.");
				continue;
			}

		}
		catch( const exception &ex ) {
			WEBFW_LOG_WARN( "nearestHeight: EXCEPTION: " << ex.what()  );
			continue;
		}

		WEBFW_LOG_DEBUG( "Nearest height: #locations: " << topoLocations.size()
				         << " lat: " << topoLocations.begin()->latitude()
				         << " lon: " << topoLocations.begin()->longitude()
				         << " height: " << topoLocations.begin()->height() );

		int testHeight = topoLocations.begin()->height();
		LocationPointList::iterator itMinDiff=modelTopoLocations.end();
		int minDiff=INT_MAX;
		int diff;

		for( LocationPointList::iterator itModelTopoLocation=modelTopoLocations.begin();
		     itModelTopoLocation != modelTopoLocations.end();
		     ++itModelTopoLocation )
		{
			diff = abs( testHeight - itModelTopoLocation->height() );

			if(  diff < minDiff ) {
				itMinDiff = itModelTopoLocation;
				minDiff = diff;
			}
		}

		if( itMinDiff == modelTopoLocations.end() ) {
			WEBFW_LOG_ERROR( "Nearest height: Cant find a Neareast height. This is a bug." );
			continue;
		}

		WEBFW_LOG_DEBUG( "Nearest height: Nearest location: "
				         << " lat: " << itMinDiff->latitude()
				         << " lon: " << itMinDiff->longitude()
				         << " height: " << itMinDiff->height() );

		ParamDefList dataForThisParams;

		std::map< std::string, std::string> nearestHeightParams = it->second.params();

		for( std::map< std::string, std::string>::iterator itNearestHeightParams=nearestHeightParams.begin();
			 itNearestHeightParams != nearestHeightParams.end();
			 ++itNearestHeightParams )
		{
			if( ! findParam( itParam, params, itNearestHeightParams->first, itNearestHeightParams->second ) ) {
				WEBFW_LOG_WARN( "Nearest height: No parameter definition for '" << itNearestHeightParams->first << ", provider '"
						         << itNearestHeightParams->second << "'.");
				continue;
			}

			dataForThisParams[itNearestHeightParams->second].push_back( *itParam );
		}

		try{
			LocationPointRead locationReadTransactor( itMinDiff->latitude(), itMinDiff->longitude(),
					                                  dataForThisParams, providerPriority,
					                                  *refTimes, to, data, wciProtocol );
			wciConnection->perform( locationReadTransactor );

		}
		catch( const exception &ex ) {
			WEBFW_LOG_WARN( "nearestHeight: EXCEPTION: " << ex.what()  );
			continue;
		}
	}
}


}




