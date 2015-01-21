#! /bin/sh
CONFIGURE=/home/borgem/projects/workspace/wdb/metlib/configure
PREFIX=/disk1/metlibs
LIBDIR=/disk1/metlibs/lib/metlibs	
INCLUDEDIR=/disk1/metlibs/include/metlibs
export PKG_CONFIG_PATH=/usr/lib/pkgconfig:/disk1/local/lib/pkgconfig

${CONFIGURE} --prefix=${PREFIX} \
--disable-diField \
--disable-diMItiff \
--disable-diSQL \
--disable-glp \
--disable-glText \
--disable-GribAPI \
--enable-libmi \
--disable-miFTGL \
--enable-milib \
--disable-pets2 \
--enable-pgconpool \
--disable-pods \
--disable-profet \
--disable-proFunctions \
--disable-propoly \
--disable-puDatatypes \
--enable-puMet \
--disable-puSQL \
--disable-qTimeseries \
--disable-qUtilities \
--disable-newarkAPI \
--disable-robs \
--disable-tsData \
--libdir=${LIBDIR} --includedir=${INCLUDEDIR} --disable-shared
