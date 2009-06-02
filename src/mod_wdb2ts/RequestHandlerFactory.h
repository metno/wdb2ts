#ifndef __REQUESTHANDLERFACTORY_H__
#define __REQUESTHANDLERFACTORY_H__

#include <Config.h>
#include <RequestHandler.h>

namespace wdb2ts {

webfw::RequestHandler*
requestHandlerFactory( const std::string &id, 
                       const wdb2ts::config::Version &ver );

}

#endif
