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


#include <iostream>
#include <sstream>
#include <string>
#include <stlContainerUtil.h>
#include <transactor/ProviderRefTime.h>
#include <UpdateProviderReftimes.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <ptimeutil.h>
#include <UpdateProviderReftimes.h>
#include <wdb2TsApp.h>
#include <Logger4cpp.h>

using namespace std;
using namespace boost::posix_time; //ptime, second_clock
using namespace miutil; //isotimeString, ptimeFromIsoString 

namespace {
bool getProviderReftimes( wdb2ts::WciConnectionPtr wciConnection,
                          wdb2ts::ProviderRefTimeList &refTimes,
                          const string &provider,
                          const string &reftimespec,
						  int dataversion);

}


namespace wdb2ts {


bool
updateProviderRefTimes( WciConnectionPtr wciConnection, 
                        const ProviderRefTimeList &requestedUpdates,
                        ProviderRefTimeList &refTimes,
                        int wciProtocol  )
{
   WEBFW_USE_LOGGER( "handler" );
   ProviderRefTimeList resRefTimes;
   ProviderRefTimeList tmpRefTimes( refTimes );
   ptime now( second_clock::universal_time() );
   ptime back(now);
   ptime endTime( now );
   string reftime;
   ostringstream ost;
   string provider;
   bool proccessed;
   bool changed=false;
   ProviderRefTimeList::const_iterator itReftime;

   back -= hours( 8640 ); //8640 hours back from now
   endTime += hours( 24 ); //1 day in the future.

   for( ProviderRefTimeList::const_iterator it = requestedUpdates.begin();
         it != requestedUpdates.end();
         ++it ) {
      ost.str("");

      provider = it->first;

      if( ! it->second.reftimeUpdateRequest ) {
         //It is not requested to update the reference time.
         //Only dataversion and/or disable status should be updated.
    	  bool changeInThis=false;
         if( it->second.disableEnableRequest )
            if( tmpRefTimes.updateDisableStatus( it->first, it->second.disabled)>0 ) {
               changed = true;
               changeInThis=true;
            }

         if( it->second.dataversionRequest )
            if( tmpRefTimes.updateDataversion( it->first, it->second.dataversion )>0 ) {
               changed = true;
               changeInThis = true;
            }

         bool disabled;
         if( !changeInThis || (tmpRefTimes.disabled(it->first, disabled) && disabled) )
        	 continue;

      }

      ost.str("");

      if( it->second.refTime.is_special() ) {
      	ost << "NULL";
      } else {
         if( wciProtocol == 1 )
            ost << "('" << isotimeString( it->second.refTime, false, true ) << "','" << isotimeString( it->second.refTime, false, true ) << "','exact')";
         else
            ost << "'exact " << isotimeString( it->second.refTime, false, true ) << "'";
      }

      reftime = ost.str();
      resRefTimes.clear();

      if( ! getProviderReftimes( wciConnection, resRefTimes, provider, reftime, it->second.dataversion ) ) {
    	  //We do NOT have data for the reference time or dataversion.
    	  ostringstream serr;
    	  serr << "updateProviderRefTimes: Missing data for: " << provider << " at ";

    	  if(it->second.refTime.is_special() )  serr << "'latest'";
    	  else serr << "'" << isotimeString( it->second.refTime, false, true ) << "'";

    	  serr << " with dataversion ";

    	  if( it->second.dataversion < 0 ) serr  << "'latest'";
    	  else serr << it->second.dataversion;
    	  serr << ".";
    	  WEBFW_LOG_WARN(serr.str());
    	  continue;
    	  //throw std::logic_error( serr.str() );
      } else {
         ProviderItem pi=ProviderList::decodeItem( provider );

         //It should be only one result.
         for( ProviderRefTimeList::iterator rit = resRefTimes.begin();
               rit != resRefTimes.end();
               ++rit ) {
            proccessed = false;
            ProviderRefTimeList::iterator refTimeIt = tmpRefTimes.find( rit->first );

            //Do not update the reftime if we allready have a record for
            //this reftime. This is neccesary since the updatedTime is used
            //in the meta data returned, runEnded or something other that tells
            //when we started to serve data from this provider/reftime combination.
            if( refTimeIt != tmpRefTimes.end() ) {
               if( refTimeIt->second.refTime == rit->second.refTime ) {
                  if( pi.placename.empty() ||
                      refTimeIt->first == pi.providerWithPlacename() ) {
                     proccessed = true;

                     if( it->second.dataversionRequest &&
                           refTimeIt->second.dataversion != it->second.dataversion ) {
                        refTimeIt->second.dataversion = it->second.dataversion;
                        changed = true;
                     }

                     if( it->second.disableEnableRequest &&
                           refTimeIt->second.disabled != it->second.disabled) {
                        refTimeIt->second.disabled = it->second.disabled;
                        changed = true;
                     }
                  }
               } else {
                  if( pi.placename.empty() ||
                      rit->first == pi.providerWithPlacename() ) {

                     changed = true;
                     proccessed = true;
                     ProviderTimes old = refTimeIt->second;
                     refTimeIt->second = rit->second;
                     refTimeIt->second.disabled = old.disabled;

                     if( it->second.dataversionRequest )
                        refTimeIt->second.dataversion = it->second.dataversion;

                     if( it->second.disableEnableRequest )
                        refTimeIt->second.disabled = it->second.disabled;
                  }
               }
            }

            if( ! proccessed ) {
               //There is no entry defined in the incoming refTimes for this provider.
               //We just add it to the refTimes.
               rit->second.dataversion = it->second.dataversion;
               rit->second.disabled = it->second.disabled;
               tmpRefTimes[rit->first] = rit->second;
               changed = true;
            }
         }
      }
   }

   refTimes = tmpRefTimes;
   return changed;
}


bool
updateProviderRefTimes( WciConnectionPtr wciConnection, 
                        ProviderRefTimeList &refTimes,
                        const ProviderList &providers,
                        int wciProtocol  )
{
   ostringstream ost;
   ProviderRefTimeList tmpRefTimes;
   ptime now( second_clock::universal_time() );
   ptime back(now);
   ptime  endTime( now );
   string reftimeSpec;
   list<string> providerListWithoutplacename=providers.providerWithoutPlacename();

   back -= hours( 48 ); //48 hours back from now
   endTime += hours( 24 ); //1 day in the future.

   if( providers.empty() )
      return true;

   refTimes.clear();

   if( wciProtocol == 1 )
      ost << "('" << isotimeString( back, false, true ) << "','" << isotimeString( endTime, false, true ) << "','inside')";
   else
      ost << "'inside " << isotimeString( back, false, true ) << " TO " << isotimeString( endTime, false, true ) << "'";

   reftimeSpec = ost.str();

   for( list<string>::iterator it = providerListWithoutplacename.begin() ;
         it != providerListWithoutplacename.end();
         ++ it ){
      tmpRefTimes.clear();

      //Search in the period [back, endTime]
      if( getProviderReftimes( wciConnection, tmpRefTimes, *it, reftimeSpec, -1 ) ) {
         for( ProviderRefTimeList::iterator pit =tmpRefTimes.begin();
              pit != tmpRefTimes.end();
              ++pit ) {
            refTimes[pit->first] = pit->second;
         }
         continue;
      }

      //Try to search the entire database.
      if( getProviderReftimes( wciConnection, tmpRefTimes, *it, "NULL", -1 ) ) {
         for( ProviderRefTimeList::iterator pit =tmpRefTimes.begin();
               pit != tmpRefTimes.end();
               ++pit )
            refTimes[pit->first] = pit->second;
      }
   }

   return ! refTimes.empty();
}



namespace {
std::string updateStatus( const ProviderRefTimeList &oldRefTime,
			     	 	  const ProviderRefTimeList &newRefTime )
{
	WEBFW_USE_LOGGER( "handler" );
	ProviderRefTimeList::const_iterator itOld;
	ProviderRefTimeList::const_iterator itNew;

	std::ostringstream logMsg;
	logMsg << "LocationForecastUpdateHandler: refTimes. " << endl;
	for( ProviderRefTimeList::const_iterator it = newRefTime.begin();
	     it != newRefTime.end();
		  ++it )
		logMsg << "  " << it->first << ": " << it->second.refTime
		       << ": " << it->second.dataversion << endl;;

	WEBFW_LOG_INFO( logMsg.str() );


	for( itOld = oldRefTime.begin(); itOld != oldRefTime.end(); ++itOld ) {
		itNew = newRefTime.find( itOld->first );

		if( itNew == newRefTime.end() ||
			 itOld->second.refTime != itNew->second.refTime ||
			 itOld->second.dataversion != itNew->second.dataversion ||
			 itOld->second.disabled != itNew->second.disabled )
			return "Updated";
	}

	//Has any new providers been added.
	for( itNew = newRefTime.begin(); itNew != newRefTime.end(); ++itNew ) {
		itOld = oldRefTime.find( itNew->first );

		if( itOld == oldRefTime.end() )
			return "Updated";
	}

	return "NoNewDataRefTime";
}

ProviderRefTimeList
checkProviders( const ProviderList &providerList,
		        const ProviderRefTimeList &oldRefTime,
		        const ProviderRefTimeList &requestedUpdate )
{
	WEBFW_USE_LOGGER( "handler" );
	list<string> providers=providerList.providerWithoutPlacename();
	list<string>::const_iterator pit;
	ProviderRefTimeList::const_iterator itOldRefTime;
	bool disabled;
	int dataversion;
	ProviderRefTimeList retRequestedUpdate;

	//Check that the requested providers i defined in the provider_prioority.
	for( ProviderRefTimeList::const_iterator it = requestedUpdate.begin();
		 it != requestedUpdate.end(); ++it ) {
		ProviderList::const_iterator hasProvider = providerList.findProviderWithoutPlacename( it->first );

		if( hasProvider == providerList.end() ) {
			WEBFW_LOG_WARN( "WARNING: LocationUpdateHandler: It is requested an update for an provider '"
					<< it->first << "' that is not in the provider_priority list.");
		} else {
			retRequestedUpdate[it->first]=it->second;
		}
	}

	if( requestedUpdate.empty() ) {
		for ( pit = providers.begin();
		      pit != providers.end();
		      ++ pit ) {
			retRequestedUpdate[*pit] = ProviderTimes();

			if( oldRefTime.disabled( *pit, disabled ) )
				retRequestedUpdate[*pit].disabled = disabled;

			if( oldRefTime.dataversion( *pit, dataversion ) )
				retRequestedUpdate[*pit].dataversion = dataversion;

			retRequestedUpdate[*pit].reftimeUpdateRequest = true;
		}
	}

	return retRequestedUpdate;
}

void checkAgainstExistingProviders( const ProviderRefTimeList &exitingProviders,
		                         ProviderRefTimeList &newRefTime )
{
	ProviderRefTimeList::iterator newIt = newRefTime.begin();
	ProviderRefTimeList::const_iterator eit;

	while(  newIt != newRefTime.end() ) {
		eit = exitingProviders.find( newIt->first );

		//If the provider do not exist, delete it.
		newIt = miutil::eraseElementIf( newRefTime, newIt, eit == exitingProviders.end() );
	}
}

}

PtrProviderRefTimes
locationForecastUpdate(
		WciConnectionPtr wciConnection,
		const ProviderRefTimeList &requestedProviders_,
		const ProviderList &providerPriorityList,
		const PtrProviderRefTimes oldRefTime,
		int wciProtocol,
		std::string &status
)
{
	WEBFW_USE_LOGGER("handler");
	ProviderRefTimeList requestedProviders;
	bool debug;

	ProviderRefTimeList exitingProviders;
	PtrProviderRefTimes newRefTime(new ProviderRefTimeList(*oldRefTime));

	requestedProviders = checkProviders(providerPriorityList, *oldRefTime,
			requestedProviders_);

	//Get a list of the providers with placename that is exiting in the database.
	//It my happen that we don't have data for some models, ie. they are deleted.
	if (updateProviderRefTimes(wciConnection, exitingProviders,
			providerPriorityList, wciProtocol)) {
		updateProviderRefTimes(wciConnection, requestedProviders, *newRefTime,
				wciProtocol);

		checkAgainstExistingProviders(exitingProviders, *newRefTime);
		//app->notes.setNote( updateid + ".LocationProviderReftimeList", new NoteProviderReftimes( newRefTime ) );
	}

	status = updateStatus(*oldRefTime, *newRefTime);
	return newRefTime;
}

PtrProviderRefTimesByDbId
locationForecastUpdateAllDbIds(
		wdb2ts::Wdb2TsApp *app,
		const std::list<std::string> &dbIds,
		const ProviderRefTimeList &requestedProviders_,
		const ProviderList &providerPriorityList,
		const PtrProviderRefTimesByDbId oldRefTime_,
		int wciProtocol,
		std::map<std::string,std::string> &statuses
)

{
	using namespace std;
	WEBFW_USE_LOGGER("handler");

	PtrProviderRefTimesByDbId result(new ProviderRefTimesByDbId());
	PtrProviderRefTimes oldRefTime;
	string status;

	statuses.clear();

	for(list<string>::const_iterator itDbId=dbIds.begin(); itDbId!=dbIds.end(); ++itDbId) {
		WciConnectionPtr con=app->newWciConnection(*itDbId);
		ProviderRefTimesByDbId::const_iterator itOldReftime= oldRefTime_->find(*itDbId);

		if( itOldReftime == oldRefTime_->end())
			oldRefTime.reset( new ProviderRefTimeList());
		else
			oldRefTime=itOldReftime->second;

		(*result)[*itDbId]=locationForecastUpdate(con,requestedProviders_,
				providerPriorityList, oldRefTime, wciProtocol, status);
		statuses[*itDbId]=status;
	}

	return result;
}


}


namespace {
bool 
getProviderReftimes( wdb2ts::WciConnectionPtr wciConnection,
                     wdb2ts::ProviderRefTimeList &refTimes,
                     const string &provider,
                     const string &reftimespec,
					 int dataversion )
{

   wdb2ts::ProviderItem pi=wdb2ts::ProviderList::decodeItem( provider );
   wdb2ts::ProviderRefTime providerReftimeTransactor( refTimes, pi.provider, reftimespec, dataversion );
   wdb2ts::PtrProviderRefTimes result;


   try {
      wciConnection->perform( providerReftimeTransactor, 3 );
      result = providerReftimeTransactor.result();

      refTimes = *result;
   }
   catch( const std::ios_base::failure &ex ) {
      throw logic_error( ex.what() );
   }
   catch( const std::runtime_error &ex ) {
      throw logic_error( ex.what() );
   }
   catch( const std::logic_error &ex ) {
      throw;
   }
   catch( ... ) {
      throw logic_error( "Unknown error while checking for reference times." );
   }

   return ! refTimes.empty();
}






}
