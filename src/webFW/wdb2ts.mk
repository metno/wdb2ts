noinst_LTLIBRARIES+=		libwebFW.la

libwebFW_la_SOURCES=	src/webFW/App.cpp \
							src/webFW/App.h \
		               src/webFW/Response.cpp \
		               src/webFW/Response.h \
		               src/webFW/CErrLogger.cpp \
		               src/webFW/CErrLogger.h \
		               src/webFW/TestRequest.cpp \
		               src/webFW/TestRequest.h \
		               src/webFW/Request.cpp \
		               src/webFW/Request.h \
		               src/webFW/IAbortHandler.h \
		               src/webFW/IAbortHandlerManager.h \
		               src/webFW/RequestHandler.cpp \
		               src/webFW/RequestHandler.h \
		               src/webFW/RequestHandlerManager.cpp \
		               src/webFW/RequestHandlerManager.h \
		               src/webFW/DefaultRequestHandlerManager.cpp \
		               src/webFW/DefaultRequestHandlerManager.h \
		               src/webFW/RWMutex.cpp \
		               src/webFW/RWMutex.h \
		               src/webFW/DefaultResponse.cpp \
		               src/webFW/DefaultResponse.h \
		               src/webFW/ApacheRequest.cpp \
		               src/webFW/ApacheRequest.h \
		               src/webFW/ApacheLogger.cpp \
		               src/webFW/ApacheLogger.h \
		               src/webFW/ApacheStream.cpp \
		               src/webFW/ApacheStream.h \
		               src/webFW/ApacheResponse.cpp \
		               src/webFW/ApacheResponse.h \
		               src/webFW/ApacheAbortHandlerManager.h \
		               src/webFW/ApacheAbortHandlerManager.cpp \
		               src/webFW/UrlQuery.h \
		               src/webFW/UrlQuery.cpp \
		               src/webFW/exception.h \
		               src/webFW/ApacheModule.h \
		               src/webFW/Logger.h \
		               src/webFW/Logger4cpp.h \
		               src/webFW/macros.h 
		                                    

EXTRA_DIST+= src/webFW/wdb2ts.mk \
				 src/webFW/Makefile.am \
				 src/webFW/Makefile.in

DISTCLEANFILES +=	src/webFW/Makefile


# Local Makefile Targets
#-----------------------------------------------------------------------------

src/webFW/all: 

src/webFW/clean: clean
		                    