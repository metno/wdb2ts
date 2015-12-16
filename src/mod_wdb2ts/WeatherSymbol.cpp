/*
 * WeatherSymbolDataBuffer.h
 *
 *  Created on: Feb 28, 2014
 *      Author: borgem
 */

#include <ios>
#include <algorithm>
#include <boost/thread/mutex.hpp>
#include <boost/function.hpp>
#include "../miutil/mathalgo.h"
#include "WeatherSymbol.h"

using namespace std;
namespace ma = miutil::algorithm;
namespace {

struct MyFactories
{
	weather_symbol::Factory *factories[6];
	static weather_symbol::Interpretor interpretor;
	MyFactories()
	{
		for( int i = 1; i <= 6; ++i )
			factories[i - 1] = new weather_symbol::Factory( i );
	}

	~MyFactories()
	{
		for( int i = 0; i < 6; ++i )
			delete factories[i];
	}

	weather_symbol::Factory* get( int hours )
	{
		if( hours < 1 || hours > 6 )
			return 0;

		return factories[hours - 1];
	}

	static std::string name( weather_symbol::Code code )
	{
		return interpretor.name( code );
	}

	static bool hasThunder( weather_symbol::Code code )
	{
		return interpretor.hasThunder( code );
	}

	static weather_symbol::Code setThunderInSymbol( bool thunder,
			weather_symbol::Code code )
	{
		if( thunder )
			return interpretor.turnOnThunder( code );
		else
			return interpretor.turnOffThunder( code );
	}
};

weather_symbol::Interpretor MyFactories::interpretor;

boost::mutex mutex;
MyFactories *factories = 0;

}

namespace wdb2ts {

WeatherSymbolDataBuffer::WeatherSymbolDataBuffer() :
		size_( INT_MAX )
{

}

WeatherSymbolDataBuffer::WeatherSymbolDataBuffer( int size ) :
		size_( size )
{

}

void WeatherSymbolDataBuffer::clear()
{
	data_.clear();
}

void WeatherSymbolDataBuffer::add( const boost::posix_time::ptime &time,
		const SymbolDataElement &data )
{
	if( time.is_special() )
		return;

	data_[time] = data;

	if( data_.size() > size_ ){
		data_.erase( data_.begin() );
	}
}

void WeatherSymbolDataBuffer::updateWeatherSymbol(
		const boost::posix_time::ptime &time, const weather_symbol::Code &symbol )
{
	SymbolData::iterator it = data_.find( time );

	if( it == data_.end() )
		return;

	it->second.weatherCode = symbol;
}

bool WeatherSymbolDataBuffer::hasThunder( const boost::posix_time::ptime &to,
		int hours ) const
{
	slice_iterator itSlice = slice( to, hours );
	return hasThunder( itSlice );
}

bool WeatherSymbolDataBuffer::hasThunder( const slice_iterator &slice ) const
{
	const_iterator it = slice.first;
	const_iterator end = slice.second;
	++end;

	if( !slice )
		throw std::logic_error( "hasThunder: Missing data" );

	for( ; it != end; ++it ){
		if( it->second.weatherCode == weather_symbol::Error )
			throw std::logic_error( "hasThunder: Symbol not set in dataset." );
		else if( MyFactories::hasThunder( it->second.weatherCode ) )
			return true;
	}

	return false;
}

WeatherSymbolDataBuffer::slice_iterator WeatherSymbolDataBuffer::slice(
		const boost::posix_time::ptime &from,
		const boost::posix_time::ptime &to ) const
{
	boost::posix_time::time_duration d;
	boost::posix_time::time_duration tmpDuration;
	const_iterator it;
	const_iterator begin = data_.find( from );
	const_iterator prevIt = data_.end();

	if( begin == data_.end() )
		return slice_iterator( data_ );

	prevIt = begin;
	it = begin;
	++it;

	if( it == data_.end() )
		return slice_iterator( data_ );

	d = it->first - prevIt->first;
	prevIt = it;

	for( ++it; it != data_.end() && it->first <= to; ++it ){
		tmpDuration = it->first - prevIt->first;
		prevIt = it;

		if( tmpDuration != d )
			return slice_iterator( data_ );
	}

	if( prevIt == data_.end() || prevIt->first != to )
		return slice_iterator( data_ );

	return slice_iterator( data_, begin, prevIt, d.hours() );
}

WeatherSymbolDataBuffer::slice_iterator WeatherSymbolDataBuffer::slice(
		const boost::posix_time::ptime &to, int hours ) const
{
	const_iterator it = data_.find( to );

	if( it == data_.end() )
		return slice_iterator( data_ );

	boost::posix_time::ptime from = to - boost::posix_time::hours( hours );

	return slice( from, to );
}

WeatherSymbolDataBuffer::slice_iterator WeatherSymbolDataBuffer::slice(
		int hours ) const
{
	const_reverse_iterator itTo = data_.rbegin();
	if( itTo == data_.rend() )
		return slice_iterator( data_ );

	boost::posix_time::ptime from;
	boost::posix_time::ptime to = itTo->first;

	from = to - boost::posix_time::hours( hours );
	return slice( from, to );
}

std::ostream&
WeatherSymbolDataBuffer::print( std::ostream &o,
		const WeatherSymbolDataBuffer::slice_iterator &it ) const
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
operator<<( std::ostream &o, const wdb2ts::SymbolDataElement &data )
{
	ios_base::fmtflags oldflags = o.flags();
	streamsize oldprec = o.precision();

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
		o << "thunder: " << data.thunderProbability << " ("
				<< (data.thunder ? "T" : "F") << ") ";
	else
		o << "thunder: " << (data.thunder ? "T" : "F") << " ";
	if( data.fogCover != FLT_MAX )
		o << "fog: " << data.fogCover << " (" << (data.fog ? "T" : "F") << ") ";
	else
		o << "maybeFog: " << (data.fog ? "T" : "F") << " ";

	o.flags( oldflags );
	o.precision( oldprec );
	return o;
}

std::ostream&
operator<<( std::ostream &o, const wdb2ts::WeatherSymbolDataBuffer &data )
{
	for( WeatherSymbolDataBuffer::SymbolData::const_iterator it =
			data.data_.begin(); it != data.data_.end(); ++it )
		o << it->first << ": " << it->second << endl;
	return o;
}

namespace WeatherSymbolGenerator {

struct Greater
{
	float n;
	Greater( float n ) :
			n( n )
	{
	}
	Greater( const Greater &g ) :
			n( g.n )
	{
	}
	bool operator()( float v ) const
	{
		return v > n;
	}
};

void init()
{
	boost::mutex::scoped_lock lock( mutex );
	if( factories )
		return;

	factories = new MyFactories();
}

std::string symbolName( weather_symbol::Code code )
{
	return factories->name( code );
}

struct Thunder
{
	typedef boost::function<float( float prob, float precip )> F;

	F thunderProb;

	static float noThunder( float prob, float precip )
	{
		return 0;
	}

	static float possibleThunder( float prob, float precip )
	{
		//We may only have thunder if the precipitation >= 0.1
		return (precip == FLT_MAX || precip < 0.1) ? 0 : prob;
	}

	Thunder( F func )
	{
		thunderProb = func;
	}

	Thunder( int timestep )
	{
		if( timestep == 1 )
			thunderProb = &possibleThunder;
		else
			thunderProb = &noThunder;
	}

	float operator()( float prob, float precip )
	{
		return thunderProb( prob, precip );
	}
};

SymbolDataElement computeBaseWeatherSymbolData( WeatherSymbolDataBuffer &data,
		const WeatherSymbolDataBuffer::slice_iterator &slice, int hours )
{
	Greater greater( 25 );
	ma::Average<float> totCloud;
	ma::Average<float> mediumCloud;
	ma::Average<float> lowCloud;
	ma::Average<float> precip;
	ma::MinMax<float> thunder;
	ma::Count<float, Greater> fog( greater );
	ma::MinMax<float> temperaturExtrema;
	float precipVal;
	int count = 0;
	int possibleFogCount = ceil( static_cast<double>( hours ) / 2 );
	float tempInFirst = FLT_MAX;

	if( !slice )
		return SymbolDataElement();

	Thunder thunderProb( slice.timestep );
	WeatherSymbolDataBuffer::const_iterator itBeforeFirst = slice.start;
	WeatherSymbolDataBuffer::const_iterator it = slice.first;
	WeatherSymbolDataBuffer::const_iterator end = slice.second;
	WeatherSymbolDataBuffer::const_iterator lastIt = end;
	++end;

	if( itBeforeFirst->second.temperature != FLT_MAX )
		tempInFirst = itBeforeFirst->second.temperature;

	int debug = 0;

	for( ; it != end; ++it ){
		++count;
		lastIt = it;
		precipVal = it->second.precipitation;
		totCloud( it->second.totalCloudCover );
		mediumCloud( it->second.mediumCloudCover );
		lowCloud( it->second.lowCloudCover );
		precip( std::max( float( 0 ), precipVal ) );
		thunder( thunderProb( it->second.thunderProbability, precipVal ) );
		fog( it->second.fogCover );
		temperaturExtrema( it->second.temperature );
	}

	SymbolDataElement wd = slice.second->second;
	wd.totalCloudCover = totCloud.avg( FLT_MAX );
	wd.mediumCloudCover = mediumCloud.avg( FLT_MAX );
	wd.lowCloudCover = lowCloud.avg( FLT_MAX );
	wd.precipitation = precip.sum( FLT_MAX );
	wd.thunder = thunder.max( 0 ) > 0.05;
	wd.fog = fog.countAbove() >= possibleFogCount;
	wd.from = slice.second->first - boost::posix_time::hours( hours );
	wd.to = slice.second->first;

	if( hours == 6 ){
		float min = temperaturExtrema.min( FLT_MAX );
		float max = temperaturExtrema.max( FLT_MAX );

		if( count == 6 ){
			wd.maxTemperature_6h = max;
			wd.minTemperature_6h = min;
		}else if( lastIt != end ){
			wd.maxTemperature_6h = ma::max( lastIt->second.maxTemperature_6h,	max, FLT_MAX );
			wd.minTemperature_6h = ma::min( lastIt->second.minTemperature_6h, min, FLT_MAX );
		}

		//Adjust the min/max temperature in the first endpoint.
		wd.maxTemperature_6h = ma::max( wd.maxTemperature_6h, tempInFirst, FLT_MAX );
		wd.minTemperature_6h = ma::min( wd.minTemperature_6h, tempInFirst, FLT_MAX );
	}
	return wd;
}

SymbolDataElement computeWeatherSymbolData( WeatherSymbolDataBuffer &data,
		int hours, int &timestep )
{
	WeatherSymbolDataBuffer::slice_iterator slice = data.slice( hours );
	timestep = slice.timestep;
	return computeBaseWeatherSymbolData( data, slice, hours );
}

SymbolDataElement computeWeatherSymbol( WeatherSymbolDataBuffer &data,
		int hours, float precip, float precipMin, float precipMax )
{
	WeatherSymbolDataBuffer::slice_iterator slice = data.slice( hours );

	if( !slice )
		return SymbolDataElement();

	SymbolDataElement wd = computeBaseWeatherSymbolData( data, slice, hours );
	wd.weatherCode = weather_symbol::Error;

	try{
		if( precip != FLT_MAX )
			wd.precipitation = std::max( float( 0 ), precip );

		if( precipMin != FLT_MAX )
			wd.minPrecipitation = std::max( float( 0 ), precipMin );

		if( precipMax != FLT_MAX )
			wd.maxPrecipitation = std::max( float( 0 ), precipMax );

		weather_symbol::Factory *factory = factories->get( hours );

		if( factory ){
			wd.weatherCode = factory->getSymbol( wd );

			if( !factory->interpretor()->hasPrecipitation( wd.weatherCode ) ){
				wd.precipitation = 0;
				wd.minPrecipitation = 0;
			}

			if( slice.timestep == hours ){
				data.updateWeatherSymbol( wd.to, wd.weatherCode );
			}else{
				try{
					//The base symbols decide if there is thunder.
					wd.weatherCode = MyFactories::setThunderInSymbol(
							data.hasThunder( slice ), wd.weatherCode );
				} catch( const std::exception &e ){
//					cerr << "EXCEPTION: WeatherSymbol: thunder: " << e.what()
//							<<  " (" << wd.from << " - " << wd.to << ")" <<endl;
				}
			}
		}
	} catch( const std::exception &ex ){
		wd.weatherCode = weather_symbol::Error;
	}
	return wd;
}

SymbolDataElement computeWeatherSymbol( WeatherSymbolDataBuffer &data,
		int hours, weather_symbol::Code weatherCode )
{
	WeatherSymbolDataBuffer::slice_iterator slice = data.slice( hours );

	if( !slice )
		return SymbolDataElement();

	if( slice.first != slice.second )
		return SymbolDataElement();

	SymbolDataElement wd = slice.second->second;
	wd.from = slice.second->first - boost::posix_time::hours( hours );
	wd.to = slice.second->first;

	wd.weatherCode = weather_symbol::Error;

	try{
		weather_symbol::Factory *factory = factories->get( hours );

		if( factory ){
			float tempInFirst = slice.start->second.temperature;
			wd.weatherCode = factory->getSymbol( weatherCode, wd );
			wd.from = slice.second->first - boost::posix_time::hours( hours );

			if( !factory->interpretor()->hasPrecipitation( wd.weatherCode ) ){
				wd.precipitation = 0;
			}

			if( hours == 6 ){
				wd.minTemperature_6h = ma::min( wd.minTemperature_6h,
							wd.temperature, FLT_MAX );
					wd.maxTemperature_6h = ma::max( wd.maxTemperature_6h,
							wd.temperature, FLT_MAX );
			}else{
				wd.minTemperature_6h = FLT_MAX;
				wd.maxTemperature_6h = FLT_MAX;
			}

			//Adjust the min/max temperature in the first endpoint.
			wd.maxTemperature_6h = ma::max( wd.maxTemperature_6h,
						tempInFirst, FLT_MAX );
			wd.minTemperature_6h = ma::min( wd.minTemperature_6h,	tempInFirst, FLT_MAX );

			if( slice.timestep == hours ){
				data.updateWeatherSymbol( wd.to, wd.weatherCode );
			}
		}
	} catch( const std::exception &ex ){
		wd.weatherCode = weather_symbol::Error;
	}
	return wd;
}
}

}

