/*
 * etcdclt.cpp
 *
 *  Created on: Sep 12, 2017
 *      Author: borgem
 */

#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <iostream>
#include <trimstr.h>
#include <map>
#include <list>
#include <thread>
#include "mod_wdb2ts/configparser/ConfigParser.h"
#include "miutil/ptimeutil.h"
#include "miutil/Dir.h"
#include "miutil/File.h"
#include "miutil/etcd.h"
#include "miutil/pathutil.h"
#include "miutil/replace.h"
#include "opt.h"

using namespace std;
using namespace miutil;
namespace pt = boost::posix_time;

//TODO: Wrap up and make a cli interface.


string ConfPathWithCurrentTime(const std::string &prefix ) {
	string ts = isotimeString(pt::second_clock::universal_time(), true, true);
	replaceString(ts, "-", "");
	replaceString(ts, ":", "");

	return prefix + ts;
}

string CurrentConfig(shared_ptr<Etcd> etcd) {
	string val;
	string curConf(fixPath(etcd->getDir() + "/config/current_conf",false));
	if (!etcd->getKey(curConf, &val, nullptr)) {
		cerr << "Failed to look up key '" << curConf << "' in etcd.\n";
		cerr << "This is fatal. The '" << curConf
				<< "' points to the current wdb2ts config.\n";
		if (!etcd->lastErrorMsg().empty()) {
			cerr << "Reason: " << etcd->lastErrorMsg().empty() << "\n";
		}
		val.clear();
	}

	return fixPath(val, false);
}

bool LoadWdb2tsConfToEtcd( const Opt &opt) {
	using namespace std;
	string confdir(opt.config);
	string confDirInEtcd(miutil::fixPath(opt.etcd->getDir()+"/config", false));
	string currConf(fixPath(confDirInEtcd+"/current_conf", false));
	string newConfDir(fixPath(ConfPathWithCurrentTime(confDirInEtcd+"/conf_"), false));

	list<string> files{"metno-wdb2ts-conf.xml", "metno-wdb2ts-db.conf", "qbSymbols.def"};
	string wdb2tsConf(confdir+"/" + *files.begin());

	cerr << "Confdir: '" << confdir << "'\n";
	cerr << "wdb2tsConf: '" << wdb2tsConf << "'\n";

	wdb2ts::config::ConfigParser parser(confdir);
	wdb2ts::config::Config *conf=parser.parseFile(wdb2tsConf);

	if( !conf ) {
		cerr << "Failed to read conf.\n";
		return false;
	}

	for( auto &f : conf->includedFiles) {
		files.push_back(f);
	}


	bool error=false;
	map<string,string> confFiles;

	//Check that all files exist
	for( auto &f:files) {
		string file(confdir+"/" +f);

		if( !dnmi::file::isFile(file) ) {
			cerr << file << " - do not exist.\n";
			error=true;
		} else {
			string etcKey=miutil::fixPath(newConfDir+"/"+f, false);
			confFiles[etcKey]=file;
		}
	}

	if( error ) {
		cerr << "Failed to locate some conf file.\n";
		return 1;
	}


	cerr << "Config files: \n";
	for( auto &f : confFiles) {
		cerr << "    " << f.second << endl;
	}

	cerr << " ----------------------\n";
	error=false;
	try {
		for( auto &f : confFiles) {
			if( opt.dryRun ) {
				cerr << "upload: '"<< f.first << "': '" << f.second << endl;
			} else {
				if( ! opt.etcd->setKeyFromFile(f.first, f.second, 0) ) {
					cerr << "Failed to upload file: " << f.second << ", " << opt.etcd->lastErrorMsg() <<"\n";
					error = true;
				}
			}
		}
	}
	catch(const EtcdError &ex) {
		error=true;
		cerr << "Etcd: Failed to load wdb2ts config into etcd. " << ex.what() << endl;
	}

	if( error )
		return false;

	if( ! opt.dryRun) {
		if( ! opt.etcd->setKey(currConf, newConfDir) ) {
			cerr << "Failed to update '" << currConf << "' with new ref '"<< newConfDir << "'.\n";
			return false;
		}
		cerr << "Updated '" << currConf << "' with new ref '"<< newConfDir << "'.\n";
	} else {
		cerr << "Dry run: Updated '" << currConf << "' with new ref '"<< newConfDir << "'.\n";
	}
	return true;
}

bool DownloadWdb2tsConfFromEtcd( const Opt &opt) {
	string confDirInEtcd(
				miutil::fixPath(opt.etcd->getDir() + "/config", false));
	string currentConfig(CurrentConfig(opt.etcd));
	string val;
	bool isCurrentConfig=false;

	if( !opt.config.empty() ) {
		confDirInEtcd=miutil::fixPath(confDirInEtcd+"/" +opt.config, false);
	} else {
		if( currentConfig.empty() ) {
			cerr << "Failed to get current config!\n";
			return false;
		}
		confDirInEtcd=currentConfig;
		isCurrentConfig=true;
	}


	string confto(opt.wdb2tsConfDir);

	if(confto.empty())
		confto="./wdb2ts-conf";

	confto = fixPath(confto+"/" + basename(confDirInEtcd), false);

	if( opt.dryRun) {
		std::list<miutil::Etcd::Entry> entries;

		if (!opt.etcd->lsDir(confDirInEtcd, &entries, nullptr, true, true)) {
			cerr << "Failed to look up key '" << confDirInEtcd << "' in etcd.\n";
			if (!opt.etcd->lastErrorMsg().empty()) {
				cerr << "Reason: " << opt.etcd->lastErrorMsg().empty() << "\n";
			}
			return false;
		}

		for (miutil::Etcd::Entry &e : entries) {
			string f(confto+"/"+miutil::fixPath(e.key, false));
			cerr << f << endl;
		}
	} else {
		if( ! opt.etcd->saveDirTo(confDirInEtcd,confto)) {
			cerr << "Failed to download config '" << confDirInEtcd << "' to directory '" << confto <<"'\n";
			if (!opt.etcd->lastErrorMsg().empty()) {
				cerr << "Reason: " << opt.etcd->lastErrorMsg().empty() << "\n";
			}
		}
	}

	if( isCurrentConfig )
		cerr << "Downloaded current config (" << basename(confDirInEtcd) << ") to: " << confto << ".\n\n";
	else
		cerr << "Downloaded config (" << basename(confDirInEtcd) << ") to: " << confto << ".\n\n";

	return true;
}

bool ListWdb2tsConfInEtcd(const Opt &opt) {
	std::list<miutil::Etcd::Entry> entries;
	string confDirInEtcd(
			miutil::fixPath(opt.etcd->getDir() + "/config", false));
	string currentConfig(CurrentConfig(opt.etcd));

	if( !opt.config.empty() ) {
		confDirInEtcd=miutil::fixPath(confDirInEtcd+"/" +opt.config, false);
	} else {
		if( currentConfig.empty() ) {
			cerr << "Failed to get current config!\n";
			return false;
		}
		confDirInEtcd=currentConfig;
	}

	if (!opt.etcd->lsDir(confDirInEtcd, &entries, nullptr, true, true)) {
		cerr << "Failed to look up key '" << confDirInEtcd << "' in etcd.\n";
		if (!opt.etcd->lastErrorMsg().empty()) {
			cerr << "Reason: " << opt.etcd->lastErrorMsg().empty() << "\n";
		}
		return false;
	}

	for (miutil::Etcd::Entry &e : entries) {
		if (e.isDir) {
			string d(miutil::fixPath(e.key, true));
			cerr << d << endl;
		} else {
			string f(miutil::fixPath(e.key, false));
			cerr << f << endl;
		}
	}


	if( currentConfig.empty() ) {
		cerr << "Failed to get current config!\n";
		return false;
	} else {
		cerr << "\nCurrent conf: '" << currentConfig << "'\n";
	}

	return true;
}

bool ListWdb2tsConfigs(const Opt &opt) {
	std::list<miutil::Etcd::Entry> entries;
	string confDirInEtcd(
			miutil::fixPath(opt.etcd->getDir() + "/config", false));
	string configDirBasename(miutil::basename(confDirInEtcd));
	string currentConfig(CurrentConfig(opt.etcd));
	string currentConfigBaseName(basename(currentConfig));

	if (!opt.etcd->lsDir(confDirInEtcd, &entries, nullptr, false, true)) {
		cerr << "Failed to look up key '" << confDirInEtcd << "' in etcd.\n";
		if (!opt.etcd->lastErrorMsg().empty()) {
			cerr << "Reason: " << opt.etcd->lastErrorMsg().empty() << "\n";
		}
		return false;
	}

	cout << "Wdb2ts configurations in: " << confDirInEtcd << ".\n";
	for (miutil::Etcd::Entry &e : entries) {
		if (e.isDir) {
			string conf(miutil::basename(miutil::fixPath(e.key, false)));
			if( conf != configDirBasename) {
				if( conf == currentConfigBaseName)
					cout << "* ";
				cout << conf << endl;
			}
		}
	}

	if( currentConfig.empty() ) {
		cerr << "Current conf is NOT set.\n";
		cerr << "This is fatal. The current conf points to the current wdb2ts config.\n";
		cerr << "Use --set-config to set the current config.\n";
	} else {
		cout << "\nCurrent conf: " << currentConfigBaseName <<" (" << currentConfig << ").\n";
	}

	return true;
}



bool
SetWdb2TsConfig(const Opt &opt ) {
	string confDirInEtcd(
			miutil::fixPath(opt.etcd->getDir() + "/config", false));
	if( opt.config.empty()) {
		cerr << "\nMust give config.\n wdb2ts-etcdcli -h for help.\n\n";
		return false;
	}

	miutil::Etcd::Entry entry;
	string val;

	string conf(confDirInEtcd+"/"+opt.config, false);

	if( !opt.etcd->getKey(conf, &val, &entry) ) {
		cerr << "Failed to look up key '" << conf << "' in etcd.\n";
		if (!opt.etcd->lastErrorMsg().empty()) {
			cerr << "Reason: " << opt.etcd->lastErrorMsg().empty() << "\n";
		}
		return false;
	}

	if( !entry.isDir) {
		cerr << "Config: '" << opt.config << "' (" << conf << ") must be a directory.\n";
		return false;
	}

	string curConf(confDirInEtcd + "/current_conf");
	if( ! opt.dryRun) {
		if( ! opt.etcd->setKey(curConf, conf) ) {
			cerr << "Failed to update '" << curConf << "' with new ref '"<< conf << "'.\n";
			return false;
		}
		cerr << "Updated '" << curConf << "' with new ref '"<< conf << "'.\n";
	} else {
		cerr << "Dry run: Updated '" << curConf << "' with new ref '"<< conf << "'.\n";
	}

	return true;
}



bool
RemoveWdb2TsConfig(const Opt &opt ) {
	string confDirInEtcd(
			miutil::fixPath(opt.etcd->getDir() + "/config", false));
	string currentConfig(CurrentConfig(opt.etcd));

	if( opt.config.empty()) {
		cerr << "\nMust give config.\n wdb2ts-etcdcli -h for help.\n\n";
		return false;
	}

	if( currentConfig.empty()) {
		cerr << "ERROR: No current config. Fix this, and try again.\n\n";
		return false;
	}

	miutil::Etcd::Entry entry;
	string val;

	string conf(confDirInEtcd+"/"+opt.config, false);

	if( conf == currentConfig) {
		cerr << "ERROR: trying to remove current config.\n\n";
		return false;
	}


	if( ! opt.etcd->getKey(conf, nullptr, &entry) ) {
		cerr << "The config '" << opt.config << "' (" << conf << ") do not exist.\n";
		return false;
	} else if( ! entry.isDir ){
		cerr << "WARNING: The config '"<<opt.config << "' (" << conf << ") do NOT reference a directory.\n";
	}

	cerr << "Current config: '" << currentConfig << "'\n";
	cerr << "Removing:       '" << conf << "'\n";

	if( opt.dryRun)
		return true;

	if( ! opt.etcd->remove(conf, true) ) {
		cerr << "Failed to remove config: " << conf << ".\n";
		return false;
	}

	cerr << "Removed config: " << conf << ".\n";

	return true;
}


int
main( int argc, char *argv[])
{
	list<string> files;
	string val;

	Opt opt = Opt::parse(argc, argv);
	string confdir(opt.etcdWdb2tsConfDir);

	opt.etcd->printSetup(cerr);

	try {
		switch (opt.cmd) {
		case Opt::Upload:
			if( ! LoadWdb2tsConfToEtcd(opt) )
				return 1;
			break;
		case Opt::Download:
			if( ! DownloadWdb2tsConfFromEtcd(opt) )
				return 1;
			break;
		case Opt::List:
			if( ! ListWdb2tsConfInEtcd(opt) )
				return 1;
			break;
		case Opt::ListConf:
			if( ! ListWdb2tsConfigs(opt) )
				return 1;
			break;
		case Opt::SetConfig:
			if( ! SetWdb2TsConfig(opt) )
				return 1;
			break;

		case Opt::RemoveConfig:
			if( ! RemoveWdb2TsConfig(opt) )
				return 1;
			break;
		default:
			break;
		}
	}
	catch( const miutil::EtcdError &ex ) {
		cerr << ex.what() << endl;
		return 1;
	}

}
