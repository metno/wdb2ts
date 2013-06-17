noinst_PROGRAMS+= shape2wdb2ts

shape2wdb2ts_SOURCES = src/util/shape2wdb2ts.cpp 
					   
					   
shape2wdb2ts_LDFLAGS= -lmiutil 
				

EXTRA_DIST+= src/util/wdb2ts.mk   \
			 src/util/Makefile.am \
			 src/util/Makefile.in

DISTCLEANFILES +=	src/util/Makefile


# Local Makefile Targets
#-----------------------------------------------------------------------------

src/util/all: 

src/util/clean: clean
