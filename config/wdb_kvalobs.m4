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

AC_DEFUN([WDB_KVALOBS_CHECK],
[

AC_ARG_WITH([kvalobsLoad],
	    AS_HELP_STRING([--with-kvalobsLoad], [Specify whether or not the kvalobsLoad daemon should be compiled. Note that kvalobsLoad requires access to the kvalobs libraries. The default is that kvalobsLoad is NOT compiled.]),
     	    [kvLoad=true],
	    [kvLoad=false]
)

AC_MSG_CHECKING(whether kvalobsLoad should be compiled)

if test x$kvLoad = xtrue ; then
	AC_MSG_RESULT(yes)
	PKG_CHECK_MODULES([kvalobs], [kvcpp >= 1.0],
    	[
		AC_SUBST(kvalobs_CFLAGS)
		AC_SUBST(kvalobs_LIBS)
		AC_MSG_RESULT(yes)
	],
	[
		AC_MSG_RESULT(no)
		AC_MSG_ERROR([
-------------------------------------------------------------------------
    Could not find kvalobs through PKG_CONFIG. Will not be able to 
    compile kvalobs related programs or libraries.
-------------------------------------------------------------------------
])
	])

else
	AC_MSG_RESULT(no)
fi

AM_CONDITIONAL([KVALOBS_LOAD], [test x$debug = xtrue])

])

