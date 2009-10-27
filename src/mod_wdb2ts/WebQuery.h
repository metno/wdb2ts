#ifndef __WDB2TS_WEBQUERY_H__
#define __WDB2TS_WEBQUERY_H__

#include <float.h>
#include <stdexcept>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <string>
#include <list>
#include <LocationPoint.h>

namespace wdb2ts {




class WebQuery {

	LocationPointList points;
	int altitude_;
	boost::posix_time::ptime from_;
	boost::posix_time::ptime to_;
	boost::posix_time::ptime reftime_;
	std::string dataprovider_;
	bool isPolygon_;

	static boost::posix_time::ptime
	decodeTimeduration( const std::string &timeduration, const boost::posix_time::ptime &valueOnError );

	
public:
	WebQuery( const std::string &queryToDecode );
	WebQuery( const LocationPointList &locationPoints, int altitude,
			  const boost::posix_time::ptime &from,
			  const boost::posix_time::ptime &to,
			  const boost::posix_time::ptime &reftime,
			  const std::string &dataprovider,
			  bool isPolygon = false );
	WebQuery( );

	/**
	 * @exception logic_error
	 */	
	static WebQuery decodeQuery( const std::string &queryToDecode );

	bool isValid()const;
	bool isPolygon()const { return isPolygon_; }
	
	float latitude() const;
	float longitude()const;
	LocationPointList locationPoints() const { return points; }
	int   altitude() const;
	boost::posix_time::ptime from() const { return from_; }
	boost::posix_time::ptime to() const { return to_; }
	boost::posix_time::ptime reftime() const { return reftime_; }
	std::string dataprovider() const { return dataprovider_; }
};


}

#endif 
