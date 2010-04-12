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
#include <string>
#include <sstream>
#include <HTTPClient.h>
#include <list>
#include <GetThread.h>
#include <gettimeofday.h>
#include <boost/program_options.hpp>

using namespace std;
namespace po = boost::program_options;

void 
doOptions(po::variables_map &vm, int argc, char **argv );


int
main( int argn, char **argv )
{
	//string url("http://localhost:8080/wdb2ts/locationforecast/?long=12;lat=60;from=all");
	string url("http://localhost:8080/wdb2ts/locationforecast");
	int nThreads;
	int nRuns;
	list<wdb2ts::GetThread*> getThreads;
	boost::thread_group getThreadsGroup;
	double start, stop;
	float lat, lon;
	bool all=false;
	po::variables_map vm;
	
	doOptions( vm, argn, argv );
		
	miutil::HTTPClient::init();
	
	start = miutil::gettimeofday();
	
	nThreads = vm["threads"].as<int>();
	nRuns = vm["runs"].as<int>();
	url = vm["request"].as<string>();
	all = vm.count( "all" )>0?true:false;
	
	try {
		lat = vm["lat"].as<float>();
		lon = vm["lon"].as<float>();
	}
	catch( ... ) {
		cerr << "Failed to cast to float!" << endl;
	}
	
	for( int i=0; i<nThreads; ++i ) {
		wdb2ts::GetThread *th = new wdb2ts::GetThread( url, lat, lon, nRuns, (unsigned int) i, i);
		th->all( all );
		getThreads.push_back( th );
		getThreadsGroup.add_thread( new boost::thread( *th ) );
	}
		
	getThreadsGroup.join_all();
	
	stop = miutil::gettimeofday();
	
	int failed=0;
	int success=0;
	int runs=0;
	int crcFail=0;
	
	for( list<wdb2ts::GetThread*>::iterator it = getThreads.begin(); 
	     it != getThreads.end(); 
	     ++it ) 
	{
		failed  += (*it)->failed();
		success += (*it)->success();
		runs += (*it)->runs();
		crcFail += (*it)->crcFail();
	}
	double runTime( stop-start );
	cout << "Runtime:          " << runTime << " seconds " << endl;
	cout << "seconds/request:  " << runTime/runs << " seconds" << endl;
	cout << "#request/seconds: " << runs/runTime << endl; 
	cout << "Runs: " << runs << " success: " << success << " Failed: " << failed <<  " CRCFail: " <<  crcFail << endl;

	return 0;
}


void
doOptions(po::variables_map &vm, int argn, char **argv )
{
	try {
		po::options_description desc("Allowed options");
		desc.add_options()
		        ("help", "This help message")
		        ("threads,t", po::value<int>()->default_value( 1 ), "Number of request threads.")
              ("runs,n", po::value<int>()->default_value( 1 ), "Number of request per thread.")
              ("request,s", po::value<string>()->default_value( "http://localhost:8080/wdb2ts/locationforecast" ), "The wdb2ts request to run")
              ("lat", po::value<float>()->default_value( FLT_MAX ), "Use only this latitude.")
              ("lon", po::value<float>()->default_value( FLT_MAX ), "Use only this longitude.")
              ("all", "Return the result for all times in the fields.")
	        ;
	
		po::store( po::parse_command_line( argn, argv, desc), vm );
		po::notify( vm );   
	
		if (vm.count("help")) {
			cout << desc << "\n";
			exit( 1 );
		}
	}
	catch(exception& e) {
		cerr << "error: " << e.what() << "\n";
		exit( 1 );
	}
	catch(...) {
		cerr << "Exception of unknown type!\n";
		exit( 1 );
	}
}
