
libetcd_la_SOURCES= \
	src/webFW/UrlQuery.h \
	src/webFW/UrlQuery.cpp \
	src/mod_wdb2ts/LocationPoint.h \
	src/mod_wdb2ts/LocationPoint.cpp \
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
	src/mod_wdb2ts/WciWebQuery/WciWebQuery.h \
	src/mod_wdb2ts/configparser/Config.h \
	src/mod_wdb2ts/configparser/Config.cc \
	src/mod_wdb2ts/configparser/ConfigParser.h \
	src/mod_wdb2ts/configparser/ConfigParser.cc \
	src/mod_wdb2ts/configparser/RequestConf.h \
	src/mod_wdb2ts/configparser/RequestConf.cc \
	src/mod_wdb2ts/configparser/State.h \
	src/mod_wdb2ts/configparser/State.cc \
	src/mod_wdb2ts/configparser/NextRun.h \
	src/mod_wdb2ts/configparser/NextRun.cc \
	src/miutil/Dir.cpp \
	src/miutil/Dir.h \
	src/miutil/File.cpp \
	src/miutil/File.h \
	src/miutil/geterrstr.h \
	src/miutil/geterrstr.cpp \
    src/miutil/etcd.h \
    src/miutil/etcd.cpp \
    src/miutil/trimstr.cpp \
	src/miutil/trimstr.h \
	src/miutil/compresspace.h \
	src/miutil/compresspace.cc \
	src/miutil/mkdir.cpp \
	src/miutil/mkdir.h \
	src/miutil/pathutil.h \
	src/miutil/pathutil.cpp \
	src/miutil/readfile.cpp \
	src/miutil/readfile.h \
	src/miutil/splitstr.cpp \
	src/miutil/splitstr.h \
	src/miutil/Value.h \
	src/miutil/Value.cc \
	src/miutil/SAXParser.h \
	src/miutil/SAXParser.cc \
	src/miutil/replace.h \
	src/miutil/replace.cpp \
	src/miutil/ptimeutil.h \
	src/miutil/ptimeutil.cpp \
	src/mod_wdb2ts/ParamDef.h \
	src/mod_wdb2ts/ParamDef.cpp \
	src/mod_wdb2ts/ProviderGroups.h \
	src/mod_wdb2ts/ProviderGroups.cpp 


if BUILD_WDB2TS_ETCDCLIENT

bin_PROGRAMS+= wdb2ts-etcdcli
#noinst_PROGRAMS+= wdb2ts_etcdclt

noinst_LTLIBRARIES+= libetcd.la

libetcd_la_CPPFLAGS=\
	-g -DNODB=1\
	 -I $(top_srcdir)/src \
	-DWDB2TS_DEFAULT_SYSCONFDIR="\""$(WDB2TS_SYSCONFDIR)"\"" \
	-DWDB2TS_DEFAULT_SYSLOGDIR="\""$(WDB2TS_SYSLOGDIR)"\"" \
	-DWDB2TS_DEFAULT_SYSTMPDIR="\""$(WDB2TS_SYSTMPDIR)"\"" 

libetcd_la_LIBADD=\
	libcetcd.la

#wdb2ts_etcdclt_SOURCES=\
#	src/util/etcd/etcdclt.cpp
#wdb2ts_etcdclt_CPPFLAGS=\
#	-g -DNODB=1 \
#	-I$(top_srcdir)/src
#wdb2ts_etcdclt_LDFLAGS= $(LDFLAGS)
#wdb2ts_etcdclt_LDADD=\
#	libWciWebQuery.la \
#	libetcd.la \
#	$(BOOST_DATETIME_LIB) \
#	$(BOOST_REGEX_LIB) \
#	$(ETCD_LIBS)

wdb2ts_etcdcli_SOURCES=\
	src/mod_wdb2ts/LocationPoint.h \
	src/mod_wdb2ts/LocationPoint.cpp \
	src/util/etcd/wdb2ts_etcdcli.cpp \
	src/util/etcd/opt.h \
	src/util/etcd/opt.cpp
wdb2ts_etcdcli_CPPFLAGS=\
	-g -DNODB=1 \
	-I$(top_srcdir)/src 
wdb2ts_etcdcli_LDFLAGS= $(LDFLAGS)
wdb2ts_etcdcli_LDADD=\
	libetcd.la \
	$(BOOST_DATETIME_LIB) \
	$(BOOST_REGEX_LIB) \
	$(BOOST_SYSTEM_LIB) \
	$(ETCD_LIBS) \
	-lm

endif


EXTRA_DIST+= src/util/etcd/wdb2ts.mk \
			 src/util/etcd/Makefile.am \
			 src/util/etcd/Makefile.in

DISTCLEANFILES +=	src/util/etcd/Makefile


# Local Makefile Targets
#-----------------------------------------------------------------------------

src/util/etcd/all: 

src/util/etcd/clean: clean
                     