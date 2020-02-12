/*
    wdb - weather and water data storage

    Copyright (C) 2007 met.no

    Contact information:
    Norwegian Meteorological Institute
    Box 43 Blindern
    0313 OSLO
    NORWAY
    E-mail: wdb@met.no

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
    MA  02110-1301, USA
*/


#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <memory>
#include <thread>
#include <iostream>
#include "src/miutil/pathutil.h"
#include "src/miutil/etcd.h"
#include "src/mod_wdb2ts/etcdconf.h"

namespace wdb2ts {

namespace {

void
EtcdWatchThread(std::shared_ptr<miutil::Etcd> etcd, const std::string key, unsigned long index, pid_t pid=0, int signal=0 ) {
	using namespace std;
	using namespace miutil;

	bool retry=true;

	while(retry) {
		try {
			Etcd::Entry entry;
			//cerr << "Watching: '" << key << "' signal " << signal << " pid " << pid << "\n";
			if(  etcd->watch(key, index, nullptr, &entry) ) {
				cerr << "EtcdWatchThread '" << key << "':  index changed from " << index << " to " << entry.index << ".\n";
				if( signal > 0 ) {
					cerr << "EtcdWatchThread '" << key << "': sending signal "<< signal << " to pid " << pid << ".\n";
					kill( pid, signal);
				}
				retry = false;
			} else {
				cerr << "EtcdWatchThread '" << key << "': watch failed. " << etcd->lastErrorMsg() << "\n";
				this_thread::sleep_for(chrono::seconds(5));
			}
		}
		catch( const EtcdError &ex) {
			cerr << "EtcdWatchThread '" << key << "': Exception. " << etcd->lastErrorMsg() << "\n";
			this_thread::sleep_for(chrono::seconds(5));
		}
	}
}


bool
StartWatchThread(std::shared_ptr<miutil::Etcd> etcd, const std::string &key, unsigned long index,  pid_t pid=0, int signal=0) {
	using namespace std;
	using namespace miutil;

	try {
		thread watcher(EtcdWatchThread, etcd, key, index, pid, signal);
		watcher.detach();
		return true;
	}
	catch(const system_error &ex) {
		cerr << "ERROR: Etcd: Failed to create watch thread for key '" << key << "'. " << ex.what() << ".\n";
	}
	catch( const bad_alloc &ex) {
		cerr << "ERROR: Etcd: Failed to create watch thread for key '" << key << "'. Not enough memmory.\n";
	}
	catch( const exception &ex) {
		cerr << "ERROR: Etcd: Failed to create watch thread for key '" << key << "'. Thread function failed. " << ex.what() << "\n";
	}
	return false;
}

}


std::string
LoadConfigFromEtcd(std::shared_ptr<miutil::Etcd> etcd, const std::string &confDir) {
	using namespace std;
	using namespace miutil;
	Etcd::Entry entry;
	string path;
	string confToDir(fixPath(confDir+"/etcd", false));
	string wdb2tsConf(miutil::fixPath(etcd->getDir()+"/config/current_conf", false));

	try {
		if( ! etcd->getKey(wdb2tsConf, &path, &entry) ) {
			cerr << "ERROR: Etcd: Exception. Failed to get wdb2ts config '" << wdb2tsConf << "'\n";
			return "";
		}

		if( ! etcd->saveDirTo(path, confToDir) ) {
			cerr << "ERROR: Etcd: Failed to save wdb2ts config '" << path << "' to '" << confToDir << "'.\n";
			return "";
		}

		if( ! StartWatchThread(etcd, wdb2tsConf, entry.index, getpid(), SIGUSR2) ) {
			cerr << "ERROR: Etcd: Failed to create watch thread for key '" << wdb2tsConf << "'.\n";
			return "";
		} else {
			cerr << "ETCD: Watching changes to key '" << wdb2tsConf<< "'.\n";
		}

		return confToDir;
	}
	catch(const EtcdError &ex) {
		cerr << "ERROR: Etcd: " << ex.what() << "\n";

	}
	catch (const logic_error &ex) {
		cerr << "ERROR: Etcd: " << ex.what() << "\n";
	}

	return "";

}

}


