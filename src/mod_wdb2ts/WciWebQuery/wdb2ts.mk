noinst_LTLIBRARIES += libWciWebQuery.la 

libWciWebQuery_la_SOURCES= \
							src/mod_wdb2ts/WciWebQuery/UrlParam.cpp \
	                  src/mod_wdb2ts/WciWebQuery/UrlParam.h \
	                  src/mod_wdb2ts/WciWebQuery/UrlParamString.cpp \
	                  src/mod_wdb2ts/WciWebQuery/UrlParamString.h \
	                  src/mod_wdb2ts/WciWebQuery/UrlParamFloat.cpp \
	                  src/mod_wdb2ts/WciWebQuery/UrlParamFloat.h \
	                  src/mod_wdb2ts/WciWebQuery/UrlParamInteger.cpp \
	                  src/mod_wdb2ts/WciWebQuery/UrlParamInteger.h \
	                  src/mod_wdb2ts/WciWebQuery/UrlParamDataProvider.cpp \
	                  src/mod_wdb2ts/WciWebQuery/UrlParamDataProvider.h \
	                  src/mod_wdb2ts/WciWebQuery/UrlParamTimeSpec.cpp \
	                  src/mod_wdb2ts/WciWebQuery/UrlParamTimeSpec.h \
	                  src/mod_wdb2ts/WciWebQuery/UrlParamParameter.cpp \
	                  src/mod_wdb2ts/WciWebQuery/UrlParamParameter.h \
	                  src/mod_wdb2ts/WciWebQuery/UrlParamLevelSpec.cpp \
	                  src/mod_wdb2ts/WciWebQuery/UrlParamLevelSpec.h \
	                  src/mod_wdb2ts/WciWebQuery/UrlParamFormat.cpp \
	                  src/mod_wdb2ts/WciWebQuery/UrlParamFormat.h \
	                  src/mod_wdb2ts/WciWebQuery/UrlParamPolygon.h \
	                  src/mod_wdb2ts/WciWebQuery/UrlParamPolygon.cpp \
	                  src/mod_wdb2ts/WciWebQuery/WciWebQuery.cpp \
	                  src/mod_wdb2ts/WciWebQuery/WciWebQuery.h 
	                                        



EXTRA_DIST+= src/mod_wdb2ts/WciWebQuery/wdb2ts.mk \
				 src/mod_wdb2ts/WciWebQuery/Makefile.am \
				 src/mod_wdb2ts/WciWebQuery/Makefile.in

DISTCLEANFILES +=	src/mod_wdb2ts/WciWebQuery/Makefile

# Local Makefile Targets
#-----------------------------------------------------------------------------

src/mod_wdb2ts/WciWebQuery/all: 

src/mod_wdb2ts/WciWebQuery/clean: clean
