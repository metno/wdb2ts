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
#include <NearestLand.h>
#include <ProviderList.h>
#include <ParamDef.h>
#include <DbManager.h>
#include <transactor/LocationPointMatrixData.h>
#include <vector>

using namespace std;

namespace wdb2ts {

NearestLand::
NearestLand(const LocationPointList &locationPoints_,
            const boost::posix_time::ptime &to_,
            LocationPointDataPtr data_,
            int altitude_,
            PtrProviderRefTimes refTimes_,
            const ProviderList &providerPriority_,
            const ParamDefList &params_,
            const NearestLandConf &nearestLands_,
            int wciProtocol_,
            WciConnectionPtr wciConnection_)
   : locationPoints( locationPoints_ ), to( to_ ), data( data_ ), altitude( altitude_ ),
     refTimes( refTimes_ ), providerPriority( providerPriority_ ), params( params_ ),
     nearestLands( nearestLands_ ), wciProtocol( wciProtocol_ ), wciConnection( wciConnection_ )
{

}

void
NearestLand::
decode( const wdb2ts::config::ActionParam &conf, const std::string &prefix, wdb2ts::NearestLandConf &nearestLands, ostream &msg )
{
   for( wdb2ts::config::ActionParam::const_iterator it = conf.begin(); it!=conf.end(); ++it ) {
      string::size_type i=it->first.find( prefix );

      if( i != string::npos && i==0 ) {
         wdb2ts::ProviderItem item = wdb2ts::ProviderList::decodeItem( it->first );
         string provider = item.providerWithPlacename();
         provider.erase(0, prefix.size() );

         if( provider.empty() )
            continue;

         msg << "Nearest land: " << provider << endl;
         //Set the modelTopoProvider to the same as the provider as default.
         nearestLands[provider].landmaskProvider_ = provider;

         //Set the modelTopoProvider to the same as the provider as default.
         nearestLands[provider].modelTopoProvider_ = provider;


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
               nearestLands[provider].renameTo_ = providerName;
            else if( paramName == "LANDCOVER" && ! providerName.empty()  )
               nearestLands[provider].landmaskProvider_ = providerName;
            else if( paramName == "MODEL.TOPOGRAPHY" && ! providerName.empty()  )
               nearestLands[provider].modelTopoProvider_ = providerName;
            else
               nearestLands[provider].paramWithProvider[paramName] = providerName;
         }

         msg << "   landmaskProvider: " << nearestLands[provider].landmaskProvider() << endl
             << "  modelTopoProvider:" << nearestLands[provider].modelTopoProvider() << endl
             << "             rename: ";

         if( nearestLands[provider].rename() )
            msg << "(true) " << nearestLands[provider].renameTo() << endl;
         else
            msg << "(false)" << endl;

         msg << "             params:";

         for( std::map< std::string, std::string>::iterator it = nearestLands[provider].paramWithProvider.begin();
               it != nearestLands[provider].paramWithProvider.end();
               ++it )
            msg << " " << it->first << ":" << it->second;

         msg << endl;
      }
   }
}

NearestLandConf
NearestLand::
configureNearestLand( const wdb2ts::config::ActionParam &conf )
{
   stringstream msg;
   NearestLandConf nearestLands;

   WEBFW_USE_LOGGER( "handler" );
   msg << "Configure nearest land : " << endl;

   decode( conf, "nearest_land-", nearestLands, msg );

   WEBFW_LOG_INFO( msg.str() );

   return nearestLands;
}



void
NearestLand::
processNearestLandPoint( )
{
/*
   const int suroundLevels=3;
	ParamDefPtr itParam;
	ParamDef landmaskParam;
	ParamDef modelTopoParam;
	LocationPointMatrix modelTopoLocations;
	LocationPointMatrix landMaskLocations;
	LocationPointMatrixTimeserie paramDataLocations;
	boost::posix_time::ptime dataRefTime;
	boost::posix_time::ptime modelTopoRefTime;
	boost::posix_time::ptime landMaskRefTime;
	LocationPointMatrixTimeserie::XYPoints landPoints;
	int ALP;

	if( nearestLands.empty() || locationPoints.empty() )
		return;

	landPoints.reserve( 4*suroundLevels*suroundLevels); //(2*suroundLevels)^2


	WEBFW_USE_LOGGER( "handler" );

	for( NearestLandConf::const_iterator it=nearestLands.begin();
		 it != nearestLands.end();
		++it )
	{
		if( it->second.landmaskProvider().empty() ) {
			WEBFW_LOG_WARN( "Nearest land: No landmask (LANDCOVER) provider defined in 'nearest_land-"<< it->first << "'.");
			continue;
		}

		if( ! findParam( itParam, params, "LANDCOVER", it->second.landmaskProvider() ) ) {
			WEBFW_LOG_WARN( "Nearest land: No parameter definition for maxconvecprec, provider '" << it->second.landmaskProvider() << "'.");
			continue;
		}

		landmaskParam = *itParam;

		if( ! findParam( itParam, params, "MODEL.TOPOGRAPHY", it->second.modelTopoProvider() ) ) {
		   WEBFW_LOG_WARN( "Nearest land: No parameter definition for MODEL.TOPOGRAPHY, provider '" << it->second.modelTopoProvider() << "'.");
		   continue;
		}

		modelTopoParam = *itParam;

		if( ! refTimes->providerReftime( it->first, dataRefTime ) ) {
			WEBFW_LOG_WARN( "Nearest land: No reference times found for provider '" << it->first << "'. Check that the provider is listed in provider_priority.");
			continue;
		}

		if( ! refTimes->providerReftime( it->second.modelTopoProvider(), modelTopoRefTime ) ) {
			WEBFW_LOG_INFO( "Nearest land: No reference times found for MODEL.TOPOGRAPHY, provider '" << it->second.modelTopoProvider() << "'. This is NOT unususal.");
			modelTopoRefTime = boost::posix_time::ptime();
		}

		if( ! refTimes->providerReftime( it->second.landmaskProvider(), landMaskRefTime ) ) {
		   WEBFW_LOG_INFO( "Nearest land: No reference times found for LANDCOVER, provider '" << it->second.landmaskProvider() << "'. This is NOT unususal.");
		   landMaskRefTime = boost::posix_time::ptime();
		}

		try{
			LocationPointMatrixData modelTopoTransactor( locationPoints.begin()->latitude(),
			                                             locationPoints.begin()->longitude(),
			                                             modelTopoParam,
			                                             it->second.modelTopoProvider(),
			                                             modelTopoRefTime,
			                                             suroundLevels,
			                                             wciProtocol );

			wciConnection->perform( modelTopoTransactor );

			if( modelTopoTransactor.result()->size() == 0 ) {
			   WEBFW_LOG_ERROR( "Nearest land: No data  found for MODEL.TOPOGRAPHY, provider '" << it->second.modelTopoProvider() << "'.");
			   continue;
			}

			modelTopoLocations = *modelTopoTransactor.result()->beginToTime()->second.second;  //It should be only one field for topography

			LocationPointMatrixData landMaskTransactor( locationPoints.begin()->latitude(),
			                                            locationPoints.begin()->longitude(),
			                                            landmaskParam,
			                                            it->second.landmaskProvider(),
			                                            landMaskRefTime,
			                                            suroundLevels,
			                                            wciProtocol );

			wciConnection->perform( landMaskTransactor );

			if( landMaskTransactor.result()->size() == 0 ) {
			   WEBFW_LOG_ERROR( "Nearest land: No data  found for LANDCOVER, provider '" << it->second.landmaskProvider() << "'.");
			   continue;
			}

			landMaskLocations = *landMaskTransactor.result()->beginToTime()->second.second;  //It should be only one field for topography


		}
		catch( const exception &ex ) {
			WEBFW_LOG_WARN( "Nearest land: EXCEPTION: " << ex.what()  );
			continue;
		}

		LocationPointMatrix *landMask =
		ALP = LocationPointMatrixTimeserie::valuesGreaterThan( landMaskLocations, 1, 0.9, landPoints );

		WEBFW_LOG_DEBUG("NearestLand: ALP1: " << ALP );

	     if( ALP >= 4 )
	        return false;

	     if( ALP > 0 )
	        return computeNearestLand( locationPoints, altitude, to, 1, landPoints, modelTopoLocations, );

	     if( ! getFildDataSuroundN( landseamask[model], x, y, xpos.vector, ypos.vector,
	                             landSeaValues.vector, 2, landSeaValues.col, landSeaValues.row ) )
	        return false;

	#ifdef DEBUGPRINT
	     cerr << "ipDataCache::getDataFromNearestLand: ALP2: " << ALP << endl;
	#endif

	     ALP = landSeaValues.countValuesGT( LANDSEA_LIMIT );

	     if( ALP > 0 )
	        return computeNearestLand( xpos, ypos, landSeaValues, hoh, model, params, data, error );

	     if( ! getFildDataSuroundN( landseamask[model], x, y, xpos.vector, ypos.vector,
	                              landSeaValues.vector, 3, landSeaValues.col, landSeaValues.row ) )
	        return false;

	     ALP = landSeaValues.countValuesGT( LANDSEA_LIMIT );

	#ifdef DEBUGPRINT
	     cerr << "ipDataCache::getDataFromNearestLand: ALP3: " << ALP << endl;
	#endif

	     if( ALP > 0 )
	        return computeNearestLand( xpos, ypos, landSeaValues, hoh, model, params, data, error );

	     return false;
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
		*/
	}

}






