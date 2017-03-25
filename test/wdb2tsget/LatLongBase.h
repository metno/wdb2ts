/*
 * LatLongBase.h
 *
 *  Created on: Mar 23, 2017
 *      Author: borgem
 */

#ifndef TEST_WDB2TSGET_LATLONGBASE_H_
#define TEST_WDB2TSGET_LATLONGBASE_H_

#include <boost/thread/mutex.hpp>
#include <fstream>
#include <string>

class LatLongBase {
public:
	virtual ~LatLongBase(){}
	virtual void nextLatLong( float &lat_, float &lon_ )=0;
};

class LatLongSame : public LatLongBase {
	float lat, lon;
public:
	LatLongSame(float lat_, float lon_):lat(lat_), lon(lon_){}
	void nextLatLong( float &lat_, float &lon_ ){
		lat_=lat;
		lon=lon_;
	}

};


class LatLongRandom : public LatLongBase {
	unsigned int randState;
	boost::mutex mutex;
public:
	LatLongRandom():randState(time(0)){}
	void nextLatLong( float &lat_, float &lon_ ){
		const int LAT = 18;
		const int LON = 30;

		boost::mutex::scoped_lock lock(mutex);
		lat_ = 55.0 + (LAT * (rand_r( &randState ) / (RAND_MAX + 1.0))) + ( 1/rand_r( &randState ) ) ;
		lon_ = 0.0 + (LON * (rand_r( &randState ) / (RAND_MAX + 1.0))) + ( 1/rand_r( &randState ) ) ;
	}

};


class LatLongFile : public LatLongBase {
	std::string fname;
	std::ifstream fs;
	boost::mutex mutex;
public:
	LatLongFile(const std::string &file, bool startAtRandomLine):fname(file), fs(fname.c_str()){}
	bool isOpen()const{ return fs.is_open();}
	void nextLatLong( float &lat_, float &lon_ ){
		std::string line;
		boost::mutex::scoped_lock lock(mutex);

		for( int i=0; i<4; ++i) {
			if( std::getline(fs, line)) {
				if( sscanf(line.c_str(), "%f,%f", &lat_, &lon_)==2)
					return;
			} else if( fs.eof()) {
				fs.close();
				fs.open(fname.c_str());
			}
		}

		//Failed, return for galdhøpiggen
		lat_=61.63639;
		lon_=8.3125;
	}
};



#endif /* TEST_WDB2TSGET_LATLONGBASE_H_ */
