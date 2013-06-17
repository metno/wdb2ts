bin_PROGRAMS += wdb2ts_check_config

noinst_LTLIBRARIES += libwdb2tsconfigparser.la 

libwdb2tsconfigparser_la_SOURCES= \
							src/mod_wdb2ts/configparser/Config.h \
							src/mod_wdb2ts/configparser/Config.cc \
							src/mod_wdb2ts/configparser/ConfigParser.h \
							src/mod_wdb2ts/configparser/ConfigParser.cc \
							src/mod_wdb2ts/configparser/RequestConf.h \
							src/mod_wdb2ts/configparser/RequestConf.cc \
							src/mod_wdb2ts/configparser/State.h \
							src/mod_wdb2ts/configparser/State.cc \
						    src/mod_wdb2ts/configparser/NextRun.h \
						    src/mod_wdb2ts/configparser/NextRun.cc

EXTRA_DIST+= src/mod_wdb2ts/configparser/wdb2ts.mk \
			 src/mod_wdb2ts/configparser/Makefile.am \
			 src/mod_wdb2ts/configparser/Makefile.in

DISTCLEANFILES += src/mod_wdb2ts/configparser/Makefile

wdb2ts_check_config_SOURCES= \
	src/mod_wdb2ts/configparser/wdb2ts_check_config.cc 
	
wdb2ts_check_config_LDFLAGS=-L$(top_builddir) -rpath $(libexecdir)
wdb2ts_check_config_LDADD= \
		libwdb2tsconfigparser.la \
	libwdb2ts.la \
	libWciWebQuery.la \
	libXML_locationforecast.la \
	libmiutil.la \
	libwebFW.la  \
	-lgfortran \
	$(LIBPQXX_LIBS) \
	 $(LDFLAGS) 

#                           libWciWebQuery.la	\
#                    	   libXML_locationforecast.la \
#                    	   libwdb2tsconfigparser.la \
#                    	   libwdb2ts.la \
#                   	   libwebFW.la  \
#                    	   libmiutil.la \
#                    	   -lgfortran \
#                    	   $(LDFLAGS) \
#                    	   $(LIBPQXX_LIBS) 



# Local Makefile Targets
#-----------------------------------------------------------------------------

src/mod_wdb2ts/configparser/all: 

src/mod_wdb2ts/configparser/clean: clean
