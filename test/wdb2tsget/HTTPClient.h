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
