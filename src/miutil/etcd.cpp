/*
 * etcd.h
 *
 *  Created on: Jun 8, 2017
 *      Author: borgem
 */

#include "etcd.h"

namespace miutil {

Etcd::Etcd( const std::list<std::string> &hosts, const std::string &dir )
{

}

Etcd::Etcd()
{

}

Etcd::~Etcd(){

}

boost::shared_ptr<miutil::Etcd> Etcd::create( const std::list<std::string> &hosts,
		const std::string &dir )
{
	return boost::shared_ptr<miutil::Etcd>();
}

bool Etcd::setKey( const std::string &key, const std::string &val, int ttl )
{
	return false;
}

bool Etcd::getKey( const std::string &key, std::string &val )
{
	return false;
}

}

