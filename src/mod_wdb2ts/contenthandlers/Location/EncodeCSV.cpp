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

/**
 * @addtogroup wdb2ts 
 * @{
 * @addtogroup mod_wdb2ts 
 * @{
 */
/** @file
 * Implementation of the EncodeCSV class
 */
#ifdef HAVE_CONFIG_H
#include <wdb2ts_config.h>
#endif
// CLASS
#include <contenthandlers/Location/EncodeCSV.h>

// PROJECT INCLUDES
#include <ptimeutil.h>
#include <transactor/WciRead.h>
#include <Logger4cpp.h>


// SYSTEM INCLUDES
//

using namespace std;

namespace {
	
	class ReadHelper : public wdb2ts::WciReadHelper 
	{
		ostringstream &out_;
		string query_;
		float lat_;
		float lon_;
		int protocol_;
	public:
		ReadHelper( std::ostringstream *out, const std::string &query, float lat, float lon,
				      int protocol )
			: out_( *out ), query_( query ), lat_( lat ), lon_( lon ), protocol_( protocol )
			{
			}
		
		std::string id() { return "EncodeCVS"; }
		std::string query() { return query_; } 
		void clear() { out_.str(""); }
		
		void doRead( pqxx::result &result ) {
			string validtimefrom("validfrom");
			string validtimeto("validto");
			
			if( protocol_ > 2 ) {
				validtimefrom = "validtimefrom";
				validtimeto = "validtimeto";
			}
			
			for( pqxx::result::const_iterator it=result.begin(); it != result.end(); ++it  ) {
				out_ << it.at("referencetime").c_str() << ","
				     << it.at("dataprovidername").c_str() << ","
				     << it.at("placename").c_str() << ","
				     << lat_ << ","
				     << lon_ << ","
				     << it.at("confidencecode").c_str() << ","
				     << it.at("dataversion").c_str() << ","
				     << it.at( validtimefrom ).c_str() << ","
				     << it.at( validtimeto ).c_str() << ","
				     << "\"" << it.at("levelparametername").c_str() << "\"" << ","
				     << it.at("levelfrom").c_str() << ","
				     << it.at("levelto").c_str() << ","
				     << it.at("levelunitname").c_str() << ","
				     << "\"" << it.at("valueparametername").c_str() << "\"" << ","
				     << it.at("value").c_str() << ","
				     << it.at("valueparameterunit").c_str()
				     << "\n";
			}
		}
	};

}


namespace wdb2ts {

EncodeCSV::
EncodeCSV()
{
	// NOOP
}

EncodeCSV::
EncodeCSV( const WciWebQuery &webQuery, WciConnectionPtr connection, int protocol )
   : webQuery_( webQuery ), connection_( connection ), protocol_( protocol )
{
	// NOOP
}

EncodeCSV::
~EncodeCSV()
{
	// NOOP
}   
      

void  
EncodeCSV::
encode( std::ostream &out )
{
	WEBFW_USE_LOGGER( "encode" );
	ostringstream ost;
	string query;
	bool hasReftime=false;
	boost::posix_time::ptime from;
	boost::posix_time::ptime to;
	boost::posix_time::ptime nextTo;
	string indCode;
	
	if( ! webQuery_.reftime.fromTime.is_special() ) {
		from = webQuery_.reftime.fromTime;
		to = webQuery_.reftime.toTime;
		hasReftime = true;
	} else if( ! webQuery_.validtime.fromTime.is_special() ) {
		from = webQuery_.validtime.fromTime;
		to = webQuery_.validtime.toTime;
	} else {
		throw range_error("Invalid qurey, reftime or validtime must be different from 'NULL'.");
	}
	
	WEBFW_LOG_DEBUG("WciReadIterator: from:  " << from << " to: " << to );
	
	out << "#referencetime,dataprovidername,placename,latitude,longitude,confidencecode,dataversion,validfrom,validto,levelparametername,levelfrom,levelto,levelunitname,valueparametername,value,valueparameterunit"
         << "\n";

	nextTo = from + boost::gregorian::days( 1 );

	do {
		if( nextTo > to )
			nextTo = to;
	
		if( from == nextTo )
			indCode = "exact";
		else
			indCode = "inside";
		
		ost.str("");
		ost << miutil::isotimeString( from, true, true ) << "," 
		    <<  miutil::isotimeString( nextTo, true, true ) << "," << indCode;
		
		from = nextTo + boost::posix_time::seconds( 1 );
		nextTo = nextTo + boost::gregorian::days( 1 );

		if( hasReftime )
			webQuery_.reftime.decode( ost.str() );
		else
			webQuery_.validtime.decode( ost.str() );
		
		query = webQuery_.wciReadQuery() +  " order by validto;";
		
		try {
			ost.str("");
			ReadHelper readHelper( &ost, query, 
					                 webQuery_.latitude.value(), webQuery_.longitude.value(),
					                 protocol_ );
			WciRead transactor( &readHelper );
			connection_->perform( transactor, 2 );
			
			out << ost.str();
		}
		catch( const std::ios_base::failure &ex ) {
			throw;
		}
		catch( const std::runtime_error &ex ) {
			throw logic_error( ex.what() );
		}
		catch( const std::logic_error &ex ) {
			throw;
		}
		catch( ... ) {
			throw logic_error( "Unknown error while encoding CSV." );
		}
	} while( from <= to );

	
}

/**
 * @}
 * 
 * @}
 */

} 
