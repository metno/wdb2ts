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
#include <splitstr.h>
#include <trimstr.h>
#include <WciWebQuery.h>
#include <UrlQuery.h>

using namespace std;

namespace {
    
struct KeyDecode {
    char *key;
    wdb2ts::UrlParam *decoder;
};

}

namespace wdb2ts {

WciWebQuery::
WciWebQuery( const std::string &returnCol )
    : returnColoumns( returnCol ),
   	dataprovider( 1 ),
 	   latitude( 1 ),
 	   longitude( 1 ),
 	   pointInterpolation( 1, "nearest", false ),
 	   reftime( 1 ),
 	   validtime( 1 ),
 	   parameter( 1 ),
 	   levelspec( 1 ),
 		dataversion( 1, true, -1 ),
 		format( 1, UrlParamFormat::CSV ),
 		altitude( 1 )
{
    // NOOP
}

WciWebQuery::
WciWebQuery( int protocol, const std::string &returnCol )
    : returnColoumns( returnCol ),
   	dataprovider( protocol ),
	   latitude( protocol ),
	   longitude( protocol ),
	   pointInterpolation( protocol, "nearest", false ),
	   reftime( protocol ),
	   validtime( protocol ),
	   parameter( protocol ),
	   levelspec( protocol ),
		dataversion( protocol, true, -1 ),
		format( protocol, UrlParamFormat::CSV ),
		altitude( protocol )
{
    // NOOP
}

/*
WciWebQuery::
WciWebQuery( const WciWebQuery &query )
{
    // NOOP
}
*/    
WciWebQuery::
~WciWebQuery()
{
    // NOOP
}
   
/**
 * Decodes a query on the form:
 * 
 * http://server/path/?lat=10;lon=10;alt=10;
 *   reftime=2007-12-10T10:00,2007-12-10T10:00,exact;
 *   dataprovider=1096;
 *   dataversion=-1;
 *   parameter=instantaneous pressure of air,instantaneous temperature of air,instantaneous velocity of air (u-component);
 *   levelspec=2,2,above ground,exact;
 *   validtime=2007-12-10T00:00,2007-12-10T10:00,intersect
 *   format=CSV
 *
 * @exception logic_error
 */
std::string
WciWebQuery::
decode( const std::string &query )
{
    ostringstream ost;
    string        currentKey;
    list<string>  keys; 
    string        val;
    int           decoderIndex;
    webfw::UrlQuery      urlQuery;
    
    KeyDecode decoders[] = { 
	{"lat",           &latitude},
	{"long",          &longitude}, //Accept also 'long' as a longitude. 
	{"lon",           &longitude},
	{"point_interpolation", &pointInterpolation},
	{"alt",           &altitude},
	{"reftime",       &reftime},
	{"validtime",     &validtime},
	{"parameter",     &parameter},
	{"dataprovider",  &dataprovider},
	{"dataversion",   &dataversion},
	{"levelspec",     &levelspec},
	{"format",        &format},
	{0, 0}
    };

    
   //Reset all values to the default values. 
 	for( decoderIndex=0; decoders[decoderIndex].key ; ++decoderIndex )
 		decoders[decoderIndex].decoder->clean();
 		
   try{
   	urlQuery.decode( query );
   }
   catch( const std::exception &ex ) {
   	//Rewamp the exceptions.
   	throw std::logic_error( ex.what() );
   }
 	
   wciReadQuery_.erase();
   keys = urlQuery.keys();
    
   for( list<string>::const_iterator itKey = keys.begin(); itKey != keys.end(); ++itKey ) {
   	if( itKey->empty() )
   		continue;
   	
   	for( decoderIndex=0; 
   	     decoders[decoderIndex].key && decoders[decoderIndex].key != *itKey; 
	        ++decoderIndex );
       
   	if( ! decoders[decoderIndex].key  ) {
   		ost << "Unknown key: " << *itKey << ".";
   		throw logic_error( ost.str() );
   	}
        
   	try {
   		val = urlQuery.asString( *itKey, "");
   		
   		if( val.empty() )
   			continue;
   		
   		decoders[decoderIndex].decoder->decode( val );
   	}
   	catch( const logic_error &ex ){
   		ost << "Invalid value for <" << *itKey << ">. " << ex.what();
   		throw logic_error( ost.str() );
   	}
   }
   
   return wciReadQuery();
}
    
std::string 
WciWebQuery::
wciReadQuery() const
{
   ostringstream        ost;
   
   if( ! latitude.valid() || ! longitude.valid() )
       throw logic_error("Missing 'lat' or 'lon/long'.");
   
   string sPointInterpolation=pointInterpolation.selectPart();
      
   /* If the pointInterpolation string is NULL, erase it.
    * If the pointInterpolation string is different from
    * null it will start and end with the SQL quota character ',
    * they must be removed. 
    */
   
   if( sPointInterpolation=="NULL" )
   	sPointInterpolation.erase();
   else
   	miutil::trimstr( sPointInterpolation, miutil::TRIMBOTH, "'" );
      
   ost << "SELECT " << returnColoumns << " FROM wci.read(" << endl  
       << "   " << dataprovider.selectPart() << ", " << endl
       << "   '" << sPointInterpolation << " POINT(" <<  longitude.value() << " " << latitude.value() << ")', " << endl
//       << "   'POINT(" <<  longitude.value() << " " << latitude.value() << ")', " << endl
       << "   " << reftime.selectPart() << ", " << endl
       << "   " << validtime.selectPart() << ", " << endl
       << "   " << parameter.selectPart() << ", " << endl
       << "   " << levelspec.selectPart() << ", " << endl
       << "   " << dataversion.selectPart() << ", " << endl
       << "   NULL::wci.returnfloat )" << endl;
   
   return ost.str();
}
    

} // namespace
