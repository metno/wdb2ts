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
#include "Dir.h"
#include "File.h"
#include "etcd.h"
#include "pathutil.h"

using namespace std;
using namespace miutil;

//TODO: Wrap up and make a cli interface.


bool loadWdb2tsConfToEtcd( shared_ptr<Etcd> etcd, const std::string &confDirInEtcd_, const std::string &confdir_) {
	using namespace std;
	string confdir(miutil::fixPath(confdir_, false));
	string confDirInEtcd(miutil::fixPath(confDirInEtcd_, false));
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
			string etcKey=miutil::fixPath(confDirInEtcd+"/"+f, false);
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
			if( ! etcd->setKeyFromFile(f.first, f.second, 0) ) {
				cerr << "Failed to upload file: " << f.second << ", " << etcd->lastErrorMsg() <<"\n";
				error = true;
			}
		}
	}
	catch(const EtcdError &ex) {
		error=true;
		cerr << "Etcd: Failed to load wdb2ts config into etcd. " << ex.what() << endl;
	}

	return ! error;
}

int quit=0;
void sigHandler( int s) {
	quit=1;
}





int
main( int argc, char *argv[])
{
	list<string> files;
	string confTo("config/");
	string confdir("/etc/metno-wdb2ts");
	string currConf;
	string val;
	miutil::Etcd::Entry entry;
	shared_ptr<Etcd> etcd=Etcd::create();


	if( ! etcd){
		cerr << "Failed to create an 'Etcd' instance.\n";
		return 1;
	}
/*
	signal(SIGTERM, sigHandler);
	etcd->printSetup(cerr);

	std::string wdb2tsConf=LoadConfigFromEtcd(etcd, confTo);

	if( wdb2tsConf.empty() ) {
		cerr << "Failed to load wdb2ts config form etcd.\n";
		quit = 1;
	} else {
		cerr << "Config saved to: '" << wdb2tsConf << "'.\n";
	}

	while( quit == 0) {
		this_thread::sleep_for(chrono::seconds(1));
	}

	cerr << "Quiting\n\n";

*/
	/*

	string confDirInEtcd(miutil::fixPath(etcd->getDir() + "/config", false));

	if( ! loadWdb2tsConfToEtcd(etcd, confDirInEtcd, confdir) ) {
		cerr << "Failed to load wdb2ts config to etcd.\n";
		return 1;
	}

	cerr << "Loaded wdb2ts config into etcd.\n";

	if( etcd->saveDirTo(confDirInEtcd, confTo) ) {
		cerr << "Etcd: saved '"<< confDirInEtcd << "' to directory '" << confTo << "'\n";
	} else {
		cerr << "Etcd: Failed to save '"<< confDirInEtcd << "' to directory '" << confTo << "'\n";
	}
*/
}
