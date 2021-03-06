#include <iostream>
#include <HandlerBase.h>
#include <App.h>
#include <wdb2TsApp.h>
#include <Logger4cpp.h>

using namespace std;

namespace wdb2ts {

ParamDefList
HandlerBase::
getParamdef()const
{
	ParamDefList params = Wdb2TsApp::app()->getParamDefs();

	if( params.empty() ) {
		params = config.paramdef.paramDefs("default");

		if( params.empty() )
			params = config.paramdef.paramDefs("");
	}
	
	params.idDefsParams = new wdb2ts::config::ParamDefConfig( config.paramdef );
	return params;
}


bool 
HandlerBase::
/*doConfigure( const wdb2ts::config::ActionParam &params,
		       const wdb2ts::config::Config::Query &query,
		       const std::string &wdbDB,
		       const std::string &schema_
		       )*/

doConfigure( boost::shared_ptr<wdb2ts::config::RequestConf> conf,
             const wdb2ts::config::Config::Query &query )

{
   config = *conf;
	schema = config.schema.asString("");

	wdb2ts::config::ActionParam::const_iterator it=config.actionParam.find("loglevels");
			
	if( it != config.actionParam.end() )  {
		try {
			map<string,int> loggers = webfw::decodeLogLevels( it->second.asString() );
			setLogLevels( loggers );
		}
		catch( const std::exception &ex ) {
			WEBFW_USE_LOGGER( "handler" );
			WEBFW_LOG_ERROR( "wdb2ts: " << name() << " : " << ex.what() );
		}
	}		
	
	return configure( config.actionParam, query, config.wdbDB.asString("") );

}

};

