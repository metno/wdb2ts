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
                     
<<<<<<< 7b89fa2bbdc26c86b1a70ecd62f075799dfe49a5
wdb2tsget_test_CPPFLAGS= $(BOOST_CPPFLAGS) $(LIBCURL_CPPFLAGS)
wdb2tsget_test_LDADD= \
	-lwebFW \
	-lmiutil \
	$(BOOST_PROGRAM_OPTIONS_LIB) \
	$(BOOST_THREAD_LIB)\
	$(LIBCURL) 
=======
wdb2tsget_test_CPPFLAGS= $(LIBCURL_CPPFLAGS)
wdb2tsget_test_LDADD= -lwebFW -lmiutil \
	$(BOOST_PROGRAM_OPTIONS_LIB) \
	$(BOOST_THREAD_LIB) \
	$(BOOST_SYSTEM_LIB) \
	 $(LIBCURL) 
<<<<<<< 1e05931c678b9b4c61912417cb1786ee4b825a35
>>>>>>>   - include the sources for pgconpool into the wdb2ts repository.
=======
>>>>>>>   - include the sources for pgconpool into the wdb2ts repository.

EXTRA_DIST+= test/wdb2tsget/wdb2ts.mk   \
				 test/wdb2tsget/Makefile.am \
				 test/wdb2tsget/Makefile.in

DISTCLEANFILES +=	test/wdb2tsget/Makefile


# Local Makefile Targets
#-----------------------------------------------------------------------------

test/wdb2tsget/all: 

test/wdb2tsget/clean: clean
