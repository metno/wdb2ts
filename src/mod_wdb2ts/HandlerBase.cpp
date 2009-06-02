#include <iostream>
#include <HandlerBase.h>
#include <App.h>

using namespace std;

namespace wdb2ts {
	
bool 
HandlerBase::
doConfigure( const wdb2ts::config::ActionParam &params,
		       const wdb2ts::config::Config::Query &query,
		       const std::string &wdbDB )
{
	
	wdb2ts::config::ActionParam::const_iterator it=params.find("loglevels");
			
	if( it != params.end() )  {
		try {
			map<string,int> loggers = webfw::decodeLogLevels( it->second.asString() );
			setLogLevels( loggers );
		}
		catch( const std::exception &ex ) {
			cerr << "wdb2ts: " << name() << " : " << ex.what() << endl; 
		}
	}		
	
	return configure( params, query, wdbDB );

}

};

