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

#ifndef __HTTPCLIENT_H__
#define __HTTPCLIENT_H__

#include <curl/curl.h>
#include <ostream>
#include <string>

namespace miutil {

class HTTPClient{
	CURL *curl;
	std::ostream *out;
	
public:
	HTTPClient();
	~HTTPClient();
	
	/**
	 * Implementation detail. Dont use.
	 */
	std::ostream *outStream(){ return out; }
	
	
	/**
	 * Initalize the library.
	 * In threaded application this must be called before any other
	 * operations is performed.
	 */
	static void init();
	
	bool open();
	void close();
	
	bool get( const std::string &url, std::ostream &content );
	
	long contenlLength()const;
	std::string contentType()const;
	long returnCode()const;
	double totalTime()const;
	
};

}

#endif 
