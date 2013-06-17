#-----------------------------------------------------------------------------
# mod_wdb2ts Unit Test Framework
#
# See: <top>/test/unit
#-----------------------------------------------------------------------------

wdb2TsUnitTest_SOURCES += 	src/mod_wdb2ts/test/UrlParamTest.cpp \
							src/mod_wdb2ts/test/UrlParamTest.h \
							src/mod_wdb2ts/test/QueryMakerTest.cpp \
							src/mod_wdb2ts/test/QueryMakerTest.h \
							src/mod_wdb2ts/test/WciWebQueryTest.cpp \
                            src/mod_wdb2ts/test/WciWebQueryTest.h

EXTRA_DIST +=				src/mod_wdb2ts/test/wdb2ts.mk \
							src/mod_wdb2ts/test/Makefile.am \
							src/mod_wdb2ts/test/Makefile.in

DISTCLEANFILES +=			src/mod_wdb2ts/test/Makefile
