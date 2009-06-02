#-----------------------------------------------------------------------------
# webFW Unit Test Framework
#
# See: <top>/test/unit
#-----------------------------------------------------------------------------

wdb2TsUnitTest_SOURCES += 	src/miutil/test/ptimeutilTest.cpp \
							      src/miutil/test/ptimeutilTest.h 

EXTRA_DIST +=	src/webFW/test/wdb2ts.mk \
					src/webFW/test/Makefile.am \
					src/webFW/test/Makefile.in

DISTCLEANFILES +=	src/miutil/test/Makefile
