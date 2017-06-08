#-----------------------------------------------------------------------------
# wdb2tsget - performance test program.
#-----------------------------------------------------------------------------
if BUILD_WDB2TSGET
bin_PROGRAMS+= wdb2tsget
else
EXTRA_PROGRAMS = wdb2tsget
endif

wdb2tsget_SOURCES= test/wdb2tsget/wdb2tsget_test.cpp \
                   test/wdb2tsget/HTTPClient.h \
                   test/wdb2tsget/HTTPClient.cpp \
                   test/wdb2tsget/GetThread.h \
                   test/wdb2tsget/GetThread.cpp \
                   test/wdb2tsget/LatLongBase.h
                     
wdb2tsget_CPPFLAGS= $(AM_CPPFLAGS) $(LIBCURL_CPPFLAGS)
wdb2tsget_LDADD= -lwebFW -lmiutil \
	$(BOOST_PROGRAM_OPTIONS_LIB) \
	$(BOOST_THREAD_LIB)\
	$(BOOST_SYSTEM_LIB) \
	$(LIBCURL) 

EXTRA_DIST+= test/wdb2tsget/wdb2ts.mk   \
				 test/wdb2tsget/Makefile.am \
				 test/wdb2tsget/Makefile.in

DISTCLEANFILES +=	test/wdb2tsget/Makefile


# Local Makefile Targets
#-----------------------------------------------------------------------------

test/wdb2tsget/all: 

test/wdb2tsget/clean: clean
