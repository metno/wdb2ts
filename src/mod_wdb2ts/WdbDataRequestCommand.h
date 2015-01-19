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

#ifndef __WDBDATAREQUESTCOMMAND_H__
#define __WDBDATAREQUESTCOMMAND_H__

#include <string>
#include <boost/shared_ptr.hpp>
#include <RequestHandler.h>
#include <PointDataHelper.h>
#include <DbManager.h>
#include <UpdateProviderReftimes.h>
#include <ParamDef.h>



namespace wdb2ts {

/**
 * A class to hold what is needed to run a wdb request on a thread.
 */
class WdbDataRequestCommand {
	LocationPointDataPtr data_;
	WciConnectionPtr connection_;
	const webfw::RequestHandler *reqHandler;
	std::string query_;
	ParamDefListPtr paramDefs;
	ProviderListPtr  providerPriority;
	PtrProviderRefTimes refTimes;
	const int wciProtocol;
	bool isPolygon;
	boost::shared_ptr<bool> ok_;
	boost::shared_ptr<std::string> errMsg_;
	int prognosisLengthSeconds_;
public:

	WdbDataRequestCommand( WciConnectionPtr connection,
						   const webfw::RequestHandler *reqHandler_, //To use for logging
					       const std::string &query,
			               ParamDefListPtr paramDefs_,
			               ProviderListPtr  providerPriority_,
			               PtrProviderRefTimes refTimes_,
			               const int wciProtocol_,
			               bool isPolygon_,
						   int prognosisLengthSeconds)
		: data_( new LocationPointData() ),
		  connection_( connection ),
		  reqHandler( reqHandler_ ),
		  query_( query ), paramDefs( paramDefs_ ),
		  providerPriority( providerPriority_ ),
		  refTimes( refTimes_ ),
		  wciProtocol( wciProtocol_ ),
		  isPolygon( isPolygon_ ),
		  ok_( new bool(true) ),
		  errMsg_( new std::string() ),
		  prognosisLengthSeconds_( prognosisLengthSeconds )
		  {
		  }

	std::string query()const { return query_;}
	void setConnection( WciConnectionPtr connection ) { connection_ = connection; }
	LocationPointData& data(){ return *data_;}
	bool ok()const { return *ok_;}
	std::string errMsg()const { return *errMsg_;}

	void validatePrognosisLength();
	void validate();
	void operator()();

};

typedef boost::shared_ptr<WdbDataRequestCommand> WdbDataRequestCommandPtr;

}

#endif /* WDBDATAREQUESTCOMMAND_H_ */
