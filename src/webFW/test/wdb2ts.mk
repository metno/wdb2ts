#-----------------------------------------------------------------------------
# webFW Unit Test Framework
#
# See: <top>/test/unit
#-----------------------------------------------------------------------------

wdb2TsUnitTest_SOURCES += 	src/webFW/test/RequestHandlerManagerTest.cpp \
							src/webFW/test/RequestHandlerManagerTest.h \
							src/webFW/test/TestApp.cpp \
							src/webFW/test/TestApp.h \
							src/webFW/test/TestRequestHandler.h \
							src/webFW/test/UrlQueryTest.h \
							src/webFW/test/UrlQueryTest.cpp

EXTRA_DIST +=				src/webFW/test/wdb2ts.mk \
							src/webFW/test/Makefile.am \
							src/webFW/test/Makefile.in

DISTCLEANFILES +=			src/webFW/test/Makefile
