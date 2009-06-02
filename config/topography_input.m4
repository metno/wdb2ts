AC_DEFUN([TOPOGRAPHY_INPUT],
[AC_ARG_WITH([topography],
[AS_HELP_STRING([--with-topography=TOPOGRAPHY-FILE], [Specify a topography file to use])],
[
	TOPOGRAPHY_FILE=$withval
	AC_SUBST(TOPOGRAPHY_FILE)
],
[
AC_MSG_WARN([Since you have not specified a topography file, you will not be able to do altitude corrections to temperature])
]
)
AM_CONDITIONAL([HAS_TOPOGRAPHY_FILE], [test "$TOPOGRAPHY_FILE"] )
])
