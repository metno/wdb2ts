#ifndef __WDB2TS_WEBQUERY_H__
#define __WDB2TS_WEBQUERY_H__

#include <float.h>
#include <stdexcept>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <string>


namespace wdb2ts {

class WebQuery {
	float latitude_;
	float longitude_;
	int   altitude_;
	boost::posix_time::ptime from_;
	boost::posix_time::ptime reftime_;
	std::string dataprovider_;
	
public:
	WebQuery( const std::string &queryToDecode );
	WebQuery( float latitude, float longitude, int altitude, 
			    const boost::posix_time::ptime &from, 
			    const boost::posix_time::ptime &reftime,
			    const std::string &dataprovider );
	WebQuery( );

	/**
	 * @exception logic_error
	 */	
	static WebQuery decodeQuery( const std::string &queryToDecode );

	bool isValid()const;
	
	float latitude() const { return latitude_; }
	float longitude()const { return longitude_; }
	int   altitude() const { return altitude_; }
	boost::posix_time::ptime from() const { return from_; }
	boost::posix_time::ptime reftime() const { return reftime_; }
	std::string dataprovider() const { return dataprovider_; }
};


}

#endif 
