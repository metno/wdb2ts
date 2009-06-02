#-----------------------------------------------------------------------------
# Project Makefile for WDB2TS
#
# This file is processed by automake to generate the non-recursive
# project Makefile
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# Global Settings
#-----------------------------------------------------------------------------

# Scanned by Autoreconf; must be static (no expansion)
ACLOCAL_AMFLAGS = 		-I config

AM_LDFLAGS =			-L@builddir@


#-----------------------------------------------------------------------------
# WDB2TS Configuration
#-----------------------------------------------------------------------------

noinst_PROGRAMS=
bin_PROGRAMS=
lib_LTLIBRARIES=
noinst_LTLIBRARIES=
TESTS=
check_PROGRAMS=
CLEANFILES=
MOSTLYCLEANFILES=
DISTCLEANFILES=
EXTRA_DIST=				etc/metno-wdb2ts.conf.template.in \
			           	etc/metno-wdb2ts-db.conf.template.in \
			           	etc/metno-wdb2ts-conf.xml.template.in \
			           	etc/qbSymbols.def.in \
			           	README.debian

if HAS_WDB2TS_CONFIGDIR
   WDB2TS_SYSCONFDIR=$(WDB2TS_CONFIGDIR)
else
   WDB2TS_SYSCONFDIR=$(APACHE_SYSCONFDIR)
endif

if HAS_WDB2TS_LOGDIR
   WDB2TS_SYSLOGDIR=$(WDB2TS_LOGDIR)
else
   WDB2TS_SYSLOGDIR=$(APACHE_SYSLOGDIR)
endif


WDB2TS_LOCALSTATEDIR=$(localstatedir)/lib/metno-wdb2ts

AM_CXXFLAGS = -DWDB2TS_DEFAULT_SYSCONFDIR="\""$(WDB2TS_SYSCONFDIR)"\"" \
				  -DWDB2TS_DEFAULT_SYSLOGDIR="\""$(WDB2TS_SYSLOGDIR)"\"" \
              -DWDB2TS_MAP_FILE="\""$(WDB2TS_MAP_FILE)"\"" \
              -DWDB2TS_LOCALSTATEDIR="\""$(WDB2TS_LOCALSTATEDIR)"\""  

 
#-----------------------------------------------------------------------------
# Source Makefiles
#-----------------------------------------------------------------------------

include src/miutil/wdb2ts.mk
include src/webFW/wdb2ts.mk
include src/mod_wdb2ts/WciWebQuery/wdb2ts.mk
include src/mod_wdb2ts/contenthandlers/LocationForecast/XML_locationforecast/wdb2ts.mk
include src/mod_wdb2ts/configparser/wdb2ts.mk
include src/mod_wdb2ts/wdb2ts.mk

#-----------------------------------------------------------------------------
# Source Makefiles
#-----------------------------------------------------------------------------

include test/unit/wdb2ts.mk
include test/wdb2tsget/wdb2ts.mk
include src/mod_wdb2ts/test/wdb2ts.mk
include src/webFW/test/wdb2ts.mk
include src/miutil/test/wdb2ts.mk

#-----------------------------------------------------------------------------
# Installation Hooks
#-----------------------------------------------------------------------------

install-data-hook:
	install -d $(DESTDIR)/$(WDB2TS_SYSCONFDIR)
	install -d $(DESTDIR)/$(WDB2TS_SYSLOGDIR)
	install -d $(DESTDIR)/$(pkgdatadir)
	install -d $(DESTDIR)/$(WDB2TS_LOCALSTATEDIR)
	$(INSTALL) -m644 etc/metno-wdb2ts.conf.template 		$(DESTDIR)/$(WDB2TS_SYSCONFDIR)
	$(INSTALL) -m644 etc/metno-wdb2ts-conf.xml.template 	$(DESTDIR)/$(WDB2TS_SYSCONFDIR)
	$(INSTALL) -m640 etc/metno-wdb2ts-db.conf.template 		$(DESTDIR)/$(WDB2TS_SYSCONFDIR)
	$(INSTALL) -m640 etc/qbSymbols.def 		$(DESTDIR)/$(WDB2TS_SYSCONFDIR)
if HAS_TOPOGRAPHY_FILE
	ln -s $(TOPOGRAPHY_FILE) $(WDB2TS_MAP_FILE)
endif 
#$(INSTALL) -m644 $(top_srcdir)/dem100.asc.gz 	$(DESTDIR)/$(pkgdatadir)
#gunzip -f $(DESTDIR)/$(pkgdatadir)/dem100.asc.gz


install-exec-hook: mod_metno_wdb2ts.la
	$(mkinstalldirs) $(DESTDIR)$(APACHE_MODULEDIR)
	$(apxs) -i -n metno_wdb2ts -S LIBEXECDIR="$(DESTDIR)$(APACHE_MODULEDIR)" mod_metno_wdb2ts.la

uninstall-local:
	rm -f $(DESTDIR)/$(WDB2TS_SYSCONFDIR)/metno-wdb2ts.conf.template
	rm -f $(DESTDIR)/$(WDB2TS_SYSCONFDIR)/metno-wdb2ts-conf.xml.template
	rm -f $(DESTDIR)/$(WDB2TS_SYSCONFDIR)/metno-wdb2ts-db.conf.template
	rm -f $(DESTDIR)/$(WDB2TS_SYSCONFDIR)/qbSymbols.def
	rm -f $(DESTDIR)$(APACHE_MODULEDIR)/mod_metno_wdb2ts.so*
	rm -f $(WDB2TS_MAP_FILE)
#rm -f $(DESTDIR)/$(pkgdatadir)/dem100.asc


DISTCHECK_CONFIGURE_FLAGS=--with-apache-moduledir=@abs_builddir@/wdb2ts/_distcheck/modules \
	--with-wdb2ts-configdir=@abs_builddir@/wdb2ts/_distcheck/conf \
	--with-wdb2ts-logdir=@abs_builddir@/wdb2ts/_distcheck/log




#-----------------------------------------------------------------------------
# HTML Documentation
#-----------------------------------------------------------------------------

include doc/wdb2ts.mk

#-----------------------------------------------------------------------------
# Doxygen Integration
#-----------------------------------------------------------------------------

#include doc/doxygen/wdb2ts.mk

#MOSTLYCLEANFILES += 		$(DX_CLEANFILES)
#EXTRA_DIST += 				$(DX_CONFIG)

#-----------------------------------------------------------------------------
# Cleaning
#-----------------------------------------------------------------------------

clean-local:
				rm -f $(CLEANFILES)
				$(DOCCLEAN_HOOK)
