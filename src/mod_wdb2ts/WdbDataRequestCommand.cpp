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
#include <transactor/WciRead.h>
#include <WdbDataRequestCommand.h>
#include <Logger4cpp.h>

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
		}

	std::string id() { return "WdbDataRequestCommand (wdb query)"; }
	std::string query() { return query_; }
	void clear() { data.clear(); }

	void doRead( pqxx::result &result ) {
		decodePData( paramDefs, providerPriority, *refTimes, wciProtocol, result, isPolygon, data );
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
	try {
		ReadHelper	readHelper( *data_, query_, paramDefs, providerPriority,
								refTimes, wciProtocol, isPolygon );

		WciRead transactor( &readHelper );

		if( connection_ )
			connection_->perform( transactor, 2 );
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



}
