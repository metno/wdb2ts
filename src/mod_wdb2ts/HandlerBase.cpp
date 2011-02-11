#include <iostream>
#include <HandlerBase.h>
#include <App.h>
#include <Logger4cpp.h>

using namespace std;

namespace wdb2ts {
	
bool 
HandlerBase::
doConfigure( const wdb2ts::config::ActionParam &params,
		       const wdb2ts::config::Config::Query &query,
		       const std::string &wdbDB,
		       const std::string &schema_
		       )
{
	schema = schema_;
	wdb2ts::config::ActionParam::const_iterator it=params.find("loglevels");
			
	if( it != params.end() )  {
		try {
			map<string,int> loggers = webfw::decodeLogLevels( it->second.asString() );
			setLogLevels( loggers );
		}
		catch( const std::exception &ex ) {
			WEBFW_USE_LOGGER( "handler" );
			WEBFW_LOG_ERROR( "wdb2ts: " << name() << " : " << ex.what() );
		}
	}		
	
	return configure( params, query, wdbDB );

}

};

