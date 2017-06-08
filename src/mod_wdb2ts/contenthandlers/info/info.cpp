/*
 * info.h
 *
 *  Created on: Jun 8, 2017
 *      Author: borgem
 */


#include <string>
#include <list>
#include <sstream>
#include <wdb2TsApp.h>
#include <Logger4cpp.h>
#include "contenthandlers/info/info.h"


namespace wdb2ts {

void Info::get( webfw::Request  &req,
	                  webfw::Response &response,
	                  webfw::Logger   &logger )
{
	std::ostringstream ost;
   std::ostream &out = response.out();
	response.contentType("text/plain");
   response.directOutput( false );

	WEBFW_USE_LOGGER( "handler" );
	WEBFW_LOG_DEBUG("Info");

	Wdb2TsApp *app=Wdb2TsApp::app();

	std::list<std::string> paths=app->listBasePaths();

	out << "Defined paths:\n\n";
	for(std::list<std::string>::iterator it = paths.begin(); it != paths.end(); ++it ) {
		out << *it << "\n";
	}

	response.status(webfw::Response::NO_ERROR);
}


}

