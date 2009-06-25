#-----------------------------------------------------------------------------
# gribWrite unit tests
#-----------------------------------------------------------------------------

TESTS +=   					wdb2TsUnitTest

check_PROGRAMS +=       	wdb2TsUnitTest

wdb2TsUnitTest_SOURCES = 	test/unit/wdb2TsUnitTest.cpp

wdb2TsUnitTest_CPPFLAGS = 	$(CPPUNIT_CFLAGS) 
#-I$(srcdir)/test/utility/configuration

wdb2TsUnitTest_LDFLAGS = 	$(AM_LDFLAGS) -L.

wdb2TsUnitTest_LDADD = 		$(CPPUNIT_LIBS) -lwebFW -lwdb2ts -lWciWebQuery -lmiutil
# -ltestConfiguration -lwdbConfig

EXTRA_DIST +=   			test/unit/wdb2ts.mk \
							test/unit/Makefile.am \
							test/unit/Makefile.in

DISTCLEANFILES +=       	test/unit/Makefile
