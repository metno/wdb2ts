
noinst_LTLIBRARIES+=	libpgconpool.la

libpgconpool_la_SOURCES= \
	src/pgconpool/dbConnectionException.h \
	src/pgconpool/dbSetup.h \
	src/pgconpool/dbConnectionHelper.h \
	src/pgconpool/dbConnection.h \
	src/pgconpool/dbConnection.cc \
	src/pgconpool/dbConnectionPool.h \
	src/pgconpool/dbConnectionPool.cc \
	src/pgconpool/parser.h \
	src/pgconpool/parser.cc 

EXTRA_DIST+= src/pgconpool/wdb2ts.mk \
				 src/pgconpool/Makefile.am \
				 src/pgconpool/Makefile.in

DISTCLEANFILES +=	src/pgconpool/Makefile


# Local Makefile Targets
#-----------------------------------------------------------------------------

src/pgconpool/all: 

src/pgconpool/clean: clean
                     