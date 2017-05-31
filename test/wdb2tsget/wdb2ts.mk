#-----------------------------------------------------------------------------
# wdb2tsget - performance test program.
#-----------------------------------------------------------------------------
if BUILD_WDB2TSGET
bin_PROGRAMS+= wdb2tsget_test
else
EXTRA_PROGRAMS = wdb2tsget_test
endif

wdb2tsget_test_SOURCES= test/wdb2tsget/wdb2tsget_test.cpp \
                        test/wdb2tsget/HTTPClient.h \
                        test/wdb2tsget/HTTPClient.cpp \
                        test/wdb2tsget/GetThread.h \
                        test/wdb2tsget/GetThread.cpp 
                     
wdb2tsget_test_CPPFLAGS= $(BOOST_CPPFLAGS) $(LIBCURL_CPPFLAGS)
wdb2tsget_test_LDADD= \
	-lwebFW \
	-lmiutil \
	$(BOOST_PROGRAM_OPTIONS_LIB) \
	$(LIBCURL) 

EXTRA_DIST+= test/wdb2tsget/wdb2ts.mk   \
				 test/wdb2tsget/Makefile.am \
				 test/wdb2tsget/Makefile.in

DISTCLEANFILES +=	test/wdb2tsget/Makefile


# Local Makefile Targets
#-----------------------------------------------------------------------------

test/wdb2tsget/all: 

test/wdb2tsget/clean: clean
