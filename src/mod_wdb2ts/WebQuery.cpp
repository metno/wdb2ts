#include <stdio.h>
#include <list>
#include <sstream>
#include <boost/assign/list_inserter.hpp>
#include <boost/lexical_cast.hpp>
#include <WebQuery.h>
#include <UrlQuery.h>
#include <ptimeutil.h>
#include <trimstr.h>
#include <limits.h>
#include <float.h>

using namespace std;

namespace wdb2ts {

WebQuery::
WebQuery( const std::string &queryToDecode )
	: statusRequest( false )
{
	*this = decodeQuery( queryToDecode );
}

WebQuery::
WebQuery( )
	: altitude_(INT_MIN), statusRequest( false )
{
}


WebQuery::
WebQuery( const LocationPointList &locationPoints, int altitude,
		  const boost::posix_time::ptime &from,
		  const boost::posix_time::ptime &to,
		  const boost::posix_time::ptime &reftime,
		  const std::string &dataprovider,
		  const Level level,
		  bool isPolygon,
		  int skip,
		  bool nearestLand )
	: altitude_( altitude ),
	  from_( from ), to_( to ), reftime_( reftime ), dataprovider_( dataprovider ),
	  isPolygon_( isPolygon ), nearestLand_( nearestLand ), skip_( skip ), level_(level),
	  statusRequest( false )
{
	points = locationPoints;
}



boost::posix_time::ptime
WebQuery::
decodeTimeduration( const std::string &timeduration, const boost::posix_time::ptime &valueOnError )
{
	using namespace boost::posix_time;
	boost::posix_time::ptime to;
	int n;
	to = boost::posix_time::second_clock::universal_time();
	to += hours( 1 );
	to = ptime( to.date(),
			    time_duration( to.time_of_day().hours(), 0, 0, 0 ) );

	if( timeduration[timeduration.length()-1] == 't'  ||
		timeduration[timeduration.length()-1] == 'd'	) {

		if( timeduration.length() == 1 ) {
			n = 1;
		} else if( sscanf( timeduration.c_str(), "%d", &n ) != 1 ) {
			return valueOnError;
		}

		if( timeduration[timeduration.length()-1] == 'd' )
			n *= 24;

		to += boost::posix_time::time_duration( n, 0, 0, 0 );
	} else {
		try{
			to =  miutil::ptimeFromIsoString( timeduration );
		}
		catch( ... ) {
			return valueOnError;
		}
	}

	return to;
}

Level
WebQuery::
decodeLevel( const std::string &lvl_ )
{
	string lvl(lvl_);
	string buf;
	int h;
	int sign=1;

	miutil::trimstr( lvl );

	if( lvl.empty() )
		return Level();

	if( lvl[0]=='-') {
		sign = -1;
		lvl.erase( 0, 1 );
	}

	string::size_type i = lvl.find_first_not_of("0123456789");

	if( i == string::npos )
		buf = lvl;
	else
		buf = lvl.substr( 0, i );

	miutil::trimstr( buf );

	if( buf.empty() )
		throw std::logic_error("Invalid level '" + lvl_ + "' expecting a number.");

	h = atoi( buf.c_str() );
	h *= sign;

	lvl.erase( 0, buf.size() );
	buf = lvl;
	miutil::trimstr( buf );

	if( buf.empty() )
		buf = "m";

	if( buf == "m" ) {
		if( h < 0 ) {
			h = -1*h;
			return Level( h, h, "depth", "m" );
		} else {
			return Level( h, h, "height above ground", "m" );
		}
	} else if( buf == "Pa" ) {
		return Level( h, h, "isobaric surface", "Pa" );
	}

	throw std::logic_error("Invalid level '" + lvl_ + "'. Unsupported unit.");
}

/**
 * @exception logic_error
 */
WebQuery 
WebQuery::
decodeQuery( const std::string &queryToDecode, const std::string &urlPath )
{
	using namespace boost::posix_time;
	ostringstream ost;
	webfw::UrlQuery urlQuery;
	float lat(FLT_MAX);
	float lon(FLT_MAX);
	int alt(INT_MIN);
	boost::posix_time::ptime to;
	boost::posix_time::ptime from;
	boost::posix_time::ptime refTime;
	string dataprovider;
	bool isPolygon( false );
	int skip=0;
	Level level;

	try { 
		urlQuery.decode( queryToDecode );
		list<string> mustHaveParams;
		string  param;
		
		if( urlQuery.hasParam( "status") ) {
			WebQuery webQury;
			webQury.statusRequest = true;
			return webQury;
		}

		boost::assign::push_back( mustHaveParams )("long")("lat");
	  
		param = urlQuery.hasParams( mustHaveParams );

		if( urlQuery.hasParam("polygon") ){
			isPolygon = true;
		} else {
			if( ! param.empty() )
				throw logic_error("Missing mandatory parameter  '" + param + "'.");
	   	  
			lat = urlQuery.asFloat( "lat" );
			lon = urlQuery.asFloat( "long" );
			alt = urlQuery.asInt( "alt", INT_MIN );
		}
		
		if( urlQuery.hasParam( "from" ) ) {
			string sfrom=urlQuery.asString( "from", "" );
			if( sfrom=="all" )
				from = boost::posix_time::neg_infin;
			else
				from = urlQuery.asPTime("from", boost::posix_time::not_a_date_time );
		}
		
		if( urlQuery.hasParam( "to" ) )
			to = decodeTimeduration( urlQuery.asString( "to", "" ), boost::posix_time::pos_infin );

		//Adjust the fromtime to the nearest hour in the future. 
		if( from.is_not_a_date_time() ) {
			from = second_clock::universal_time();
			from += hours( 1 );
			from = ptime( from.date(), 
					        time_duration( from.time_of_day().hours(), 0, 0, 0 ));
		}
		
		if( urlQuery.hasParam( "refTime" ) ) 
			refTime = urlQuery.asPTime("fromTime", boost::posix_time::not_a_date_time );
		
		if( urlQuery.hasParam( "dataprovider" ) ) 
			dataprovider = urlQuery.asString("dataprovider", "" );

		LocationPointList myPoints;

		if( isPolygon ) {
			LocationPoint::decodePointList( urlQuery.asString( "polygon", "" ), myPoints );
		} else {
			myPoints.push_back( LocationPoint( lat, lon ) );
		}

      if( urlQuery.hasParam( "skip" ) ) {
         if( isPolygon ) {
            skip = urlQuery.asInt("skip", 0 );

            if( skip < 0 )
               skip=0;
         } else {
            skip = -1;
         }
      }

      if( urlQuery.hasParam( "level" ) ) {
    	  level = decodeLevel( urlQuery.asString("level", "") );
      }


      WebQuery webQury(myPoints, alt, from, to, refTime, dataprovider, level, isPolygon,
                       skip, urlQuery.asBool( "nearest_land", false ) );

      if( urlQuery.hasParam( "status") )
    	  webQury.statusRequest = true;

      if( urlPath.empty() )
         webQury.urlQuery_=queryToDecode;
      else
         webQury.urlQuery_= urlPath +"?"+queryToDecode;

      return webQury;
	}
	catch( const std::exception &ex ) {
		ostringstream ost;
		ost << "WebQuery::decodeQuery: Exception, decode query: " << ex.what() << endl 
		<< "Query: " << queryToDecode << endl;
		throw logic_error( ost.str() );
	}
	catch( ... ) {
		ostringstream ost;
		ost << "WebQuery::decodeQuery: Unknown Exception." << endl; 
		throw logic_error( ost.str() );
	}

}


bool 
WebQuery::
isValid()const
{
	if( points.empty() )
		return false;
	
	return true;
}

float
WebQuery::
latitude() const
{
	if( ! points.empty() )
		return points.begin()->latitude();

	return FLT_MAX;
}

float
WebQuery::
longitude()const
{
	if( ! points.empty() )
		return points.begin()->longitude();

	return FLT_MAX;
}

int
WebQuery::
altitude() const
{
	return altitude_;
}



}

