#! /bin/sh

set -e

autoreconf -i
mkdir -p build
cd build
../configure \
	--with-wdb2ts-configdir=/etc/metno-wdb2ts \
	--with-wdb2ts-logdir=/var/log/metno-wdb2ts \
	--with-wdb2ts-tmpdir=/var/lib/metno-wdb2ts/tmp \
	--prefix=/usr \
	--with-apxs=/usr/bin/apxs2 \
    --with-libcurl \
    --enable-wdb2ts-etcdcli \
    --localstatedir=/var 

make all
