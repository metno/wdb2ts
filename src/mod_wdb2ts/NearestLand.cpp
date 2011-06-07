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
#include <iostream>
#include <splitstr.h>
#include <NearestLand.h>
#include <ProviderList.h>
#include <ParamDef.h>
#include <DbManager.h>
#include <ptimeutil.h>
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
         nearestLands[provider].provider_ = provider;

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
               << "  modelTopoProvider: " << nearestLands[provider].modelTopoProvider() << endl
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

   WEBFW_USE_LOGGER( "nearest_land" );
   msg << "Configure nearest land : " << endl;

   decode( conf, "nearest_land-", nearestLands, msg );

   WEBFW_LOG_INFO( msg.str() );

   return nearestLands;
}

void
NearestLand::
setData( const ParamDef &param,
         const std::string &paramname,
         const std::string &provider,
         const LocationPointMatrix &modeltopo,
         const LocationPointMatrixTimeserie::XYPoints &xyPoints,
         int suroundLevel,
         LocationPointMatrixTimeserie &data
)
{
   WEBFW_USE_LOGGER(  "nearest_land" );
   int n;
   float sum;
   float avg;
   float val=FLT_MIN;
   std::string setParamname( paramname );

   SetPDataHelper setval;
   SetPDataHelper getval;

   if( setParamname == "T.2M" || setParamname == "T.2M.LAND" )
      setParamname = "T.2M.NO_ADIABATIC_HIGHT_CORRECTION";

   if( !setval.init( setParamname ) ) {
      WEBFW_LOG_WARN("Nearest Land: Cant set new value for param '" << paramname << "', provider '" << provider << "'. This is a bug.");
      return;
   }

   if( !getval.init( paramname ) ) {
      WEBFW_LOG_WARN("Nearest Land: Cant set new value for param '" << paramname << "', provider '" << provider << "'. This is a bug.");
      return;
   }

   if( this->data->size() == 0 ) {
      WEBFW_LOG_INFO("Nearest Land: Cant set new value for param '" << paramname << "', provider '" << provider << "'. No data.");
      return;
   }

   if( this->data->size() > 1 ) {
      int s = this->data->size();
      WEBFW_LOG_WARN("Nearest Land: Number of locations in the dataset is '" << s << "' expecting only 1 location.\n set new value for param '" << paramname << "', provider '" << provider << "' only for the first location.");
   }

   TimeSerie::iterator tit;
   FromTimeSerie::iterator fit;
   ProviderPDataList::iterator pit;


   TimeSeriePtr myDataPtr = this->data->begin()->second;

   if( ! myDataPtr ) {
      WEBFW_LOG_WARN("Nearest Land: Cant set new value for param '" << paramname << "', provider '" << provider << "'. This is a bug.");
      return;
   }

   for( LocationPointMatrixTimeserie::DataType::const_iterator it = data.beginToTime();
         it != data.endToTime(); ++it ) {
      sum = 0;
      n=0;

      for(LocationPointMatrixTimeserie::XYPoints::const_iterator itxy=xyPoints.begin();
            itxy != xyPoints.end(); ++itxy ) {
         //WEBFW_LOG_DEBUG(" ++++ setData: xy x: " << itxy->x << " Y: " << itxy->y );
         val = (it->second.second)[itxy->x][itxy->y].value();

         if( val != FLT_MIN ) {
            n++;
            if( altitude == INT_MIN )
               sum += val;
            else
               sum += ( val - (altitude - modeltopo[itxy->x][itxy->y].value())*0.006 );
         }
      }

      if( n == 0 )
         continue;

      avg = sum/n;

      tit = myDataPtr->find( it->first );

      if( tit == myDataPtr->end() )
         continue;

      fit = tit->second.find( it->second.first  );

      if( fit == tit->second.end() )
         continue;

      /*
      WEBFW_LOG_DEBUG(" ++++ setData: fit: " << provider );

      for( pit=fit->second.begin(); pit != fit->second.end(); ++pit )
         WEBFW_LOG_DEBUG(" ++++ setData: provider: " << pit->first );
      */
      pit = fit->second.find( provider );

      if( pit == fit->second.end() )
         continue;

      float newValue = correctValue( it->first, locationPoints.begin()->longitude(), avg, getval.get( pit->second ) );
      WEBFW_LOG_DEBUG("Nearest Land: [" << tit->first << "," << fit->first << "] h: " << altitude << " new value '" << newValue << "' (N: " << avg << ", O: " << getval.get( pit->second )<< ") for param '" << setParamname << "', provider '" << provider);
      setval.set( pit->second, newValue );
   }

}

float
NearestLand::
correctValue( const boost::posix_time::ptime &time,
              float longitude,
              float t1, float t2 )
{
   static const boost::posix_time::time_duration c00(0,0,0);
   static const boost::posix_time::time_duration c06(6,0,0);
   static const boost::posix_time::time_duration c12(12,0,0);
   static const boost::posix_time::time_duration c18(18,0,0);
   static const boost::posix_time::time_duration c24(24,0,0);
   float retVal;
   boost::posix_time::ptime lt = miutil::geologicalLocalTime( time, longitude );
   boost::posix_time::time_duration clock=lt.time_of_day();

   if( clock>=c12 && clock <= c18 ) { //12-18
      retVal = t1;
   } else  if( clock >= c00 && clock <= c06 ){ // 00-06
      retVal = t2;
   } else if( clock > c18 && clock < c24 ) { // 18-00
      retVal = linearInterpolateValue( clock, c18, c24, t1, t2 );
   }else {  //06-12
      retVal = linearInterpolateValue( clock, c06, c12, t2, t1 );
   }

   return retVal;
}

float
NearestLand::
linearInterpolateValue( const boost::posix_time::time_duration &clock,
                        const boost::posix_time::time_duration &c1,
                        const boost::posix_time::time_duration &c2,
                        float t1, float t2 )
{
   boost::posix_time::time_duration cDif = c2-c1;
   boost::posix_time::time_duration dif = clock-c1;
   float a = static_cast<float>(dif.hours())/static_cast<float>(cDif.hours());
   float tDelta = t2-t1;

   return t1 + tDelta*a;
}


void
NearestLand::
computeNearestLand( const NearestLandConfElement &conf,
                    const LocationPointMatrix &modeltopo,
                    const LocationPointMatrixTimeserie::XYPoints &xyPoints,
                    int suroundLevel )
{
   WEBFW_USE_LOGGER( "nearest_land" /*"handler"*/ );
   ParamDefPtr itParam;
   string provider;
   boost::posix_time::ptime dataRefTime;
   bool disabled;
   int dataversion;
   std::map< std::string, std::string> NearestLandParams = conf.params();

   for( std::map< std::string, std::string>::iterator itNearestLandParams=NearestLandParams.begin();
         itNearestLandParams != NearestLandParams.end();
         ++itNearestLandParams ) {
      if( ! params.findParam( itParam, itNearestLandParams->first, itNearestLandParams->second ) ) {
         WEBFW_LOG_WARN( "Nearest Land: No parameter definition for '" << itNearestLandParams->first << ", provider '"
                         << itNearestLandParams->second << "'.");
         continue;
      }

      if( ! refTimes->providerReftimeDisabledAndDataversion( itNearestLandParams->second,
                                                             dataRefTime,
                                                             disabled,
                                                             dataversion
                                                             ) ) {
         WEBFW_LOG_WARN( "Nearest land: No reference times found for provider '" << itNearestLandParams->second << "'. Check that the provider is listed in provider_priority.");
         continue;
      }

      if( disabled ) {
         WEBFW_LOG_WARN( "Nearest land: provider '" << itNearestLandParams->second << "' disabled.");
         continue;
      }

      try{
         LocationPointMatrixData dataTransactor( locationPoints.begin()->latitude(),
                                                 locationPoints.begin()->longitude(),
                                                 params,
                                                 *itParam,
                                                 itNearestLandParams->second,
                                                 dataRefTime,
                                                 dataversion,
                                                 suroundLevel,
                                                 wciProtocol,
                                                 "nearest_land" );

         wciConnection->perform( dataTransactor );

         if( dataTransactor.result()->size() == 0 ) {
            WEBFW_LOG_ERROR( "Nearest land: No data  found for '" << itNearestLandParams->first
                             << "', provider '" << itNearestLandParams->second << "'.");
            continue;
         }

         if( conf.rename() )
            provider = conf.renameTo();
         else
            provider = itNearestLandParams->second;

         setData( *itParam,
                  itNearestLandParams->first,
                  provider,
                  modeltopo,
                  xyPoints,
                  suroundLevel,
                  *dataTransactor.result() );
      }
      catch ( const std::exception &ex ) {
         ostringstream msg;
         msg << "EXCEPTION: Nearest land: DB trouble for '" << itNearestLandParams->first
               << "', provider '" << itNearestLandParams->second << "'.";
         WEBFW_LOG_WARN( msg.str() );
         continue;
      }
   }

}

void
NearestLand::
processNearestLandPoint( )
{

   const int suroundLevels=3;
   ParamDefPtr itParam;
   ParamDef landmaskParam;
   ParamDef modelTopoParam;
   LocationPointMatrix modelTopoLocations( 6, 6 );
   LocationPointMatrix landMaskLocations( 6, 6 ) ;
   LocationPointMatrixTimeserie paramDataLocations;
   boost::posix_time::ptime modelTopoRefTime;
   boost::posix_time::ptime landMaskRefTime;
   LocationPointMatrixTimeserie::XYPoints landPoints;
   int ALP;

   if( nearestLands.empty() || locationPoints.empty() )
      return;

   landPoints.reserve( 4*suroundLevels*suroundLevels); //(2*suroundLevels)^2


   WEBFW_USE_LOGGER( "nearest_land" );

   for( NearestLandConf::const_iterator it=nearestLands.begin();
         it != nearestLands.end();
         ++it ) {
      if( it->second.landmaskProvider().empty() ) {
         WEBFW_LOG_WARN( "Nearest land: No landmask (LANDCOVER) provider defined in 'nearest_land-"<< it->first << "'.");
         continue;
      }

      if( ! params.findParam( itParam, "LANDCOVER", it->second.landmaskProvider() ) ) {
         WEBFW_LOG_WARN( "Nearest land: No parameter definition for 'LANDCOVER', provider '" << it->second.landmaskProvider() << "'.");
         continue;
      }

      landmaskParam = *itParam;

      if( ! params.findParam( itParam,  "MODEL.TOPOGRAPHY", it->second.modelTopoProvider() ) ) {
         WEBFW_LOG_WARN( "Nearest land: No parameter definition for 'MODEL.TOPOGRAPHY', provider '" << it->second.modelTopoProvider() << "'.");
         continue;
      }

      modelTopoParam = *itParam;

      if( ! refTimes->providerReftime( it->second.modelTopoProvider(), modelTopoRefTime ) ) {
         WEBFW_LOG_INFO( "Nearest land: No reference times found for 'MODEL.TOPOGRAPHY', provider '" << it->second.modelTopoProvider() << "'. This is NOT unususal.");
         modelTopoRefTime = boost::posix_time::ptime();
      }

      if( ! refTimes->providerReftime( it->second.landmaskProvider(), landMaskRefTime ) ) {
         WEBFW_LOG_INFO( "Nearest land: No reference times found for LANDCOVER, provider '" << it->second.landmaskProvider() << "'. This is NOT unususal.");
         landMaskRefTime = boost::posix_time::ptime();
      }

      try{
         LocationPointMatrixData modelTopoTransactor( locationPoints.begin()->latitude(),
                                                      locationPoints.begin()->longitude(),
                                                      params,
                                                      modelTopoParam,
                                                      it->second.modelTopoProvider(),
                                                      modelTopoRefTime,
                                                      -1,
                                                      suroundLevels,
                                                      wciProtocol,
                                                      "nearest_land");

         wciConnection->perform( modelTopoTransactor );

         if( modelTopoTransactor.result()->size() == 0 ) {
            WEBFW_LOG_ERROR( "Nearest land: No data found for MODEL.TOPOGRAPHY, provider '" << it->second.modelTopoProvider() << "'.\n"
                             << modelTopoTransactor.query() << "\nParamDef: " << modelTopoParam );
            continue;
         }

         modelTopoLocations = modelTopoTransactor.result()->beginToTime()->second.second;  //It should be only one field for topography

         LocationPointMatrixData landMaskTransactor( locationPoints.begin()->latitude(),
                                                     locationPoints.begin()->longitude(),
                                                     params,
                                                     landmaskParam,
                                                     it->second.landmaskProvider(),
                                                     landMaskRefTime,
                                                     -1,
                                                     suroundLevels,
                                                     wciProtocol,
                                                     "nearest_land" );

         wciConnection->perform( landMaskTransactor );

         if( landMaskTransactor.result()->size() == 0 ) {
            WEBFW_LOG_ERROR( "Nearest land: No data  found for LANDCOVER, provider '" << it->second.landmaskProvider() << "'.");
            continue;
         }

         landMaskLocations = landMaskTransactor.result()->beginToTime()->second.second;  //It should be only one field for topography
         ALP = LocationPointMatrixTimeserie::valuesGreaterThan( landMaskLocations, 1, 0.9, landPoints );


         WEBFW_LOG_DEBUG("NearestLand: ALP1: " << ALP );

         if( ALP >= 4 ) {
            continue;
         }

         if( ALP > 0 ) {
            computeNearestLand( it->second, modelTopoLocations, landPoints, 1 );
            continue;
         }

         ALP = LocationPointMatrixTimeserie::valuesGreaterThan( landMaskLocations, 2, 0.9, landPoints );

         if( ALP > 0 ) {
            computeNearestLand( it->second, modelTopoLocations, landPoints, 2 );
            continue;
         }

         ALP = LocationPointMatrixTimeserie::valuesGreaterThan( landMaskLocations, 3, 0.9, landPoints );

         if( ALP > 0 ) {
            computeNearestLand( it->second, modelTopoLocations, landPoints, 3 );
            continue;
         }
      }
      catch( const exception &ex ) {
         WEBFW_LOG_WARN( "Nearest land: EXCEPTION: " << ex.what()  );
         continue;
      }
      catch( ... ) {
         WEBFW_LOG_WARN( "Nearest land: UNKNOWN EXCEPTION!"  );
         continue;
      }
   }
}

}
