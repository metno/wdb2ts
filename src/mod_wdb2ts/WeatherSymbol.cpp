/*
 * WeatherSymbolDataBuffer.h
 *
 *  Created on: Feb 28, 2014
 *      Author: borgem
 */

#include <ios>
#include <boost/thread/mutex.hpp>
#include "../miutil/mathalgo.h"
#include "WeatherSymbol.h"

using namespace std;

namespace {
struct MyFactories
{
	weather_symbol::Factory *factories[6];
	weather_symbol::Interpretor interpretor;
	MyFactories()
	{
		for( int i = 1; i <= 6; ++i )
			factories[i-1] = new weather_symbol::Factory( i );
	}

	~MyFactories() {
		for( int i=0; i<6; ++i )
			delete factories[i];
	}


	weather_symbol::Factory* get( int hours ) {
		if( hours<1 || hours > 6)
			return 0;

		return factories[hours-1];
	}

	std::string name( weather_symbol::Code code) {
			return interpretor.name( code );
	}


};

boost::mutex mutex;
MyFactories *factories=0;

}

namespace wdb2ts {



WeatherSymbolDataBuffer::
WeatherSymbolDataBuffer()
	: size_( INT_MAX )
{

}

WeatherSymbolDataBuffer::
WeatherSymbolDataBuffer( int size )
	: size_( size )
{

}

void
WeatherSymbolDataBuffer::
clear()
{
	data_.clear();
}

void
WeatherSymbolDataBuffer::
add( const boost::posix_time::ptime &time, const SymbolDataElement &data )
{
	if( time.is_special() )
		return;

	data_[time] = data;

	if( data_.size() > size_ ) {
		data_.erase( data_.begin() );
	}
}

WeatherSymbolDataBuffer::slice_iterator
WeatherSymbolDataBuffer::
slice( const boost::posix_time::ptime &from, boost::posix_time::ptime &to, int &timestep )const
{
	boost::posix_time::time_duration d;
	boost::posix_time::time_duration tmpDuration;
	const_iterator begin=data_.end();
	const_iterator it = data_.find( from );
	const_iterator prevIt = data_.end();

	timestep = INT_MAX;

//	cerr << "---- iterator: " << from << " - " << to << endl;

	if( it == data_.end()  )
		return slice_iterator( data_.end(), data_.end() );

//	cerr << "   iterator: check point 1: " << it->first << endl;
	prevIt = it;
	++it;
	if( it == data_.end() )
		return slice_iterator( data_.end(), data_.end()  );

//	cerr << "   iterator: check point 2: " << it->first << " prev: " << prevIt->first << endl;

	begin = it;
	d = begin->first - prevIt->first;
//	cerr << "   iterator: check point 3: " << it->first << " prev: " << prevIt->first
//		 << " d: " << d << endl;
	prevIt = it;

	for( ++it; it != data_.end()  &&  it->first <= to ; ++it ) {
		tmpDuration = it->first - prevIt->first;
		//cerr << "   iterator: check point 4: " << it->first << " prev: " << prevIt->first
		//		 << " tmpD: " << tmpDuration << endl;
		prevIt = it;

		if( tmpDuration != d )
			return slice_iterator( data_.end(), data_.end()  );
	}

	if( prevIt == data_.end() || prevIt->first != to  )
		return slice_iterator( data_.end(), data_.end()  );

	timestep = d.hours();
	//cerr << " ---- iterator: check point 5: " << begin->first << " - " << prevIt->first << endl;
	return slice_iterator( begin, prevIt );
}

WeatherSymbolDataBuffer::slice_iterator
WeatherSymbolDataBuffer::
slice( int hours, int &timestep )const
{
	const_reverse_iterator itTo=data_.rbegin();
	if( itTo == data_.rend() )
		return slice_iterator( data_.end(), data_.end()  );

	boost::posix_time::ptime from;
	boost::posix_time::ptime to = itTo->first;

	from = to - boost::posix_time::hours( hours );
	return slice( from, to, timestep );
}

std::ostream&
WeatherSymbolDataBuffer::
print( std::ostream &o, const WeatherSymbolDataBuffer::slice_iterator &it )const
{
	if( it.first == end() || it.second == end() )
		return o;

	for( WeatherSymbolDataBuffer::SymbolData::const_iterator sit = it.first;
			sit != it.second; ++sit )
		o << sit->first << ": " << sit->second << endl;

	o << it.second->first << ": " << it.second->second << endl;
	return o;
}


std::ostream&
operator<<( std::ostream &o, const wdb2ts::SymbolDataElement &data)
{
	ios_base::fmtflags oldflags = o.flags();
	streamsize oldprec=o.precision();

	o.setf( ios::fixed );
	o.precision( 2 );

	o << WeatherSymbolGenerator::symbolName( data.weatherCode ) << " ";

	if( data.temperature != FLT_MAX )
		o << "TA: " << data.temperature << " ";
	if( data.wetBulbTemperature != FLT_MAX )
		o << "TA.WetBulb: " << data.wetBulbTemperature << " ";
	if( data.totalCloudCover != FLT_MAX )
		o << "NN: " << data.totalCloudCover << " ";
	if( data.lowCloudCover != FLT_MAX )
		o << "lowClouds: " << data.lowCloudCover << " ";
	if( data.mediumCloudCover != FLT_MAX )
		o << "mediumClouds: " << data.mediumCloudCover << " ";
	if( data.highCloudCover != FLT_MAX )
		o << "highClouds: " << data.highCloudCover << " ";
	if( data.precipitation != FLT_MAX )
		o << "precip: " << data.precipitation << " ";
	if( data.minPrecipitation != FLT_MAX )
		o << "minPrecip: " << data.minPrecipitation << " ";
	if( data.maxPrecipitation != FLT_MAX )
		o << "maxPrecip: " << data.maxPrecipitation << " ";
	if( data.thunderProbability != FLT_MAX )
		o << "thunder: " << data.thunderProbability << " (" << (data.thunder?"T":"F") << ") ";
	else
		o << "thunder: " << (data.thunder?"T":"F") << " ";
	if( data.fogCover != FLT_MAX )
		o << "fog: " << data.fogCover << " (" << (data.fog?"T":"F") << ") ";
	else
		o << "maybeFog: " << (data.fog?"T":"F") << " ";

	o.flags( oldflags );
	o.precision( oldprec );
	return o;
}


std::ostream&
operator<<( std::ostream &o, const wdb2ts::WeatherSymbolDataBuffer &data)
{
	for( WeatherSymbolDataBuffer::SymbolData::const_iterator it = data.data_.begin();
		it != data.data_.end(); ++it )
		o << it->first << ": " << it->second << endl;
	return o;
}


namespace WeatherSymbolGenerator {

struct Greater {
	float n;
	Greater( float n ): n( n ) {}
	Greater( const Greater &g ): n( g.n ) {}
	bool operator()( float v )const {
		return v>n;
	}
};


void init() {
	boost::mutex::scoped_lock lock( mutex );
	if( factories )
		return;

	factories = new MyFactories();
}

std::string symbolName( weather_symbol::Code code )
{
	return factories->name( code );
}


SymbolDataElement
computeWeatherSymbolData(  const WeatherSymbolDataBuffer &data, int hours)
{
	namespace ma = miutil::algorithm;
	Greater greater( 25 );
	ma::Average<float> totCloud;
	ma::Average<float> mediumCloud;
	ma::Average<float> lowCloud;
	ma::Average<float> precip;
	ma::MinMax<float> thunder;
	ma::Count<float, Greater> fog( greater );
	int  possibleFogCount = ceil( static_cast<double>(hours)/2 );

	int timestep;
	WeatherSymbolDataBuffer::slice_iterator slice=data.slice( hours, timestep );

	if( slice.first == data.end() )
		return SymbolDataElement();

	WeatherSymbolDataBuffer::const_iterator it=slice.first;
	WeatherSymbolDataBuffer::const_iterator end = slice.second;
	++end;

	for(; it != end; ++it ) {
		totCloud( it->second.totalCloudCover );
		mediumCloud( it->second.mediumCloudCover );
		lowCloud( it->second.lowCloudCover );
		precip( it->second.precipitation );
		thunder( it->second.thunderProbability );
		fog( it->second.fogCover );
	}

	SymbolDataElement wd = slice.second->second;
	wd.totalCloudCover = totCloud.avg( FLT_MAX );
	wd.mediumCloudCover = mediumCloud.avg( FLT_MAX );
	wd.lowCloudCover = lowCloud.avg( FLT_MAX  );
	wd.precipitation = precip.sum( FLT_MAX );
	wd.thunder = thunder.max( 0 ) > 0.05;
	wd.fog = fog.countAbove() >= possibleFogCount;
	wd.from = slice.second->first - boost::posix_time::hours( hours );

	return wd;
}

SymbolDataElement
computeWeatherSymbol( const WeatherSymbolDataBuffer &data, int hours, float precip, float precipMin, float precipMax )
{

	SymbolDataElement wd = computeWeatherSymbolData( data, hours );
	wd.weatherCode = weather_symbol::Error;

	try {
		if( precip != FLT_MAX)
			wd.precipitation = precip;

		if( precipMin != FLT_MAX)
			wd.minPrecipitation = precipMin;

		if( precipMax != FLT_MAX)
			wd.maxPrecipitation = precipMax;

		weather_symbol::Factory *factory = factories->get( hours );

		if( factory )
			wd.weatherCode = factory->getSymbol( wd );
	}
	catch( const std::exception &ex ) {
		//cerr <<  "computeWeatherSymbol EXCEPTION: (" << ex.what() << ") " << wd << "\n";
		wd.weatherCode = weather_symbol::Error;
	}
	return wd;
}


SymbolDataElement
computeWeatherSymbol( const WeatherSymbolDataBuffer &data, int hours, weather_symbol::Code weatherCode )
{
	int timestep;
	int count=0;
	WeatherSymbolDataBuffer::slice_iterator slice = data.slice( hours, timestep );

	if( slice.first == data.end() )
		return SymbolDataElement();


	if( slice.first != slice.second )
		return SymbolDataElement();

//	//It should be no elements between first and second
//	for( WeatherSymbolDataBuffer::const_iterator it = slice.first; it != slice.second; ++it)
//		++count;
//
	SymbolDataElement wd = slice.second->second;
	wd.weatherCode = weather_symbol::Error;

	try {
		weather_symbol::Factory *factory = factories->get( hours );

		if( factory ) {
			wd.weatherCode = factory->getSymbol( weatherCode, wd );
			wd.from	= slice.second->first - boost::posix_time::hours( hours );

//		if( wd.weatherCode != weatherCode )
//			cerr << "computeWeatherSymbol (code): " << weather_symbol::name( weatherCode )
//			     << " " << slice.first->first << " - " << slice.second->first
//			     << " -> " << wd << endl;
		}

	}
	catch( const std::exception &ex ) {
		//cerr <<  "computeWeatherSymbol (code) EXCEPTION: (" << ex.what() << ") " << wd << "\n";
		wd.weatherCode = weather_symbol::Error;
	}
	return wd;
}
}



}


