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

#include <pqxx/transactor>
#include <stlContainerUtil.h>
#include <transactor/WciRead.h>
#include <WdbDataRequestCommand.h>
#include <Logger4cpp.h>
#include <PqTupleContainer.h>
#include <sstream>

namespace {

class ReadHelper : public wdb2ts::WciReadHelper
{
	wdb2ts::LocationPointData &data;
	std::string query_;
	const wdb2ts::ParamDefList &paramDefs;
	const wdb2ts::ProviderList  &providerPriority;
	wdb2ts::PtrProviderRefTimes refTimes;
	const int wciProtocol;
	bool isPolygon;
	std::string id_;

public:
	ReadHelper( wdb2ts::LocationPointData &data_,
			    const std::string &query,
				const wdb2ts::ParamDefList &paramDefs_,
				const wdb2ts::ProviderList  &providerPriority_,
				wdb2ts::PtrProviderRefTimes refTimes_,
				const int wciProtocol_,
				bool isPolygon_ )
		: data( data_ ),
		  query_( query ),
		  paramDefs( paramDefs_ ),
		  providerPriority( providerPriority_ ),
		  refTimes( refTimes_ ),
		  wciProtocol( wciProtocol_ ),
		  isPolygon( isPolygon_ )
		{
			std::ostringstream ost;
			ost << "tid: " << boost::this_thread::get_id() << "";
			ost << ". WdbDataRequestCommand";
			id_ = ost.str();
		}

	std::string id() { return id_; }
	std::string query() { return query_; }
	void clear() { data.clear(); }

	void doRead( pqxx::result &result ) {
		//miutil::container::PqContainer container( result );
		decodePData( paramDefs, providerPriority, *refTimes, result, isPolygon, data, wciProtocol );
	}
};

}


namespace wdb2ts {


void
WdbDataRequestCommand::
operator()()
{
	//Register the requesthandler in this thread.
	WEBFW_USE_LOGGER_REQUESTHANDLER( "wdb", reqHandler );
	log4cpp::Priority::Value loglevel = WEBFW_GET_LOGLEVEL();
	std::ostringstream tidOst;

	tidOst << boost::this_thread::get_id();
	myTid = tidOst.str();

	try {
		ReadHelper	readHelper( *data_, query_, *paramDefs, *providerPriority,
								refTimes, wciProtocol, isPolygon );

		WciRead transactor( &readHelper );

		if( connection_ )
			connection_->perform( transactor, 2 );

		miutil::MetricTimer validateTimer( *validateMetric_);
		validate();
		validateTimer.stop();
		dbMetric_->addToTimer(transactor.dbMetric->getTimerSum());
		decodeMetric_->addToTimer(transactor.doReadMetric->getTimerSum());
	}
	catch( const std::exception &ex ) {
		*ok_ = false;
		*errMsg_ = ex.what();
	}
	catch( ... ) {
		*ok_ = false;
		*errMsg_ = "Unknown exception";
	}

	connection_.reset(); //Release the connection
};


/**
 * The function removes data if the forecast data is shorter than the
 * prognosis length.
 */
void
WdbDataRequestCommand::
validatePrognosisLength()
{
	using namespace boost;
	using namespace boost::posix_time;

	if( minPrognosisEndTime_.is_special() )
		return;

	WEBFW_USE_LOGGER_REQUESTHANDLER( "wdb", reqHandler );
	ptime constFieldTime( gregorian::date(1970, 1, 1), time_duration( 0, 0, 0 ) );
	//ptime progLength = now + seconds( prognosisLengthSeconds_ );

	LocationPointData::iterator itLocation=data_->begin();

	while( itLocation != data_->end() ) {
		if( ! itLocation->second || itLocation->second->empty() ) {
			itLocation = miutil::eraseElement( *data_, itLocation );
			WEBFW_LOG_INFO("tid: " << myTid << ". Data removed, less data than the minimum prognosis  length " << minPrognosisEndTime_ << ".");
			continue;
		}

//		if( logLevel >= log4cpp::Priority::DEBUG ) {
//			std::ostringstream ost;
//			printTimeSerie( ost, itLocation->second->begin(), itLocation->second->end(), 3 );
//			WEBFW_LOG_DEBUG("tid: " << myTid <<".\n" << ost.str());
//		}

		//The TimeSerie collection is sorted by to_time. With the start of
		//the forecast first.
		//We use a reverse iterator, ie we start at the end of the forecast.
		//If there is only constant fields, this is now at the start of the
		//reverse iterator.
		TimeSerie::reverse_iterator ritTime = itLocation->second->rbegin();

		//Ignore constant fields, ie fields that do not vary i time.
		if( ritTime->first == constFieldTime )
			continue;

		if( ritTime->first < minPrognosisEndTime_ ) {
			if( logLevel >= log4cpp::Priority::DEBUG ) {
				std::ostringstream ost;
				printTimeSerie( ost, itLocation->second->begin(), itLocation->second->end(), 3 );
				WEBFW_LOG_DEBUG("tid: " << myTid << ". Data removed, less data than the minimum prognosis  length " << minPrognosisEndTime_ << ".\n"
						        << ost.str());
			}

			itLocation = miutil::eraseElement( *data_, itLocation );
			WEBFW_LOG_INFO("tid: " << myTid << ". Data removed, less data than the minimum prognosis  length " << minPrognosisEndTime_ << ".");
			continue;
		}

		++itLocation;
	}
}

void
WdbDataRequestCommand::
validate()
{
	validatePrognosisLength();
	removeEmptyData( *data_ );
}

}
