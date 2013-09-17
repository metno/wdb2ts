#ifndef __WEBFW_LOGGER_4_CPP_H__
#define __WEBFW_LOGGER_4_CPP_H__

#include <RequestHandler.h>


#define WEBFW_USE_LOGGER( name ) \
	log4cpp::Category &webfw_logger___ = webfw::RequestHandler::getLogger( name )

#define WEBFW_CREATE_LOGGER_FILE( name ) \
    webfw::RequestHandler::getLogger( std::string("+") + name )


#define WEBFW_USE_LOGGER_REQUESTHANDLER( name, handler ) \
	log4cpp::Category &webfw_logger___ = webfw::RequestHandler::getLogger( name, handler )

#define WEBFW_SET_LOGGER( name ) \
	webfw_logger___ = webfw::RequestHandler::getLogger( name )

#define WEBFW_GET_LOGLEVEL( ) \
	webfw_logger___.getPriority()

#define WEBFW_SET_LOGLEVEL( loglevel ) \
	webfw_logger___.setPriority( loglevel )

#define WEBFW_LOG_TRACE( stream ) \
	{ webfw_logger___.debugStream() << stream; \
	webfw_logger___.debugStream().flush();}

#define WEBFW_LOG_DEBUG( stream ) \
	{webfw_logger___.debugStream() << stream; \
	webfw_logger___.debugStream().flush();}

#define WEBFW_LOG_INFO( stream ) \
	{webfw_logger___.infoStream() << stream; \
	webfw_logger___.infoStream().flush();}

#define WEBFW_LOG_NOTICE( stream ) \
	{webfw_logger___.noticeStream() << stream; \
	webfw_logger___.noticeStream().flush();}

#define WEBFW_LOG_WARN( stream ) \
	{webfw_logger___.warnStream() << stream; \
	webfw_logger___.warnStream().flush();}
	
#define WEBFW_LOG_ERROR( stream ) \
	{webfw_logger___.errorStream() << stream; \
	webfw_logger___.errorStream().flush();}

#define WEBFW_LOG_CRIT( stream ) \
	{webfw_logger___.critStream() << stream; \
	webfw_logger___.critStream().flush();}
	
#define WEBFW_LOG_ALERT( stream ) \
	{webfw_logger___.alertStream() << stream; \
	webfw_logger___.alertStream().flush();}

#define WEBFW_LOG_FATAL( stream ) \
	{webfw_logger___.fatalStream() << stream; \
	webfw_logger___.fatalStream().flush();}

#define WEBFW_LOG_EMERG( stream ) \
	{webfw_logger___.emergStream() << stream; \
	webfw_logger___.emergStream().flush();}

#endif 
