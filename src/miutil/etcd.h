/*
 * etcd.h
 *
 *  Created on: Jun 8, 2017
 *      Author: borgem
 */

#ifndef SRC_MIUTIL_ETCD_H_
#define SRC_MIUTIL_ETCD_H_

#include <string>
#include <list>
#include <cetcd.h>
#include <boost/shared_ptr.hpp>
#include <boost/thread/thread.hpp>


namespace miutil {

class Etcd {
	cetcd_client cli_;
	cetcd_array addrs_;
	std::string dir_;

	static mutable boost::mutex mutex_;

	Etcd( const std::list<std::string> &hosts, const std::string &dir);
public:
	Etcd();
	~Etcd();

	static boost::shared_ptr<Etcd> create(const std::list<std::string> &hosts, const std::string &dir);

	bool setKey(const std::string &key, const std::string &val, int ttl=0);
	bool getKey(const std::string &key, std::string &val);

};


}




#endif /* SRC_MIUTIL_ETCD_H_ */
