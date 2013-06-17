#-----------------------------------------------------------------------------
# gribWrite unit tests
#-----------------------------------------------------------------------------

TESTS +=   					wdb2TsUnitTest

check_PROGRAMS +=       	wdb2TsUnitTest

wdb2TsUnitTest_SOURCES = 	test/unit/wdb2TsUnitTest.cpp

wdb2TsUnitTest_CPPFLAGS = 	$(CPPUNIT_CFLAGS)  \
                         -DTESTDIR_MOD_WDB2TS="\""$(top_srcdir)/src/mod_wdb2ts/test"\"" 
                          

#-I$(srcdir)/test/utility/configuration

wdb2TsUnitTest_LDFLAGS = 	$(AM_LDFLAGS) -L. -L$(top_buildir)/.libs

wdb2TsUnitTest_LDADD = 		$(CPPUNIT_LIBS) \
                            -lwebFW \
                            -lwdb2tsconfigparser \
                            -lwdb2ts \
                            -lWciWebQuery \
                            -lXML_locationforecast \
                            -lmiutil \
                            $(pgconpool_LIBS) \
                            $(putools_LIBS) \
                            $(pumet_LIBS) \
                            $(milib_LIBS) \
                            $(LIBPQXX_LIBS) \
                            -lgfortran
                             
# -ltestConfiguration -lwdbConfig

EXTRA_DIST +=   			test/unit/wdb2ts.mk \
							test/unit/Makefile.am \
							test/unit/Makefile.in

DISTCLEANFILES +=       	test/unit/Makefile
