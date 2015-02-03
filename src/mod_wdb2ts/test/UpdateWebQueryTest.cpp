/*
    wdb - weather and water data storage

    Copyright (C) 2008 met.no

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


#include <UpdateWebQueryTest.h>
#include <contenthandlers/LocationForecast/LocationForecastUpdateHandler.h>
#include <UrlQuery.h>
#include <splitstr.h>


CPPUNIT_TEST_SUITE_REGISTRATION( UpdateWebQueryTest );

using namespace std;
using namespace wdb2ts;
namespace pt=boost::posix_time;


bool
oldDecodeQuery( const std::string &query, ProviderRefTimeList &newRefTime, bool &debug )
{
	using namespace miutil;
	using namespace boost::posix_time;

	WEBFW_USE_LOGGER( "handler" );

	std::list<std::string> keys;
	//vector<string> keys;
	vector<string> keyvals;
	string              buf;
	int                 dataversion;
	bool                disable;
	bool 	            doDisableEnable=false;
	bool              doDataversion=false;
	webfw::UrlQuery     urlQuery;

	debug=false;

	newRefTime.clear();

	if( query.empty() )
		return true;

	try {
		urlQuery.decode( query );
	}
	catch( const std::exception &ex ) {
		WEBFW_LOG_ERROR( "LocationUpdateHandler:Query: Invalid query '" + query +". Reason: " + ex.what() );
		return false;
	}

	keys = urlQuery.keys();

	if( std::count( keys.begin(), keys.end(), "debug" ) > 0 ) {
		debug = true;
		return true;
	}

	for( std::list<std::string>::const_iterator iKey=keys.begin(); iKey != keys.end(); ++iKey ) {
		string key = *iKey ;
		string val = urlQuery.asString( *iKey, "");
		string::size_type i;

		trimstr( key );
		trimstr( val );

		if( key.empty() ) {
			WEBFW_LOG_ERROR( "LocationUpdateHandler:Query: Invalid key. Empty key." );;
			return false;
		}

		if( val.empty() ) {
			WEBFW_LOG_ERROR( "LocationUpdateHandler:Query: Key: '" << key << "'. Empty value." );;
			return false;
		}

		WEBFW_LOG_DEBUG( "LocationUpdateHandler:Query: Key: '" << key << "' value: '" << val );
		ProviderItem pi=ProviderList::decodeItem( key );


		//The val can be on the form YYYY-MM-DDThh:mm:ss,dataversion
		//Where dataversion is optional.
		//A dataversion is used only if the reftime is given.
		keyvals = splitstr( val, ',' );
		dataversion = -1;
		disable = false;
		doDataversion = false;
		doDisableEnable = false;

		if( keyvals.size() > 1 ) {
			val = keyvals[0];
			buf = keyvals[1];

			if( sscanf( buf.c_str(), "%d", &dataversion ) != 1 )
				dataversion = -1;
			else
			   doDataversion = true;
		}

		if( val.empty() ) {
			newRefTime[pi.providerWithPlacename()] = ProviderTimes();
			if( doDataversion ) {
			   newRefTime[pi.providerWithPlacename()].dataversion = dataversion;
			   newRefTime[pi.providerWithPlacename()].dataversionRequest = true;
			}
			continue;
		}


		i = val.find_first_not_of( "0123456789" );


		if( i == string::npos ) {
			int n;

			if( sscanf( val.c_str(), "%d", &n ) != 1 ) {
				WEBFW_LOG_ERROR( "LocationUpdateHandler:Query: Invalid Value. key <" << key << "> expecting a number." );
				return false;
			}

			if( n != 0 ) {
				WEBFW_LOG_ERROR( "LocationUpdateHandler:Query: Invalid Value. key <" << key << "> Not a valid value. Valid values [0]." );;
				return false;
			}

			newRefTime[pi.providerWithPlacename()] = ProviderTimes();
			newRefTime[pi.providerWithPlacename()].reftimeUpdateRequest = true;

			if( doDataversion ) {
			   newRefTime[pi.providerWithPlacename()].dataversion = dataversion;
			   newRefTime[pi.providerWithPlacename()].dataversionRequest = true;
			}

			continue;
		} if( val[0]=='d' || val[0]=='D' ||  val[0]=='e' || val[0]=='E' ) {
			doDisableEnable = true;

			if( val[0]=='d' || val[0]=='D' )
			   disable = true;
		}

		try{
			if( doDisableEnable ) {
			   newRefTime[pi.providerWithPlacename()].disableEnableRequest = true;
				newRefTime[pi.providerWithPlacename()].disabled = disable;
			} else {
				newRefTime[pi.providerWithPlacename()].refTime = ptimeFromIsoString( val );
				newRefTime[pi.providerWithPlacename()].reftimeUpdateRequest = true;
			}

			if( doDataversion ) {
			   newRefTime[pi.providerWithPlacename()].dataversion = dataversion;
			   newRefTime[pi.providerWithPlacename()].dataversionRequest = true;
			}
		}
		catch( logic_error &e ) {
			WEBFW_LOG_ERROR( "LocationUpdateHandler:Query: Invalid value. key <" << key << "> value <" << val << ">. Not a valid timespec." );;
			return false;
		}
	}

	std::ostringstream logMsg;
	logMsg << "LocationUpdateHandler:Query: Requested providers reftime!\n";
	for( ProviderRefTimeList::const_iterator it = newRefTime.begin(); it != newRefTime.end(); ++it )
		logMsg <<"     " << it->first << ": " << it->second.refTime
		       << " dataversion (" << (it->second.dataversionRequest?"t":"f") << "): " << it->second.dataversion
		       << " disabled(" << (it->second.disableEnableRequest?"t":"f") << "): " << (it->second.disabled?"true":"false")
		       <<  '\n';
	WEBFW_LOG_DEBUG(logMsg.str());

	return true;
}




UpdateWebQueryTest::
UpdateWebQueryTest()
{
	// NOOP
}

UpdateWebQueryTest::
~UpdateWebQueryTest()
{
	// NOOP
}

void
UpdateWebQueryTest::
setUp()
{
	// NOOP
}

void
UpdateWebQueryTest::
tearDown()
{
	// NOOP
}

bool
isReftimeUpdateRequest( const ProviderTimes &pt, const pt::ptime &reftime=pt::ptime(), int expectedDataversion=-1 )
{
	if( pt.reftimeUpdateRequest &&
			pt.refTime == reftime && !pt.disableEnableRequest ) {

		if( pt.dataversionRequest )
			return pt.dataversion == expectedDataversion;
		return true;
	} else {
		return false;
	}
}

bool
isDisableEnableRequest( const ProviderTimes &pt, bool expetedStatus )
{
	if( pt.disableEnableRequest && pt.disabled == expetedStatus
		&& !pt.reftimeUpdateRequest && !pt.dataversionRequest && pt.refTime.is_special()	)
		return true;
	else
		return false;
}



bool
isDataversionRequest( const ProviderTimes &pt, int expetedStatusDataversion, bool expectRefTime )
{
	if( pt.dataversionRequest && pt.dataversion == expetedStatusDataversion ){
		return pt.reftimeUpdateRequest == expectRefTime;
	} else {
		return false;
	}
}
ostream&
printProviderTimes( ostream &os, const ProviderTimes &pt )
{
	os << "reftimeUpdateRequest: " << (pt.reftimeUpdateRequest?"true ":"false") << "     refTime: " << pt.refTime << endl ;
	os << "disableEnableRequest: " << (pt.disableEnableRequest?"true ":"false") << "    disabled: " << (pt.disabled?"true":"false") << endl;
	os << "dataversionRequest:   " << (pt.dataversionRequest?"true ":"false")   << " dataversion: "<< pt.dataversion << endl ;
	return os;
}

pt::ptime
mkptime( const std::string &t )
{
	return miutil::ptimeFromIsoString( t );
}

void
UpdateWebQueryTest::
testReferenceTimeAndVersion()
{
	bool (*decodeQueryValue)(const std::string &provider, const std::string &val, ProviderRefTimeList &newRefTime);
	ProviderRefTimeList updateTimes;
	bool debug;

	decodeQueryValue = LocationForecastUpdateHandler::decodeQueryValue;
	CPPUNIT_ASSERT( decodeQueryValue("test", "2015-01-03T06:00:00", updateTimes) );
	CPPUNIT_ASSERT( isReftimeUpdateRequest( updateTimes["test"], mkptime("2015-01-03T06:00:00") ) );

	updateTimes.clear();
	CPPUNIT_ASSERT( decodeQueryValue("test", "2015-01-03T06:00:00,-1", updateTimes) );
	CPPUNIT_ASSERT( isReftimeUpdateRequest( updateTimes["test"], mkptime("2015-01-03T06:00:00"), -1 ) );

	updateTimes.clear();
	CPPUNIT_ASSERT( decodeQueryValue("test", "2015-01-03T06:00:00,3", updateTimes) );
	CPPUNIT_ASSERT( isReftimeUpdateRequest( updateTimes["test"], mkptime("2015-01-03T06:00:00"), 3 ) );

	updateTimes.clear();
	CPPUNIT_ASSERT( ! decodeQueryValue("test", "2015-01-03T06:00:00,", updateTimes) );

	updateTimes.clear();
	CPPUNIT_ASSERT( decodeQueryValue("test", "2015-01-03T06:00:00Z", updateTimes) );

	updateTimes.clear();
	CPPUNIT_ASSERT( !decodeQueryValue("test", "2015-01-03T06:00:00Z,a", updateTimes) );

	updateTimes.clear();
	CPPUNIT_ASSERT( !decodeQueryValue("test", "2015-01-03T06:00:0Z,a", updateTimes) );
}

void
UpdateWebQueryTest::
testVersionOnly()
{
	bool (*decodeQueryValue)(const std::string &provider, const std::string &val, ProviderRefTimeList &newRefTime);
	ProviderRefTimeList updateTimes;
	bool debug;
	decodeQueryValue = LocationForecastUpdateHandler::decodeQueryValue;

	CPPUNIT_ASSERT( decodeQueryValue("test", ",-1", updateTimes) );
	CPPUNIT_ASSERT( isDataversionRequest(updateTimes["test"], -1, false ) );

	updateTimes.clear();
	CPPUNIT_ASSERT( decodeQueryValue("test", "0,-1", updateTimes) );
	CPPUNIT_ASSERT( isDataversionRequest(updateTimes["test"], -1, true ) );

	updateTimes.clear();
	CPPUNIT_ASSERT( !decodeQueryValue("test", "0", updateTimes) );

	updateTimes.clear();
	CPPUNIT_ASSERT( !decodeQueryValue("test", "0,", updateTimes) );

	updateTimes.clear();
	CPPUNIT_ASSERT( !decodeQueryValue("test", ",", updateTimes) );

	updateTimes.clear();
	CPPUNIT_ASSERT( !decodeQueryValue("test", "1,-1", updateTimes) );
}

void
UpdateWebQueryTest::
testEnableDisable()
{
	bool (*decodeQueryValue)(const std::string &provider, const std::string &val, ProviderRefTimeList &newRefTime);
	ProviderRefTimeList updateTimes;
	bool debug;
	decodeQueryValue = LocationForecastUpdateHandler::decodeQueryValue;

	CPPUNIT_ASSERT( decodeQueryValue("test", "disable", updateTimes) );
	printProviderTimes( cerr, updateTimes["test"] );
	CPPUNIT_ASSERT( isDisableEnableRequest( updateTimes["test"], true ) );

	updateTimes.clear();
	CPPUNIT_ASSERT( decodeQueryValue("test", "enable", updateTimes) );
	CPPUNIT_ASSERT( isDisableEnableRequest( updateTimes["test"], false ));

	updateTimes.clear();
	CPPUNIT_ASSERT( ! decodeQueryValue("test", "e", updateTimes) );

	updateTimes.clear();
	CPPUNIT_ASSERT( decodeQueryValue("test", "eN", updateTimes) );
	CPPUNIT_ASSERT( isDisableEnableRequest( updateTimes["test"], false ));
}
