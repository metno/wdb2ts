#! /bin/bash

sed -e "s/@PGPORT@/$PGPORT/" /etc/metno-wdb2ts/metno-wdb2ts-db.conf.template > /tmp/metno-wdb2ts-db.conf.template
sed -e "s/@PGHOST@/$PGHOST/" /tmp/metno-wdb2ts-db.conf.template > /etc/metno-wdb2ts/metno-wdb2ts-db.conf

#sed -e "s/@PGPORT@/$PGPORT/" etc/metno-wdb2ts/metno-wdb2ts-db.conf.template > /tmp/metno-wdb2ts-db.conf.template
#sed -e "s/@PGHOST@/$PGHOST/" /tmp/metno-wdb2ts-db.conf.template > etc/metno-wdb2ts/metno-wdb2ts-db.conf



START_APACHE=true

while [[ $# -gt 0 ]]; do
	echo "DEBUG1: $1"
	case "$1" in
		-b|--bash)
			echo "Setting START_APACHE=false"
    		START_APACHE=false
    	;;
    	*)
            # unknown option
    	;;
	esac
	shift # past argument or value
done

echo "START_APACHE: $START_APACHE"

if [ "$START_APACHE" = "true" ]; then
	exec /usr/sbin/apache2ctl -D FOREGROUND
else
	exec /bin/bash
fi

