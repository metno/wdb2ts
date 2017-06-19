#! /bin/bash

#
#Script to build wdb2ts in a container.
#It serves as the entrypoint in the container.
#
# Option 
#  -b|--build Build wdb2ts and exit. This is the default
#  -d|--devel Configure and return to a bash prompt in the
#       build directory. 
#

set -e

BUILD=true

while [[ $# -gt 0 ]]; do
	case "$1" in
		-b|--build)
    		BUILD=true
    	;;
    	-d|--devel)
    		BUILD=false
    	;;
    	*)
            # unknown option
    	;;
	esac
	shift # past argument or value
done

#Dependencies is compiled into /usr/local/lib.
#COPY dependencies from /usr/local/lib to /artifacts/usr/local/lib 
mkdir -p /artifacts/usr/local/lib
cp -R /usr/local/lib/*.so* /artifacts/usr/local/lib

#Allways configure the build
/src/configure \
	--with-wdb2ts-configdir=/etc/metno-wdb2ts \
	--with-wdb2ts-logdir=/var/log/metno-wdb2ts \
	--with-wdb2ts-tmpdir=/var/lib/metno-wdb2ts/tmp \
	--prefix=/usr \
	--with-apxs=/usr/bin/apxs2 \
    --with-libcurl \
    --localstatedir=/var 


if [ "$BUILD" == "false" ]; then
	# Replace this script with bash
	exec /bin/bash
fi

#Build wdb2ts and exit 

make 
make install DESTDIR=/artifacts
