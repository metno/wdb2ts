noinst_LTLIBRARIES+= libcetcd.la

libcetcd_la_SOURCES= \
	src/cetcd/cetcd-master/sds/sds.c \
	src/cetcd/cetcd-master/sds/sds.h \
	src/cetcd/cetcd-master/cetcd_array.c \
	src/cetcd/cetcd-master/cetcd_array.h \
	src/cetcd/cetcd-master/cetcd_json_parser.h \
	src/cetcd/cetcd-master/cetcd.c \
	src/cetcd/cetcd-master/cetcd.h 

libcetcd_la_CPPFLAGS=\
	-Wall -Wextra -fPIC $(ETCD_CPPFLAGS)
libcetcd_la_LIBADD=\
	$(ETCD_LIBS)
	
EXTRA_DIST+=\
	src/cetcd/cetcd-master/LICENSE \
	src/cetcd/cetcd-master/sds/LICENSE \
	src/cetcd/cetcd-master-20170923.zip \
	src/cetcd/README \
	src/cetcd/wdb2ts.mk \
	src/cetcd/Makefile.am \
	src/cetcd/Makefile.in

DISTCLEANFILES += src/cetcd/Makefile


# Local Makefile Targets
#-----------------------------------------------------------------------------

src/statd/all: 

src/statd/clean: clean
                     