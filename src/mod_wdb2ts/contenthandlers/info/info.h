/*
 * info.h
 *
 *  Created on: Jun 8, 2017
 *      Author: borgem
 */

#ifndef SRC_MOD_WDB2TS_CONTENTHANDLERS_INFO_INFO_H_
#define SRC_MOD_WDB2TS_CONTENTHANDLERS_INFO_INFO_H_

#include <RequestHandler.h>

namespace wdb2ts {

class Info : public webfw::RequestHandler
{

	public:
	Info( int major, int minor )
			: webfw::RequestHandler( major, minor )
		{
		}

	virtual const char *name()const { return "info"; };

	virtual void get( webfw::Request  &req,
	                  webfw::Response &response,
	                  webfw::Logger   &logger );

};


}


#endif /* SRC_MOD_WDB2TS_CONTENTHANDLERS_INFO_INFO_H_ */
