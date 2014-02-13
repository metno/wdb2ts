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

if HAS_WDB2TS_TMPDIR
   WDB2TS_SYSTMPDIR=$(WDB2TS_TMPDIR)
else
   WDB2TS_SYSTMPDIR=/tmp
endif


WDB2TS_LOCALSTATEDIR=$(localstatedir)/lib/metno-wdb2ts

AM_CXXFLAGS = -DWDB2TS_DEFAULT_SYSCONFDIR="\""$(WDB2TS_SYSCONFDIR)"\"" \
			  -DWDB2TS_DEFAULT_SYSLOGDIR="\""$(WDB2TS_SYSLOGDIR)"\"" \
			  -DWDB2TS_DEFAULT_SYSTMPDIR="\""$(WDB2TS_SYSTMPDIR)"\"" \
              -DWDB2TS_MAP_FILE="\""$(WDB2TS_MAP_FILE)"\"" \
              -DWDB2TS_LOCALSTATEDIR="\""$(WDB2TS_LOCALSTATEDIR)"\""

 
#-----------------------------------------------------------------------------
# Source Makefiles
#-----------------------------------------------------------------------------

include src/miutil/wdb2ts.mk
include src/util/wdb2ts.mk
include src/tuple_container/wdb2ts.mk
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
include src/mod_wdb2ts/configparser/test/wdb2ts.mk
include src/webFW/test/wdb2ts.mk
include src/miutil/test/wdb2ts.mk
include src/mod_wdb2ts/test/wdb2ts.mk

#-----------------------------------------------------------------------------
# Installation Hooks
#-----------------------------------------------------------------------------

install-data-hook:
	install -d $(DESTDIR)/$(WDB2TS_SYSCONFDIR)
	install -d $(DESTDIR)/$(WDB2TS_SYSLOGDIR)
	install -d $(DESTDIR)/$(WDB2TS_SYSTMPDIR)
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
	--with-wdb2ts-logdir=@abs_builddir@/wdb2ts/_distcheck/log \
	--with-wdb2ts-tmpdir=@abs_builddir@/wdb2ts/_distcheck/tmp




#-----------------------------------------------------------------------------
# HTML Documentation
#-----------------------------------------------------------------------------

include doc/wdb2ts.mk

#-----------------------------------------------------------------------------
# Doxygen Integration
#-----------------------------------------------------------------------------

include doc/doxygen/wdb2ts.mk

MOSTLYCLEANFILES += 		$(DX_CLEANFILES)
EXTRA_DIST += 				$(DX_CONFIG)

#-----------------------------------------------------------------------------
# Cleaning
#-----------------------------------------------------------------------------

clean-local:
				rm -f $(CLEANFILES)
				$(DOCCLEAN_HOOK)
				
#-----------------------------------------------------------------------------
# Debian stuff
#-----------------------------------------------------------------------------

EXTRA_DIST += \
	debian_files/changelog \
	debian_files/compat \
	debian_files/conffiles \
   debian_files/config \
   debian_files/control \
   debian_files/copyright \
   debian_files/dirs \
   debian_files/docs \
   debian_files/postinst \
   debian_files/postrm \
   debian_files/README.Debian \
   debian_files/rules \
   debian_files/templates
   
PKG_DIR = $(PACKAGE)-$(VERSION)
DEBIAN_DIR = $(PKG_DIR)/debian
DEBIAN_PACKAGE = $(PACKAGE)_$(VERSION)
ARCH = `dpkg-architecture -qDEB_HOST_ARCH_CPU`
DEBIAN_PACKAGE_NAME = `head -n1 $(top_srcdir)/debian_files/changelog | sed "s/ (/_/" | sed "s/).*//"`_$(ARCH).deb

debian-name:
	@echo $(DEBIAN_PACKAGE_NAME)

debian:	dist clean-debian 
	tar xvzf $(PKG_DIR).tar.gz
	(test -d $(DEBIAN_DIR) && rm -rf $(DEBIAN_DIR)) || true
	mkdir -p $(DEBIAN_DIR)
	cp -r $(top_srcdir)/debian_files/* $(DEBIAN_DIR)
	cp $(PKG_DIR).tar.gz $(DEBIAN_PACKAGE).orig.tar.gz
	cd $(PKG_DIR) && dpkg-buildpackage -rfakeroot -us -uc
#	lintian --info $(DEBIAN_PACKAGE_NAME)
#	linda $(DEBIAN_PACKAGE_NAME)

update-debian:
	rm -rf $(DEBIAN_DIR)/*
	cp -r $(top_srcdir)/debian_files/* $(DEBIAN_DIR)
	cd $(PKG_DIR) && dpkg-buildpackage -rfakeroot -us -uc -nc
#	lintian --info $(DEBIAN_PACKAGE_NAME)
#	linda $(DEBIAN_PACKAGE_NAME)



clean-debian:
	debclean
	rm -rf $(PKG_DIR) $(DEBIAN_PACKAGE)*


