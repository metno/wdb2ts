
AC_DEFUN([PGCONPOOL_CHECK],
[
PKG_CHECK_MODULES( pgconpool, pgconpool >= 0.3,
[
         AC_SUBST(pgconpool_CFLAGS)
         AC_SUBST(pgconpool_LIBS)
         CPPFLAGS="$pgconpool_CFLAGS $CPPFLAGS"
         LDFLAGS="$pgconpool_LIBS $LDFLAGS"
],
[
         AC_MSG_ERROR(no)
])
])
