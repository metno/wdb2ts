#-----------------------------------------------------------------------------
# webFW Unit Test Framework
#
# See: <top>/test/unit
#-----------------------------------------------------------------------------

wdb2TsUnitTest_SOURCES += src/mod_wdb2ts/configparser/test/configTest.cpp \
						  src/mod_wdb2ts/configparser/test/configTest.h 

EXTRA_DIST += src/mod_wdb2ts/configparser/test/wdb2ts.mk \
			  src/mod_wdb2ts/configparser/test/Makefile.am \
			  src/mod_wdb2ts/configparser/test/Makefile.in

DISTCLEANFILES += src/mod_wdb2ts/configparser/test/Makefile
