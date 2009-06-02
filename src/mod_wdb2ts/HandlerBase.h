#ifndef __ICONFIGURE_H__
#define __ICONFIGURE_H__

#include <RequestHandler.h>
#include <Config.h>

namespace wdb2ts {

class HandlerBase 
	: public webfw::RequestHandler
{
protected:
	std::string wdbdb;
	
public:
	HandlerBase() {}
	
	HandlerBase( int major, int minor )
		: webfw::RequestHandler( major, minor )
	{
	}

	bool doConfigure( const wdb2ts::config::ActionParam &params,
			            const wdb2ts::config::Config::Query &query,
	         			const std::string &wdbDB );
		
	
	virtual bool configure( const wdb2ts::config::ActionParam &params,
			                  const wdb2ts::config::Config::Query &query,
			                  const std::string &wdbDB )=0;

};

};

#endif 
