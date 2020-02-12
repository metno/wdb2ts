/*
 * opt.h
 *
 *  Created on: Sep 20, 2017
 *      Author: borgem
 */

#ifndef SRC_UTIL_ETCD_OPT_H_
#define SRC_UTIL_ETCD_OPT_H_

#include <memory>
#include <miutil/etcd.h>


struct Opt {
	enum Cmd { Missing, Upload, Download, List, ListConf, SetConfig, RemoveConfig, Last };
	std::string wdb2tsConfDir;
	std::string etcdWdb2tsConfDir;
	std::string etcdHosts;
	std::string config;
	bool dryRun;

	std::shared_ptr<miutil::Etcd> etcd;
	Cmd cmd;

	Opt():dryRun(false), cmd(Opt::List){}

	static Opt parse( int argc, char *argv[] );

};



#endif /* SRC_UTIL_ETCD_OPT_H_ */
