
noinst_LTLIBRARIES+=		libstatd.la

libstatd_la_SOURCES= \
	src/statd/statsd_client.cpp \
	src/statd/statsd_client.h
	
EXTRA_DIST+= src/statd/wdb2ts.mk \
				 src/statd/Makefile.am \
				 src/statd/Makefile.in

DISTCLEANFILES +=	src/statd/Makefile


# Local Makefile Targets
#-----------------------------------------------------------------------------

src/statd/all: 

src/statd/clean: clean
                     