#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
#
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# Pre-Conf
#-----------------------------------------------------------------------------
AC_PREREQ(2.59)
AC_INIT(metno-wdb2ts, 0.5-dev, wdb@met.no)

# Config
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_SRCDIR([wdb2ts_config.h.in])
AC_CONFIG_HEADER([wdb2ts_config.h])


#-----------------------------------------------------------------------------
# Source & Build Directories
#-----------------------------------------------------------------------------

# Source Directories
# - All directories that need to be linked into AM_FLAGS
src_subdirs="	src/mod_wdb2ts \
					src/mod_wdb2ts/WciWebQuery \
					src/mod_wdb2ts/contenthandlers/LocationForecast/XML_locationforecast \
					src/miutil \
					src/mod_wdb2ts/configparser \
					src/webFW "

#					src/mod_wdb2ts/configparser \
						
test_subdirs="	test/unit \
					src/mod_wdb2ts/test \
					src/webFW/test \
					src/miutil/test \
					test/wdb2tsget "
												

# Build Directories
# - All directories that building can occur in
build_subdirs="	src/mod_wdb2ts/WciWebQuery \
						src/mod_wdb2ts/contenthandlers/LocationForecast/XML_locationforecast \
						src/mod_wdb2ts \
						src/mod_wdb2ts/configparser \
						src/miutil \
						src/webFW \
						src/mod_wdb2ts/test \
						src/webFW/test \
						src/miutil/test \
						test/wdb2tsget "

#						src/mod_wdb2ts/configparser \
#-----------------------------------------------------------------------------
# Programs
#-----------------------------------------------------------------------------
AC_PROG_LIBTOOL
AC_PROG_CXX
AC_PROG_CC
AM_PROG_CC_C_O
AC_LANG(C++)

# topography
TOPOGRAPHY_INPUT

# Apache apxs
AC_PROG_APXS

# wdb2ts-configdir
WITH_WDB2TS_CONFIG_DIR

# wdb2ts-logdir
WITH_WDB2TS_LOG_DIR


WDB2TS_MAP_FILE='${pkgdatadir}/topography100.asc'
AC_SUBST(WDB2TS_MAP_FILE)

#-----------------------------------------------------------------------------
# Checks
#-----------------------------------------------------------------------------

# Header Files
AC_HEADER_STDC
AC_HEADER_DIRENT
AC_CHECK_HEADERS([stdlib.h string.h unistd.h], ,AC_MSG_ERROR("missing needed header file"))

# Typedefs, structures, and compiler characteristics
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_STRUCT_TM

# Library functions
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_ERROR_AT_LINE
AC_FUNC_STAT
AC_CHECK_FUNCS([floor getcwd regcmp regcomp])

# STL
AC_CXX_REQUIRE_STL

#-----------------------------------------------------------------------------
# Module Checks
#-----------------------------------------------------------------------------

#POSTGRES
WDB_POSTGRES_CHECK

#BOOST
WDB_BOOST_CHECK

#pgconpool
PGCONPOOL_CHECK

#proj 
WDB_PROJ_CHECK

#libcurl
LIBCURL_CHECK_CONFIG([no],[7.15.5],[want_libcurl=yes],[want_libcurl=no])
AM_CONDITIONAL([BUILD_WDB2TSGET],[test x$want_libcurl = xyes])



#libxml
PKG_CHECK_MODULES([libxml2], [libxml-2.0 >= 2.6.0])
AC_SUBST(libxml2_CFLAGS)
AC_SUBST(libxml2_LIBS)
CPPFLAGS="$libxml2_CFLAGS $CPPFLAGS"
LDFLAGS="$libxml2_LIBS $LDFLAGS"

PKG_CHECK_MODULES(pumet, pumet)
CPPFLAGS="$pumet_CFLAGS $CPPFLAGS"
LDFLAGS="$pumet_LIBS $LDFLAGS"

PKG_CHECK_MODULES(putools, putools)
CPPFLAGS="$putools_CFLAGS $CPPFLAGS"
LDFLAGS="$putools_LIBS $LDFLAGS"

PKG_CHECK_MODULES(milib, milib)
CPPFLAGS="$milib_CFLAGS $CPPFLAGS"
LDFLAGS="$milib_LIBS $LDFLAGS"

PKG_CHECK_MODULES([log4cpp], [log4cpp >= 0.3])
CPPFLAGS="$log4cpp_CFLAGS $CPPFLAGS"
LDFLAGS="$log4cpp_LIBS $LDFLAGS"


# CppUnit
AM_PATH_CPPUNIT(1.12.0,, [echo WARNING: CppUnit not found - you will not be able to run all tests])
AC_MSG_CHECKING(for CPPUNIT_CFLAGS)
AM_CONDITIONAL(HAS_CPPUNIT,test -n $CPPUNIT_CFLAGS)

# DOCBOOK
WDB_DOCBOOK_CHECK

# Doxygen Support
DX_HTML_FEATURE(ON)
DX_CHM_FEATURE(OFF)
DX_CHI_FEATURE(OFF)
DX_MAN_FEATURE(OFF)
DX_RTF_FEATURE(OFF)
DX_XML_FEATURE(OFF)
DX_PDF_FEATURE(OFF)
DX_PS_FEATURE(OFF)
#DX_INIT_DOXYGEN(metno-wdb2ts, doc/doxygen/doxygen.cfg)
#
# Todo: There is a problem with the Doxygen setup for wdb2ts for Autoconf 2.6.1

#-----------------------------------------------------------------------------
# Setup CPP linking
#-----------------------------------------------------------------------------

# Link to Source Directories (header files)
for srcdir_tmp in $src_subdirs; do
	CPPFLAGS="$CPPFLAGS -I$srcdir/$srcdir_tmp"
done

# Link to Source Directories (tests)
for testdir_tmp in $test_subdirs; do
	CPPFLAGS="$CPPFLAGS -I$srcdir/$testdir_tmp"
done

#-----------------------------------------------------------------------------
# Automake
#-----------------------------------------------------------------------------
AM_INIT_AUTOMAKE(-Wall -Werror subdir-objects tar-ustar)


#-----------------------------------------------------------------------------
# Configure and Exit
#-----------------------------------------------------------------------------

AC_CONFIG_FILES([Makefile \
                 src/miutil/Makefile \
                 src/webFW/Makefile \
                 src/mod_wdb2ts/Makefile \
                 src/mod_wdb2ts/WciWebQuery/Makefile \
                 src/mod_wdb2ts/contenthandlers/LocationForecast/XML_locationforecast/Makefile \
                 src/mod_wdb2ts/configparser/Makefile \
                 test/unit/Makefile \
                 test/wdb2tsget/Makefile \
                 src/webFW/test/Makefile \
                 src/mod_wdb2ts/test/Makefile \
                 etc/metno-wdb2ts.conf.template \
                 etc/metno-wdb2ts-db.conf.template \
                 etc/metno-wdb2ts-conf.xml.template \
                 etc/qbSymbols.def])

#                 src/mod_wdb2ts/configparser/Makefile \
AC_OUTPUT

# Print Out Summary of Configuration
AC_MSG_RESULT([ ])
AC_MSG_RESULT([$PACKAGE_NAME $PACKAGE_VERSION])
AC_MSG_RESULT([Configuration:])
AC_MSG_RESULT([-------------------------------------------------------------------------])
AC_MSG_RESULT([ ])
AC_MSG_RESULT([ Source Code:      $srcdir])
AC_MSG_RESULT([ Host System:      $host])
AC_MSG_RESULT([  ])
AC_MSG_RESULT([ Prefix:           $prefix])
AC_MSG_RESULT([ Binaries:         $exec_prefix])
AC_MSG_RESULT([ Manuals:          $mandir])
AC_MSG_RESULT([ Data:             $datadir])
AC_MSG_RESULT([ System Config:    $sysconfdir])
AC_MSG_RESULT([ ])
AC_MSG_RESULT([ CPPFLAGS:         $CPPFLAGS])
AC_MSG_RESULT([ CXXFLAGS:         $CXXFLAGS])
AC_MSG_RESULT([ LDFLAGS:          $LDFLAGS])
AC_MSG_RESULT([ LIBS:             $LIBS])
AC_MSG_RESULT([ WDB2TS_MAP_FILE:  $WDB2TS_MAP_FILE])
AC_MSG_RESULT([  ])
AC_MSG_RESULT([-------------------------------------------------------------------------])
AC_MSG_RESULT([ ])
