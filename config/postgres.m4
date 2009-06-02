#
#   wdb - weather and water data storage
#
#   Copyright (C) 2007 met.no
#   
#   Contact information:
#   Norwegian Meteorological Institute
#   Box 43 Blindern
#   0313 OSLO
#   NORWAY
#   E-mail: wdb@met.no
# 
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, 
#   MA  02110-1301, USA
#
#   Checks for the presence of postgres
#

AC_DEFUN([WDB_POSTGRES_CHECK],
[
# Search for the libpqxx Library
# automatically adds -lpqxx to the LIBS variable
PKG_CHECK_MODULES([LIBPQXX], [libpqxx >= 2.6.8])
AC_SUBST(LIBPQXX_CFLAGS)
AC_SUBST(LIBPQXX_LIBS)
CPPFLAGS="$LIBPQXX_CFLAGS $CPPFLAGS"
LDFLAGS="$LIBPQXX_LIBS $LDFLAGS"


# Set up option
AC_ARG_WITH([pgsql],
		AS_HELP_STRING([--with-pgsql=PATH], 
		[Specify the directory in which postgresql is installed (by default, configure searches your PATH). If set, configure will search PATH/bin for pg_config]),
		[PGSQL_CONFIG="${with_pgsql}/bin/pg_config"])

# Run PG_CONFIG
AC_PATH_PROG(PGSQL_CONFIG,pg_config)
if test -f "${PGSQL_CONFIG}"; then
	CPPFLAGS="$CPPFLAGS -I`${PGSQL_CONFIG} --includedir`"
	LDFLAGS="$LDFLAGS -L`${PGSQL_CONFIG} --libdir`"
else
	AC_MSG_ERROR([
-------------------------------------------------------------------------
   Unable to find pg_config. If Postgres is installed, make sure 
   pg_config is in your PATH, or specify the path in which postgres 
   is installed with --with-pgsql=PATH
-------------------------------------------------------------------------
])
fi


# Search for the libpq Library
# automatically adds -lpq to the LIBS variable
AC_SEARCH_LIBS(PQexec, 
	       	[pq],	 
	       	,
		[
		 AC_MSG_ERROR([
-------------------------------------------------------------------------
    Unable to link with libpq. If the library is installed, make sure 
    -L(PGSQL_PATH)/lib is in your LDFLAGS, or specify the path in which 
    postgres is installed with --with-pgsql=PATH
-------------------------------------------------------------------------
])
		]
)

])
