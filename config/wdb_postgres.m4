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

# Set Postgres Embedded SQL Compiler
#AC_SUBST(EC)
#AC_PATH_PROG(EC, [ecpg])

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

# Search for the libepcg Library
# automatically adds -lepcg to the LIBS variable
#AC_SEARCH_LIBS(ECPGdebug, 
#	       	[ecpg],	 
#	       	,
#		[
#		 AC_MSG_ERROR([------------------------------------------------------------])
#		 AC_MSG_ERROR([Error:])
#		 AC_MSG_ERROR([Unable to link with libecpg. If the library is installed, make sure -L(PGSQL_PATH)/lib is in your #LDFLAGS, or specify the path in which postgres is installed with --with-pgsql=PATH])
#		 AC_MSG_ERROR([------------------------------------------------------------])
#		]
#)

# Search for the libpqxx Library
# automatically adds -lpqxx to the LIBS variable
PKG_CHECK_MODULES([LIBPQXX], [libpqxx >= 2.6.8])
AC_SUBST(LIBPQXX_CFLAGS)
AC_SUBST(LIBPQXX_LIBS)
CPPFLAGS="$LIBPQXX_CFLAGS $CPPFLAGS"
LDFLAGS="$LIBPQXX_LIBS $LDFLAGS"

# Looking for PostGis
#AC_ARG_WITH([postgis],
#	    AS_HELP_STRING([--with-postgis=PATH], 
#	    [Specify the directory in which postgis is installed (default is the postgresql contrib directory)]),
#	    [postgis_dir="${with_postgis}"],
#	    [postgis_dir=`${PGSQL_CONFIG} --sharedir`]
#)
#
#AC_MSG_CHECKING(for postgis)
#if test ! -f ${postgis_dir}/lwpostgis.sql; then
#	postgis_dir=${postgis_dir}/../
#	if test ! -f ${postgis_dir}/lwpostgis.sql; then
#		AC_MSG_RESULT(no)
#		AC_MSG_ERROR([
#-------------------------------------------------------------------------
#    Cannot find postgis SQL files. Ensure that these files are 
#    installed in the share directory of your PostgreSQL installation, 
#    or explicitly specify its location using the --with-postgis=PATH 
#    option.
#-------------------------------------------------------------------------
#])
#	fi
#fi
#AC_MSG_RESULT(yes)
#
#AC_SUBST(postgis_dir)
#
])
