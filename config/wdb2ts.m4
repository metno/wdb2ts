AC_DEFUN([WITH_WDB2TS_CONFIG_DIR],
[AC_CACHE_CHECK(wdb2ts configdir location,
                WDB2TS_CONFIGDIR, 
                [AC_ARG_WITH(wdb2ts-configdir,
                             AC_HELP_STRING([--with-wdb2ts-configdir],
                                            [location of wdb2ts config dir (default apxs -q SYSCONFDIR)]),
                             WDB2TS_CONFIGDIR=$withval,
                             WDB2TS_CONFIGDIR=""
                            )
                ]
               )


AC_SUBST(WDB2TS_CONFIGDIR)
                                                  
AM_CONDITIONAL([HAS_WDB2TS_CONFIGDIR], [test  x$WDB2TS_CONFIGDIR != x ] )

])

AC_DEFUN([WITH_WDB2TS_LOG_DIR],
[AC_CACHE_CHECK(wdb2ts logdir location,
                WDB2TS_LOGDIR, 
                [AC_ARG_WITH(wdb2ts-logdir,
                             AC_HELP_STRING([--with-wdb2ts-logdir],
                                            [Default location to write wdb2ts logfiles (default apxs -q LOCALSTATEDIR/log/wdb2ts)]),
                             WDB2TS_LOGDIR=$withval,
                             WDB2TS_LOGDIR=""
                            )
                ]
               )

AC_SUBST(WDB2TS_LOGDIR)
AM_CONDITIONAL([HAS_WDB2TS_LOGDIR], [test  x$WDB2TS_LOGDIR != x ] )

])

AC_DEFUN([WITH_WDB2TS_TMP_DIR],
[AC_CACHE_CHECK(wdb2ts tmpdir location,
                WDB2TS_TMPDIR, 
                [AC_ARG_WITH(wdb2ts-tmpdir,
                             AC_HELP_STRING([--with-wdb2ts-tmpdir],
                                            [Default location to write wdb2ts temporary files (default /tmp).]),
                             WDB2TS_TMPDIR=$withval,
                             WDB2TS_TMPDIR=""
                            )
                ]
               )

AC_SUBST(WDB2TS_TMPDIR)
AM_CONDITIONAL([HAS_WDB2TS_TMPDIR], [test  x$WDB2TS_TMPDIR != x ] )

])

                                                  


