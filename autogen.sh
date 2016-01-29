#!/bin/sh

if [ x$ACLOCALDIR = x ]; then 
    echo "autoreconf -i -f"
    autoreconf -i -f 
else
    echo "autoreconf -i -f -I$ACLOCALDIR"
    autoreconf -i -f -I$ACLOCALDIR
fi 

