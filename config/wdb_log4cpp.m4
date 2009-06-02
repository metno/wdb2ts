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
#   Checks for the presence of log4cpp
#

AC_DEFUN([WDB_LOG4CPP_CHECK],
[
AC_LANG([C++])
# log4cpp settings
AC_ARG_WITH(log4cpp,
            AC_HELP_STRING([--with-log4cpp=PATH], [Specify the directory in which log4cpp-config is installed (by default, configure searches your PATH). If set, configure will search PATH/bin for log4cpp-config]),
            [LOG4CPP_CONFIG="${with_log4cpp}/bin/log4cpp-config"])
AC_PATH_PROG(LOG4CPP_CONFIG,log4cpp-config)

if test -f "${LOG4CPP_CONFIG}"; then
	CPPFLAGS="$CPPFLAGS `${LOG4CPP_CONFIG} --cflags`"
	LDFLAGS="$LDFLAGS -L`${LOG4CPP_CONFIG} --libdir`"
    	# Check for the presence of -lnsl
	AC_SEARCH_LIBS(getservbyname, 
		[nsl],,
		[AC_MSG_ERROR([
-------------------------------------------------------------------------
    Unable to locate libnsl. libnsl is the name services library and is 
    utilized by log4cpp. Ensure that libnsl is located in your LDFLAGS 
    or pointed to by your LD_LIBRARY_PATH and try again.

    LDFLAGS: $LDFLAGS
-------------------------------------------------------------------------
])
	])
	# Add log4cpp
	LIBS="-llog4cpp $LIBS"
else
	AC_MSG_ERROR([
-------------------------------------------------------------------------
    Unable to find log4cpp-config. If the library is installed, make 
    sure log4cpp-config is in your PATH, or specify a path to the 
    executable using --with-log4cpp=PATH
-------------------------------------------------------------------------
])
fi

# Check header files
AC_CHECK_HEADER([log4cpp/Category.hh],
		,
		[
		AC_MSG_ERROR([
-------------------------------------------------------------------------
    Unable to find log4cpp header file Category.hh. Verify that 
    log4cpp is installed correctly
-------------------------------------------------------------------------
])
		]
)

AC_CHECK_HEADER([log4cpp/PropertyConfigurator.hh],
		,
		[
		AC_MSG_ERROR([
------------------------------------------------------------------------
    Unable to verify that this is the correct version of log4cpp. You 
    need at least version 0.3.x
-------------------------------------------------------------------------
])
		]
)

AC_LANG_PUSH([C++])
AC_TRY_LINK(
	    [#include <log4cpp/Category.hh>],
	    [log4cpp::Category::getInstance("foo")],
	    [AC_DEFINE(HAVE_LIBLOG4CPP,1,[Define if log4cpp library was found])],
	    [
		AC_MSG_ERROR([
-------------------------------------------------------------------------
    Found the header files. Trying to link with log4cpp and failing. 
    If the library is installed, make sure log4cpp-config is in your 
    PATH, or specify a path to the executable using --with-log4cpp=PATH
-------------------------------------------------------------------------
])
	    ]
	   )
AC_LANG_POP([C++])

])

