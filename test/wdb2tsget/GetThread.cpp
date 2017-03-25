#include <stdlib.h>
#include <GetThread.h>
#include <HTTPClient.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <boost/crc.hpp>      // for boost::crc_basic, boost::crc_optimal
#include <boost/cstdint.hpp>  // for boost::uint16_t
#include <miutil/Metric.h>

using namespace std;

namespace wdb2ts {

void 
GetThread::
nextLatLong( float &lat_, float &lon_ )
{
	nextLatLong_->nextLatLong(lat_, lon_);
}

void GetThread::validateResult( const string &path, const string &request, ostringstream &out) {
	string result;
	string::size_type i;
	result = out.str();

	i = result.find( "<meta>" );

	if( i != string::npos )
		result = result.substr( i );

	crc_ccitt1.reset();
	crc_ccitt1.process_bytes( result.data(), result.length() );
	crcValue = crc_ccitt1.checksum();

	map<string, boost::crc_basic<16>::value_type>::iterator it = crcResults->find(
			request );

	bool crcFail = false;

	if( it != crcResults->end() ){
		if( it->second != crcValue ){
			ostringstream fname;
			ofstream f;

			fname << "tmp/" << id << "-" << nRuns << ".txt";
			f.open( fname.str().c_str() );

			if( f ){
				f << path << endl << result << endl;
				f.close();
			}

			results->nCrcFail++;
			crcFail = true;
			cout << "FAILED (CRC): t: " << id << " req: " << request << endl;

		}
	}else{
		ostringstream fname;
		ofstream f;

		fname << "tmp/base-" << id << "-" << nRuns << ".txt";
		f.open( fname.str().c_str() );

		if( f ){
			f << path << endl << result << endl;
			f.close();
		}

		(*crcResults)[request] = crcValue;
	}

	if( !crcFail ){
		cout << "t: " << id << " : " << request << " len: " << out.str().size()
				<< " CRC: " << crcValue << endl; // << "s["<<result <<				 "]" <<endl;
		results->nSuccess++;
	}

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
	int errorCode;
	cerr << "URL: '" << url << "'" << endl;
	
	query->decode( url, true );

	path = query->getPath();
	
	if( query->hasParam( "lon") )
		sLon = "lon";
	
	tmpLat = query->asFloat( "lat", FLT_MAX );
	tmpLon = query->asFloat( sLon, FLT_MAX );
		
	
	if( all_ )
		query->setValue( "from", "all" );

	while( nRuns > 0 ) {
		nRuns--;
		nextLatLong( tmpLat, tmpLon );
		query->setValue( "lat", tmpLat );
		query->setValue( sLon, tmpLon );
		
		cerr << "Path:    '" << path << "'\n";
		request = query->encode( path ); //   url << "/?from=all;lat=" << lat <<";long=" << lon;
		cerr << "Request: '" << request << "'\n";
		out.str("");
		
		results->timer.startTimer();
		if( ! http.get( request, out, errorCode ) ) {
			cout << "FAILED: t: " << id << " curl retcode(" << errorCode <<")  : " << request << endl;
			results->nFailed++;
			results->timer.cancel();
		} else if( http.returnCode() == 503 ) {
		   cout << "UNAVAILABLE t: " << id << " : code: " << http.returnCode() << " req: " << request << endl;
		   results->unavailable++;
		}else if( http.returnCode() == 200 ) {
			results->timer.stopTimer();
			results->timer.count(1);
			validateResult(path, request, out);
		} else {
			cout << "FAILED: t: " << id << " : code: " << http.returnCode() << " req: " << request << endl;
			results->nFailed++;
			results->timer.cancel();
		}
	}
}

}


