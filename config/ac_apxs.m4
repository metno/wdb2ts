dnl
dnl Copyright (C) 2008 - Borge Moe
dnl
dnl This program is free software; you can redistribute it and/or modify it
dnl under the terms of the GNU General Public Licence as published by the Free
dnl Software Foundation; either version 2 of the Licence, or (at your option)
dnl any later version.
dnl
dnl This program is distributed in the hope that it will be useful, but WITHOUT
dnl ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
dnl FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public Licence for
dnl more details.
dnl
dnl You should have received a copy of the GNU General Public Licence along
dnl with this program; if not, write to the Free Software Foundation, Inc., 59
dnl Temple Place, Suite 330, Boston, MA 02111-1307 USA
dnl



dnl Find APXS
AC_DEFUN([AC_PROG_APXS],[
AC_ARG_WITH([apxs],
			 AC_HELP_STRING([--with-apxs], [location of APache eXtenSion tool (APXS)]),
            [AC_PATH_PROG([apxs],
            			  ["`basename $withval`"],,
                          ["`AS_DIRNAME($withval)`"])
            ],
            [AC_PATH_PROGS([apxs],
            			   [apxs2 apxs],,
                           [/usr/sbin:/usr/local/apache2/bin:$PATH])
            ])

if test "x$apxs" = 'x'; then
	AC_MSG_ERROR([apxs missing])
fi

AC_ARG_WITH([apache-moduledir],
			 AC_HELP_STRING([--with-apache-moduledir=PATH], [Apache Module directory; path in which to install apache modules. Overrides APXS.]),
            [APACHE_MODULEDIR="$withval"],
            [APACHE_MODULEDIR="`$apxs -q LIBEXECDIR`"]
           )


APACHE_SYSCONFDIR="`$apxs -q SYSCONFDIR`"
APACHE_SYSLOGDIR="`$apxs -q LOCALSTATEDIR`/log/wdb2ts"
CXXFLAGS="`$apxs -q CFLAGS` $CFLAGS"
CPPFLAGS="-I. -I`$apxs -q INCLUDEDIR`"
LDXXFLAGS="$LDXXFLAGS -L`$apxs -q LIBDIR`"

#apr flags
PKG_CHECK_MODULES([apr1], [apr-1])
CPPFLAGS="$apr1_CFLAGS $CPPFLAGS"
LDFLAGS="$apr1_LIBS $LDFLAGS"

AC_SUBST(APACHE_MODULEDIR)
AC_SUBST(APACHE_SYSCONFDIR)
AC_SUBST(APACHE_SYSLOGDIR)
])
