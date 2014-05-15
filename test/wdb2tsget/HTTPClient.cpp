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


#include <HTTPClient.h>

namespace {
size_t write_function( void *ptr, size_t size, size_t nmemb, void *stream);

}

namespace miutil {

using namespace std;

HTTPClient::
HTTPClient()
	: curl( 0 ), out( 0 )
{ 
	open();
}

HTTPClient::
~HTTPClient()
{
	close();
}


void
HTTPClient::
init()
{
	curl_global_init( CURL_GLOBAL_ALL );
}

bool
HTTPClient::
open()
{
	close();
	
	curl = curl_easy_init();
	
	if( curl ){
		if( curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, write_function ) != CURLE_OK ) {
			close();
			return false;
		}
		if( curl_easy_setopt( curl, CURLOPT_WRITEDATA, this ) != CURLE_OK ) {
			close();
			return false;
		}
	}
	
	return curl != 0;
}

void
HTTPClient::
close()
{
	if( curl ) {
		curl_easy_cleanup( curl );
		curl = 0;
	}
}
	
bool 
HTTPClient::
get( const std::string &url, std::ostream &content, int &error_code )
{
	CURLcode ret;
	
	if( ! curl )
		return false;
	
	if( curl_easy_setopt( curl, CURLOPT_HTTPGET, 1 ) != CURLE_OK ) {
		close();
		return false;
	}
	
	ret = curl_easy_setopt(curl, CURLOPT_URL, url.c_str() );
	
	if( ret != CURLE_OK) {
		return false;
	}

	out = &content;
	ret = curl_easy_perform( curl );
	out = 0;
	error_code = ret;
	return ret == CURLE_OK;
}
	
long
HTTPClient::
contenlLength()const
{
	double d;
	
	if( curl ) {
		if( curl_easy_getinfo( curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &d ) == CURLE_OK )
			return static_cast<long>( d );
	}
	
	return -1;
}

std::string 
HTTPClient::
contentType()const
{
	char *ct;
	
	if( curl ) {
		if( curl_easy_getinfo( curl, CURLINFO_CONTENT_TYPE, &ct ) == CURLE_OK && ct != NULL )
			return string( ct );
	}
		
	return string();
}

long
HTTPClient::
returnCode()const
{
	long rc;
	
	if( curl ) {
		if( curl_easy_getinfo( curl, CURLINFO_RESPONSE_CODE, &rc ) == CURLE_OK )
			return rc;
	}
		
	return -1;
}

double 
HTTPClient::
totalTime()const
{
	double dt;
	
	if( curl ) {
		if( curl_easy_getinfo( curl, CURLINFO_TOTAL_TIME, &dt ) == CURLE_OK )
			return dt;
	}
	
	return dt;
}
	
}


namespace {
size_t 
write_function( void *ptr, size_t size, size_t nmemb, void *stream)
{
	size_t n=0;
	int len = size*nmemb;
	
	if( stream != NULL ) {
		miutil::HTTPClient *client = static_cast<miutil::HTTPClient*>( stream );
		std::ostream *out = client->outStream();
		
		if( out ) {
			out ->write(static_cast<char*>( ptr ), len );
			n = len;
		}
	}
	
	return n;
}

}
