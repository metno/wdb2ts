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
decode( const wdb2ts::config::ActionParam &conf,
		const std::string &prefix,
		wdb2ts::NearestHeights &nearestHeights,
		const std::string &defaultDbId,
		ostream &msg )
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
				string value;

				if( paramName.empty() )
					continue;

				if( values.size() > 1 ) {
					wdb2ts::ProviderItem itemTmp = wdb2ts::ProviderList::decodeItem( values[1] );
					value = itemTmp.providerWithPlacename();
				} else {
					value = provider;
				}

				if( paramName == "RENAME" )
					nearestHeights[provider].renameTo_ = value;
				else if( paramName == "MODEL.TOPOGRAPHY" && ! value.empty()  )
					nearestHeights[provider].modelTopoProvider_ = value;
				else if( paramName == "TOPOGRAPHY" && ! value.empty()  )
					nearestHeights[provider].topoProvider_ = value;
				else if( paramName=="wdbdb")
					nearestHeights[provider].wdbid_= value;
				else
					nearestHeights[provider].paramWithProvider[paramName] = value;

			}

			if(nearestHeights[provider].wdbid_.empty()) {
				nearestHeights[provider].wdbid_=defaultDbId;
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
configureNearestHeight( const wdb2ts::config::ActionParam &conf,const std::string &defaultDbId )
{
	stringstream msg;
	NearestHeights nearestHeights;

	WEBFW_USE_LOGGER( "handler" );
	msg << "Configure nearest height : " << endl;

	decode( conf, "nearest_height-", nearestHeights, defaultDbId, msg );

	WEBFW_LOG_INFO( msg.str() );

	return nearestHeights;
}



void
NearestHeight::
processNearestHeightPoint( ConfigData *config,
		const LocationPointList &locationPoints,
			               const boost::posix_time::ptime &to,
		                   LocationPointDataPtr data,
		                   int altitude,
		                   const ProviderList &providerPriority,
		                   const ParamDefList &params,
		                   const NearestHeights &nearestHeights,
		                   int wciProtocol
				          )
{
	ParamDefPtr itParam;
	ParamDef topoParam;
	ParamDef modelTopoParam;
	LocationPointList topoLocations;
	LocationPointList modelTopoLocations;
	bool disabled;
	int dummyDataversion;
	boost::posix_time::ptime dataRefTime;
	boost::posix_time::ptime modelTopoRefTime;
	boost::posix_time::ptime topoRefTime;
	ostringstream msg;
	bool debug;

	if( nearestHeights.empty() || locationPoints.empty() )
		return;

	WEBFW_USE_LOGGER( "handler" );
	debug = WEBFW_GET_LOGLEVEL()>= log4cpp::Priority::DEBUG;

	if( debug )
		msg << " ---- NearestHeight BEGIN -----------\n";
	for( NearestHeights::const_iterator it=nearestHeights.begin();
		 it != nearestHeights.end();
		++it )
	{

		if( debug )
			msg << "\nNearestHeight (" + it->first << ")\n";

		WciConnectionPtr wciConnection;
		if( it->second.topoProvider().empty() ) {
			if( debug )
				msg << "  topoProvider: '" << it->second.topoProvider() << "' No TOPOGRAPHY provider defined\n";

			WEBFW_LOG_WARN( "Nearest height: No TOPOGRAPHY provider defined in 'nearest_height-"<< it->first << "'.");
			continue;
		}

		if( debug )
			msg << "  topoProvider: '" << it->second.topoProvider() << "'\n";

		if( ! params.findParam( itParam, "TOPOGRAPHY", it->second.topoProvider() ) ) {
			WEBFW_LOG_WARN( "Nearest height: No parameter definition for TOPOGRAPHY, provider '" << it->second.topoProvider() << "'.");
			msg << "   topoProvider: No parameter defined for TOPOGRAPHY, provider '" << it->second.topoProvider() << "'\n";
			continue;
		}


		topoParam = *itParam;

		if( ! params.findParam( itParam, "MODEL.TOPOGRAPHY", it->second.modelTopoProvider() ) ) {
			if( debug )
				msg << "   modelTopoProvider: No parameter definition for MODEL.TOPOGRAPHY, provider '" << it->second.modelTopoProvider() << "'\n";

			WEBFW_LOG_WARN( "Nearest height: No parameter definition for MODEL.TOPOGRAPHY, provider '" << it->second.modelTopoProvider() << "'.");
			continue;
		}

		if( debug )
			msg << "   modelTopoProvider: '" << it->second.modelTopoProvider() << "'\n";

		modelTopoParam = *itParam;
		PtrProviderRefTimes refTimes=config->getReferenceTimeByDbId(it->first);

		if( ! refTimes->providerReftimeDisabledAndDataversion( it->first,
		                                                       dataRefTime,
		                                                       disabled,
		                                                       dummyDataversion ) ) {
			if( debug )
				msg << "   referanceTime: '" << it->first << "' No reference times found\n";

			WEBFW_LOG_WARN( "Nearest height: No reference times found for provider '" << it->first << "'. Check that the provider is listed in provider_priority.");
			continue;
		}

		if( debug ) {
			msg << "   referanceTime: '" << it->first << "'  " << dataRefTime << "\n";
			msg << "   disabled: '" << it->first << (disabled?"true":"false") << "\n";
		}

		if( disabled ) {
		   WEBFW_LOG_WARN( "Nearest height: Provider '" << it->first << "' disabled.");
		   continue;
		}

		if( ! refTimes->providerReftime( it->second.topoProvider(), topoRefTime ) ) {
			WEBFW_LOG_INFO( "Nearest height: No reference times found for TOPOGRAPHY, provider '" << it->second.topoProvider() << "'. This is expected.");
			topoRefTime = boost::posix_time::ptime();
		}

		if( debug )
			msg << "   topoReftime: '" << it->second.topoProvider() << "' " << topoRefTime <<"\n";

		if( ! refTimes->providerReftime( it->second.modelTopoProvider(), modelTopoRefTime ) ) {
			WEBFW_LOG_INFO( "Nearest height: No reference times found for MODEL.TOPOGRAPHY, provider '" << it->second.modelTopoProvider() << "'. This is NOT unususal.");
			modelTopoRefTime = boost::posix_time::ptime();
		}

		if( debug )
			msg << "   modelTopoReftime: '" << it->second.modelTopoProvider() << "' " << modelTopoRefTime <<"\n";

		try{
			wciConnection=config->newWciConnection( it->second.topoProvider());
			Topography topographyTransactor( locationPoints.begin()->latitude(),
					                         locationPoints.begin()->longitude(),
					                         topoParam,
					                         it->second.topoProvider(),
					                         topoRefTime,
					                         false,
					                         wciProtocol );

			wciConnection->perform( topographyTransactor );
			topoLocations = topographyTransactor.result();

			if( debug )
				msg << "   topoLocation size: " << topoLocations.size() <<"\n";

			if( topoLocations.size() == 0 ) {
				WEBFW_LOG_WARN( "Nearest height: No location heights found for TOPOGRAPHY, provider '" << it->second.topoProvider() << "'.");
				continue;
			}

			wciConnection=config->newWciConnection( it->second.modelTopoProvider());
			Topography modelTopographyTransactor( locationPoints.begin()->latitude(), locationPoints.begin()->longitude(),
    											  modelTopoParam,
												  it->second.modelTopoProvider(),
												  modelTopoRefTime,
												  true,
												  wciProtocol );

			wciConnection->perform( modelTopographyTransactor );
			modelTopoLocations = modelTopographyTransactor.result();

			if( debug )
				msg << "   modelTopoLocations: " << modelTopoLocations.size()  <<"\n";

			if( modelTopoLocations.size() == 0 ) {
				WEBFW_LOG_WARN( "Nearest height: No location heights found for MODEL.TOPOGRAPHY, provider '" << it->second.topoProvider() << "'.");
				continue;
			}

		}
		catch( const miutil::pgpool::DbNoConnectionException &ex) {
			if( debug )
				msg << "   " << it->first << ": No DB connection.\n";

			WEBFW_LOG_ERROR( "nearestHeight: EXCEPTION: " << ex.what()  );
			throw;
		}
		catch( const exception &ex ) {
			if( debug )
				msg << "   " << it->first << ": DB Exception: " << ex.what() << "\n";
			WEBFW_LOG_WARN( "nearestHeight: EXCEPTION: " << ex.what()  );
			continue;
		}

		WEBFW_LOG_DEBUG( "Nearest height: #locations: " << topoLocations.size()
				         << " lat: " << topoLocations.begin()->latitude()
				         << " lon: " << topoLocations.begin()->longitude()
				         << " height: " << topoLocations.begin()->asInt() );

		int testHeight = topoLocations.begin()->asInt();
		LocationPointList::iterator itMinDiff=modelTopoLocations.end();
		int minDiff=INT_MAX;
		int diff;

		for( LocationPointList::iterator itModelTopoLocation=modelTopoLocations.begin();
		     itModelTopoLocation != modelTopoLocations.end();
		     ++itModelTopoLocation )
		{
			diff = abs( testHeight - itModelTopoLocation->asInt() );

			if(  diff < minDiff ) {
				itMinDiff = itModelTopoLocation;
				minDiff = diff;
			}
		}

		if( itMinDiff == modelTopoLocations.end() ) {
			WEBFW_LOG_ERROR( "Nearest height: Cant find a Neareast height. This is a bug." );
			msg << "    Cant find a Neareast height. This is a bug.n";
			continue;
		}

		if( debug )
			msg <<  "   Nearest location: "
				<< " lat: " << itMinDiff->latitude()
				<< " lon: " << itMinDiff->longitude()
				<< " height: " << itMinDiff->asInt() << "\n";



		WEBFW_LOG_DEBUG( "Nearest height: Nearest location: "
				         << " lat: " << itMinDiff->latitude()
				         << " lon: " << itMinDiff->longitude()
				         << " height: " << itMinDiff->asInt() );

		ParamDefList dataForThisParams;

		std::map< std::string, std::string> nearestHeightParams = it->second.params();

		for( std::map< std::string, std::string>::iterator itNearestHeightParams=nearestHeightParams.begin();
			 itNearestHeightParams != nearestHeightParams.end();
			 ++itNearestHeightParams )
		{
			if( ! params.findParam( itParam, itNearestHeightParams->first, itNearestHeightParams->second ) ) {
				WEBFW_LOG_WARN( "Nearest height: No parameter definition for '" << itNearestHeightParams->first << ", provider '"
						         << itNearestHeightParams->second << "'.");
				if( debug )
					msg << "   No parameter definition for '" << itNearestHeightParams->first << ", provider '"
					    << itNearestHeightParams->second << "'.\n";
				continue;
			}

			dataForThisParams[itNearestHeightParams->second].push_back( *itParam );
		}

		try{
			wciConnection=config->newWciConnection( it->first);
			LocationPointRead locationReadTransactor( itMinDiff->latitude(), itMinDiff->longitude(),
					                                  dataForThisParams, providerPriority,
					                                  *refTimes, to, data, wciProtocol );
			wciConnection->perform( locationReadTransactor );
		}
		catch( const miutil::pgpool::DbNoConnectionException &ex) {
			WEBFW_LOG_ERROR( "nearestHeight: EXCEPTION: " << ex.what()  );

			if( debug ) {
				msg << "   No DB CONNECTION (" << it->first << ")\n";
				cerr << msg.str();
			}
			throw;
		}
		catch( const exception &ex ) {
			WEBFW_LOG_WARN( "nearestHeight: EXCEPTION: " << ex.what()  );
			if( debug )
				msg << "   DB excetion:  (" << it->first << ")"  << ex.what() << "\n";
			continue;
		}
	}

	if( debug ) {
		msg << " ---- NearestHeight END -----------\n";
		cerr << msg.str();
	}
}


}




