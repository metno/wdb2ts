#noinst_PROGRAMS += shape2wdb2ts 
#wdb2cvs
noinst_LTLIBRARIES += libtuplecontainer.la
noinst_PROGRAMS += iterator1

iterator1_SOURCES = src/tuple_container/iterator1.cpp
iterator1_LDADD=libtuplecontainer.la libmiutil.la

libtuplecontainer_la_SOURCES= \
	src/tuple_container/CSV.cpp \
	src/tuple_container/CSV.h \
	src/tuple_container/ITupleContainer.h \
	src/tuple_container/ITupleContainer.cpp \
	src/tuple_container/PqTupleContainer.h \
	src/tuple_container/PqTupleContainer.cpp \
	src/tuple_container/SimpleTupleContainer.h \
	src/tuple_container/SimpleTupleContainer.cpp

libtuplecontainer_la_CFLAGS=$(LIBPQXX_CFLAGS))
libtuplecontainer_la_LDFLAGS=
libtuplecontainer_la_LIBADD= $(LIBPQXX_LIBS) 
	
	
EXTRA_DIST+= src/tuple_container/wdb2ts.mk   \
			 src/tuple_container/Makefile.am \
			 src/tuple_container/Makefile.in

DISTCLEANFILES +=	src/tuple_container/Makefile


# Local Makefile Targets
#-----------------------------------------------------------------------------

src/tuple_container/all: 

src/tuple_container/clean: clean
