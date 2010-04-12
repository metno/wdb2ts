#include <stdlib.h>
#include <GetThread.h>
#include <HTTPClient.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <boost/crc.hpp>      // for boost::crc_basic, boost::crc_optimal
#include <boost/cstdint.hpp>  // for boost::uint16_t


using namespace std;

namespace wdb2ts {



void 
GetThread::
nextLatLong( float &lat_, float &lon_ )
{
	const int LAT = 18;
	const int LON = 30;
	
	if( lat != FLT_MAX )
		 lat_ = lat;
	else 
		lat_ = 55.0 + (LAT * (rand_r( &randState ) / (RAND_MAX + 1.0))) + ( 1/rand_r( &randState ) ) ;
	
	if( lon != FLT_MAX )
		lon_ = lon;
	else
		lon_ = 0.0 + (LON * (rand_r( &randState ) / (RAND_MAX + 1.0))) + ( 1/rand_r( &randState ) ) ;
}

void
GetThread::
operator()()
{
	ostringstream out;
	string request;
	string sLon("long");
	
	miutil::HTTPClient http;
	float tmpLat, tmpLon;
	string result;
	string::size_type i;
	string path;
	boost::crc_basic<16>::value_type crcValue;
	map<string, boost::crc_basic<16>::value_type > crcResults;
	// Simulate CRC-CCITT
	boost::crc_basic<16>  crc_ccitt1( 0x1021, 0xFFFF, 0, false, false );
	bool crcFail;
	
	//cerr << "URL: '" << url << "'" << endl;
	
	query->decode( url, true );

	path = query->getPath();
	
	if( query->hasParam( "lon") )
		sLon = "lon";
	
	tmpLat = query->asFloat( "lat", FLT_MAX );
	tmpLon = query->asFloat( sLon, FLT_MAX );
		
	if( tmpLat != FLT_MAX && tmpLon != FLT_MAX ) {
		lat = tmpLat;
		lon = tmpLon;
	}
	
	if( all_ )
		query->setValue( "from", "all" );

	/*
	if( ! query->hasParam( "from") ) 
		query->setValue( "from", "all" );
	*/
	while( nRuns > 0 ) {
		nRuns--;
		nextLatLong( tmpLat, tmpLon );
		query->setValue( "lat", tmpLat );
		query->setValue( sLon, tmpLon );
		
		request = query->encode( path ); //   url << "/?from=all;lat=" << lat <<";long=" << lon;
		out.str("");
		
		if( ! http.get( request, out ) ) {
			cout << "FAILED: t: " << id << " : " << request << endl;
			results->nFailed++;
		} else if( http.returnCode() != 200 ) {
			cout << "FAILED: t: " << id << " : code: " << http.returnCode() << " req: " << request << endl;
			results->nFailed++;
		} else {
			result = out.str();
			
			i = result.find( "<meta>");
			
			if( i != string::npos ) 
				result = result.substr( i );

			crc_ccitt1.reset();
			crc_ccitt1.process_bytes( result.data(), result.length() );
			crcValue = crc_ccitt1.checksum();
			
			map<string, boost::crc_basic<16>::value_type >::iterator it = crcResults.find( request );
			
			crcFail = false;
			
			if( it != crcResults.end() ) {
				if( it->second != crcValue ) {
					ostringstream fname;
					ofstream f;

					fname << "tmp/" << id << "-" << nRuns << ".txt";
					f.open( fname.str().c_str() );

					if( f ) {
						f << path << endl << result << endl;
						f.close();
					}

					results->nCrcFail++;
					crcFail = true;
					cout << "FAILED (CRC): t: " << id << " req: " << request << endl;

				}
			} else {
				crcResults[ request ] = crcValue;
			}
					
			if( ! crcFail ) {
				cout << "t: " << id << " : " << request << " len: " << out.str().size() 
			     	  << " CRC: " << crcValue <<  endl; // << "s["<<result <<				 "]" <<endl;
				results->nSuccess++;
			}
		}
	}
}

}


