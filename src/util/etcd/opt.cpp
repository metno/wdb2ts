/*
 * opt.h
 *
 *  Created on: Sep 20, 2017
 *      Author: borgem
 */

#include <unistd.h>
#include <stdlib.h>    /* for exit */
#include <getopt.h>
#include <iostream>
#include "miutil/pathutil.h"
#include "opt.h"

using namespace std;

namespace {

//enum {no_argument, required_argument, optional_argument };

int index;

struct option options[] = {
		{"upload", required_argument, NULL, Opt::Upload},
		{"download", optional_argument, NULL, Opt::Download},
		{"list", optional_argument, NULL, Opt::List},
		{"list-config", no_argument, NULL, Opt::ListConf},
		{"set-config", required_argument, 0,Opt::SetConfig},
		{"rm-config", required_argument, 0,Opt::RemoveConfig},
		{"hosts", required_argument, NULL, 's'},
		{"wdb2ts-configdir", required_argument, NULL, 'c'},
		{"etcd-configdir", required_argument, NULL, 'e'},
		{"dry-run", no_argument, NULL, 'r'},
		{"help", required_argument, NULL, 'h'},
		{0, 0, 0, 0}
};

void use( int exit_, const std::string &msg="") {
	cerr << "\nUSE: \n"
		 << "  wdb2ts-etcdcli CMD OPTIONS \n\n"
		 <<	"Client to maintain wdb2ts config in etcd\n\n"
		 << "CMD \n"
		 << "\t--upload Upload the configuration given with --wdb2ts-configdir\n"
		 <<	"\t      etcd.\n"
		 << "\t--download Download the current wdb2ts configuration from etcd.\n"
		 << "\t     Use -d to specify a directory to save to, default ./wdb2ts-conf.\n"
		 << "\t--list(=config) List the wdb2ts configurations for 'config' or all configs if\n"
		 << "\t     the config parameter is missing.\n\n"
		 << "\t--list-config List all wdb2ts configurations in etcd. This is the default.\n"
		 << "\t--set-config config Set current config to what is specified with config. \n"
		 << "\t      The config must allready exist in etcd.\n"
		 << "\t--rm-config config Remove config. It is not possible to remove the current config.\n"
		 << "OPTIONS: \n"
		 << "\t--hosts|-s hosts, the etcd to use. Hosts is a comma separated list of\n"
		 << "\t      etcd in a cluster.\n"
		 << "\t--etcd-configdir|-c dir upload/download to/from this directory in etcd.\n"
		 << "\t      Default /wdbt2s. Most of the time leave this at the default.\n\n"
		 << "\t--dry-run|-r Just show what will eventually be downloaded or uploaded.\n"
		 << "\t--help|-h Print this help screen and exit.\n\n"
		 << "If --host is not given and the environment variable ETCD_HOSTS is set then the hosts\n"
		 << "given with ETCD_HOSTS is used. The environment variable ETCD_WDB2TS_PATH is used for\n"
		 << "--etcd-configdir if it is set\n\n";

	if( ! msg.empty())
		cerr << msg << "\n\n";

	exit(exit_);
}
}


Opt
Opt::parse( int argc, char *argv[] ) {
	int ch;
	bool end=false;
	Opt opt;

	opt.cmd=Opt::Missing;

	while(!end) {
		 ch = getopt_long(argc, argv, "rd:s:c:h",
		                     options, &index);

		 if( opt.cmd != Opt::Missing && ch>Opt::Missing && ch < Opt::Last) {
			 use(1,"ERROR: Can only give one CMD option.");
		 }

		 switch( ch ) {
		 case -1:
			 end=true;
			 break;
		 case '?':
			 use(1);
			 break;
		 case Opt::Upload:
			 opt.cmd = static_cast<Opt::Cmd>(ch);
			 opt.config = optarg?optarg:"";
			 break;
		 case Opt::Download:
			 opt.cmd = static_cast<Opt::Cmd>(ch);
			 opt.config = optarg?optarg:"";
			 break;
		 case Opt::List:
			 opt.cmd = static_cast<Opt::Cmd>(ch);
			 opt.config = optarg?optarg:"";
			 break;
		 case Opt::ListConf:
			 opt.cmd = static_cast<Opt::Cmd>(ch);
			 break;
		 case Opt::SetConfig:
			 opt.cmd = static_cast<Opt::Cmd>(ch);
			 opt.config = optarg?optarg:"";
			 break;
		 case Opt::RemoveConfig:
			 opt.cmd = static_cast<Opt::Cmd>(ch);
			 opt.config = optarg?optarg:"";
			 break;
		 case 'd':
			 opt.wdb2tsConfDir=optarg;
			 break;
		 case 's':
			 opt.etcdHosts=optarg;
			 break;
		 case 'c':
			 opt.etcdWdb2tsConfDir=miutil::fixPath(optarg, false);
			 break;

		 case 'r':
			 opt.dryRun=true;
			 break;
		 case 'h':
			 use(0);
			 break;
		 default:
			 cerr << "Unknown value '" << ch << "'\n";
		 }
	}

	if( opt.cmd == Opt::Missing)
		opt.cmd = Opt::ListConf;


	opt.etcd = miutil::Etcd::create(opt.etcdHosts, opt.etcdWdb2tsConfDir);

	if( !opt.etcd || opt.etcd->hosts().empty()) {
		cerr << "No etcd hosts is given.\n";
		use(1);
	}

	if( opt.etcd->getDir().empty()) {
		cerr << "Must give an config directory to use in etcd.\n";
		use(1);
	}

	return opt;
}

