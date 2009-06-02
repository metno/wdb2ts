#ifndef __WEBFW_LOGGER_4_CPP_H__
#define __WEBFW_LOGGER_4_CPP_H__

#include <RequestHandler.h>


#define WDB2TS_USE_LOGGER( name ) \
	log4cpp::Category &wdb2ts_logger___ = webfw::RequestHandler::getLogger( name )

#define WDB2TS_SET_LOGGER( name ) \
	wdb2ts_logger___ = webfw::RequestHandler::getLogger( name )
	

#define WDB2TS_LOG_TRACE( stream ) \
	wdb2ts_logger___.debugStream() << stream; \
	wdb2ts_logger___.debugStream().flush()

#define WDB2TS_LOG_DEBUG( stream ) \
	wdb2ts_logger___.debugStream() << stream; \
	wdb2ts_logger___.debugStream().flush()

#define WDB2TS_LOG_INFO( stream ) \
	wdb2ts_logger___.infoStream() << stream; \
	wdb2ts_logger___.infoStream().flush()

#define WDB2TS_LOG_NOTICE( stream ) \
	wdb2ts_logger___.noticeStream() << stream; \
	wdb2ts_logger___.noticeStream().flush()

#define WDB2TS_LOG_WARN( stream ) \
	wdb2ts_logger___.warnStream() << stream; \
	wdb2ts_logger___.warnStream().flush()
	
#define WDB2TS_LOG_ERROR( stream ) \
	wdb2ts_logger___.errorStream() << stream; \
	wdb2ts_logger___.errorStream().flush()

#define WDB2TS_LOG_CRIT( stream ) \
	wdb2ts_logger___.critStream() << stream; \
	wdb2ts_logger___.critStream().flush()
	
#define WDB2TS_LOG_ALERT( stream ) \
	wdb2ts_logger___.alertStream() << stream; \
	wdb2ts_logger___.alertStream().flush()

#define WDB2TS_LOG_FATAL( stream ) \
	wdb2ts_logger___.fatalStream() << stream; \
	wdb2ts_logger___.fatalStream().flush()

#define WDB2TS_LOG_EMERG( stream ) \
	wdb2ts_logger___.emergStream() << stream; \
	wdb2ts_logger___.emergStream().flush()

#endif 
