#! /bin/bash

set -e

VOLUME="/media/borgem/backup_pc4816/docker-volumes/etcd"
ETCD="gcr.io/etcd-development/etcd:v3.2"

if [ -d "$VOLUME" ]; then 
	sudo rm -rf "$VOLUME"
fi

docker run \
  -p 2379:2379 \
  -p 2380:2380 \
  --name etcd \
  --volume=$VOLUME:/etcd-data \
   $ETCD \
  /usr/local/bin/etcd \
  --name etcd-1 \
  --data-dir /etcd-data \
  --listen-client-urls http://0.0.0.0:2379 \
  --advertise-client-urls http://0.0.0.0:2379 \
  --listen-peer-urls http://0.0.0.0:2380 \
  --initial-advertise-peer-urls http://0.0.0.0:2380 \
  --initial-cluster etcd-1=http://0.0.0.0:2380 \
  --initial-cluster-token etcd-token \
  --initial-cluster-state new
