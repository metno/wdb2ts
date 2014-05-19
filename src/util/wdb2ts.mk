noinst_PROGRAMS += shape2wdb2ts 
#wdb2symbols wdb2cvs 
noinst_LTLIBRARIES += libminimal.la

libminimal_la_SOURCES= \
	src/mod_wdb2ts/ParamDef.cpp src/mod_wdb2ts/ParamDef.h \
	src/mod_wdb2ts/ProviderGroups.cpp src/mod_wdb2ts/ProviderGroups.h \
	src/mod_wdb2ts/ProviderList.cpp src/mod_wdb2ts/ProviderList.h \
	src/mod_wdb2ts/ProviderReftimes.cpp src/mod_wdb2ts/ProviderReftimes.h \
	src/mod_wdb2ts/PointDataHelper.cpp src/mod_wdb2ts/PointDataHelper.h \
	src/mod_wdb2ts/LocationPoint.cpp src/mod_wdb2ts/LocationPoint.h \
	src/mod_wdb2ts/LocationElem.cpp src/mod_wdb2ts/LocationElem.h \
	src/mod_wdb2ts/configdata.cpp src/mod_wdb2ts/configdata.h \
	src/mod_wdb2ts/LocationData.cpp src/mod_wdb2ts/LocationData.h \
	src/mod_wdb2ts/SymbolConf.cpp src/mod_wdb2ts/SymbolConf.h \
	src/mod_wdb2ts/SymbolHolder.cpp src/mod_wdb2ts/SymbolHolder.h \
	src/mod_wdb2ts/ConfigUtils.cpp src/mod_wdb2ts/ConfigUtils.h \
	src/mod_wdb2ts/SymbolGenerator.cpp src/mod_wdb2ts/SymbolGenerator.h \
	src/mod_wdb2ts/probabilityCode.h src/mod_wdb2ts/probabilityCode.cpp \
	src/mod_wdb2ts/ProjectionHelper.h src/mod_wdb2ts/ProjectionHelper.cpp \
	src/mod_wdb2ts/WeatherSymbol.h	src/mod_wdb2ts/WeatherSymbol.cpp
	 
libminimal_la_LDFLAGS=
libminimal_la_LIBADD= 
	
	
shape2wdb2ts_SOURCES = src/util/shape2wdb2ts.cpp 
shape2wdb2ts_LDFLAGS= -lmiutil 

#wdb2cvs_SOURCES = \
#	src/util/wdb2cvs.cpp 
##wdb2cvs_la_LDFLAGS =
#wdb2cvs_LDADD = libtuplecontainer.la libwdb2tsconfigparser.la   libminimal.la libWciWebQuery.la  libmiutil.la

#wdb2symbols_SOURCES = src/util/wdb2symbols.cpp
#wdb2symbols_LDADD = libtuplecontainer.la \
#                    libwdb2tsconfigparser.la \
#                    libWciWebQuery.la \
#                    libXML_locationforecast.la \
#                    libminimal.la \
#                    libwebFW.la \
#                    libmiutil.la \
#                    $(pumet_LIBS)\
#                    $(milib_LIBS)
                    
EXTRA_DIST+= src/util/wdb2ts.mk   \
			 src/util/Makefile.am \
			 src/util/Makefile.in

DISTCLEANFILES +=	src/util/Makefile


# Local Makefile Targets
#-----------------------------------------------------------------------------

src/util/all: 

src/util/clean: clean
