/*
 * etcd.h
 *
 *  Created on: Jun 8, 2017
 *      Author: borgem
 */


#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include "miutil/trimstr.h"
#include "miutil/splitstr.h"
#include "miutil/mkdir.h"
#include "miutil/pathutil.h"
#include "miutil/compresspace.h"
#include "miutil/readfile.h"
#include "etcd.h"

namespace miutil {

namespace {
	thread_local std::string ErrMsg_;
	thread_local int ErrCode_;


	char *dup(const std::string &s) {
		char *b=new char[s.size()+1];
		b[s.size()]='\0';
		strncpy(b, s.c_str(), s.size());
		return b;
	}

	std::string getEnv(const char *var, const std::string &defaultVal) {
		char *env = getenv(var);
		if( env == NULL )
			return defaultVal;

		std::string e(env);
		miutil::trimstr(e);

		if( e.empty())
			return defaultVal;
		else
			return e;
	}


	bool getVal(const Etcd::NodeWrapper &nw, std::string *val, Etcd::Entry *entry) {
		if( ! nw.node ) {
			return false;
		}

		if( val ) {
			val->clear();
			if( nw.node->value)
				*val=nw.node->value;
		}

		if( entry ) {
			*entry = nw;
		}

		return true;
	}

	void getKeys(const Etcd::NodeWrapper &nw, const std::string &dir, std::list<Etcd::Entry> &entries)
	{
		using namespace std;

		if (!nw.node)
			return;

		if(nw.node->key)
			entries.push_back(Etcd::Entry(nw));

		if (!nw.node->nodes)
			return;

		for(int i=0; i<cetcd_array_size(nw.node->nodes); i++) {
			cetcd_response_node *n=static_cast<cetcd_response_node*>(cetcd_array_get(nw.node->nodes, i));
			getKeys(Etcd::NodeWrapper(nw.index, n), dir, entries);
		}
	}

	/**
	 * returns true on ok. Print the error  to the errStream.
	 */
	bool
	isOk(cetcd_response *resp, std::ostream &errStream, const std::string &errMsg="") {
		using namespace std;

		if(resp->err) {
			const int N=1024;
			char buf[N];
			if(errMsg.empty())
				snprintf(buf,N,"Etcd: error :%d, %s (%s)\n", resp->err->ecode, resp->err->message, resp->err->cause);
			else
				snprintf(buf,N,"Etcd (%s) : error :%d, %s (%s)\n", errMsg.c_str(), resp->err->ecode, resp->err->message, resp->err->cause);

			buf[N-1]='\0';
			errStream << buf;

			ErrCode_ = resp->err->ecode;
			ErrMsg_=buf;
			if(resp->err->ecode >= 1000 )
				throw EtcdError(resp->err->ecode, buf);
			return false;
		} else {
			return true;
		}
	}


}

void
Etcd::Entry::
init(const NodeWrapper &nw)
{
	if( !nw.node )
		return;

	if( nw.node->key)
		key=fixPath(nw.node->key, false);

	index = nw.index;
	modifiedIndex=nw.node->modified_index;
	createdIndex = nw.node->created_index;
	isDir=nw.node->dir!=0?true:false;
	ttl = nw.node->ttl;
}


Etcd::Entry::
Entry(const NodeWrapper &nw){
	init(nw);
}

Etcd::Entry&
Etcd::Entry::operator=(const NodeWrapper &rhs)
{
	init( rhs );
	return *this;
}




Etcd::Etcd( const std::list<std::string> &hosts, const std::string &dir )
{
	using namespace std;
	hosts_.reserve(hosts.size());
	cetcd_array_init(&addrs_, hosts.size());

	for( auto &host : hosts) {
		hosts_.push_back(shared_ptr<char>(dup(host)));
		cetcd_array_append(&addrs_, hosts_.rbegin()->get());
	}

	dir_=miutil::fixPath(dir, true);
	cetcd_client_init(&cli_, &addrs_);
}

std::string
Etcd::theKey( const std::string &key_, bool pathSepAtEnd)const
{
	using namespace std;
	string key(miutil::trimstrCopy(key_));

	if( key.empty())
		return "";
	else if( key[0] != '/')
		key = dir_+"/"+key;

	return miutil::fixPath(key, pathSepAtEnd);
}

Etcd::Etcd()
{

}

Etcd::~Etcd(){
	cetcd_client_destroy(&cli_);
	cetcd_array_destroy(&addrs_);
}


int
Etcd::lastErrorCode()const{
	return ErrCode_;
}

std::string
Etcd::lastErrorMsg()const{
	return ErrMsg_;
}

std::string Etcd::setDir(const std::string &p){
	std::string old=dir_;
	dir_= miutil::fixPath(p, true);
	return old;
}

std::shared_ptr<miutil::Etcd> Etcd::create( const std::list<std::string> &hosts,
		const std::string &dir )
{
	return std::shared_ptr<miutil::Etcd>(new Etcd(hosts, dir));
}

std::shared_ptr<Etcd>
Etcd::create(const std::string &hosts_, const std::string &dir_)
{
	using namespace std;

	string hosts(hosts_);
	string dir(miutil::trimstrCopy(dir_));
	list<string> lHosts;

	if( hosts.empty()) {
		hosts = getEnv("ETCD_HOSTS", "localhost:2379");
		if( hosts.empty() )
			return shared_ptr<Etcd>();
	}

	if( dir.empty())
		dir=getEnv("ETCD_WDB2TS_PATH","/wdb2ts/");

	dir = "/" + dir;
	dir = miutil::fixPath(dir, true);

	vector<std::string> vHosts=miutil::splitstr(hosts);

	for( auto &host : vHosts)
		lHosts.push_back(host);

	return create(lHosts, dir);
}

void Etcd::printSetup(std::ostream &o)const
{
	o << "Etcd config: \n";
	o << "  path: '" << dir_ << "\n";

	if( hosts_.empty())
		o << "  hosts: none\n";
	else
		o << "  hosts:\n";

	for( auto &host : hosts_)
		o << "     " << host << "\n";
}

bool Etcd::setKey( const std::string &key_, const std::string &val, int ttl, Entry *entry )
{
	using namespace std;
	cetcd_response *resp;
	string key(theKey(key_, false));

	if( key.empty() )
		return false;

	resp = cetcd_set(&cli_, key.c_str(), val.c_str(), ttl);

	if( ! isOk(resp, cerr, string("setKey: '")+key+"'") )
		return false;

	if(entry)
		*entry = NodeWrapper(resp);

	//cetcd_response_print(resp);
	cetcd_response_release(resp);
	return true;
}

bool
Etcd::getKey( const std::string &key_, std::string *val, Entry *entry )
{
	using namespace std;
	cetcd_response *resp;
	string key(theKey(key_, false));

	if( key.empty() )
		return false;

	resp = cetcd_get(&cli_, key.c_str());

	if( !isOk(resp, cerr, string("getKey: '")+key+"'"))
		return false;

	bool ret=getVal(NodeWrapper(resp), val, entry);

	cetcd_response_release(resp);
	return ret;
}

bool
Etcd::lsDir(const std::string &key_, std::list<Entry> *keys, unsigned long *index, bool recursive_, bool sort_)
{
	using namespace std;
	int sort=sort_?1:0;
	int recursive= recursive_?1:0;
	cetcd_response *resp;
	string key(theKey(key_, false));

	if( key.empty() || !keys)
		return false;

	keys->clear();

	resp=cetcd_lsdir(&cli_, key.c_str(), sort, recursive );

	if( !isOk(resp, cerr, string("lsDir: '")+key+"'"))
		return false;

	getKeys(NodeWrapper(resp), key, *keys);
	cetcd_response_release(resp);

	if( keys && index ) {
		*index = 0;
		for( Entry &e : *keys ) {
			if(  e.isDir && e.key == key) {
				*index = e.index;
				break;
			}
		}
	}

	return true;
}

bool
Etcd::lsLink(const std::string &key, std::list<Entry> *keys, unsigned long *index, bool recursive, bool sort){
	using namespace std;
	string val;
	Etcd::Entry entry;


	if( ! getKey( key, &val, &entry) ) {
		return false;
	}

	if( entry.isDir ) {
		return lsDir(key, keys, index, recursive, sort);
	}

	if( index )
		*index = entry.index;

	return lsDir(val, keys, nullptr, recursive, sort);
}

bool
Etcd::remove(const std::string &key_, bool recursive){
	using namespace std;
	cetcd_response *resp;
	string key(theKey(key_, false));
	Entry entry;

	if( key.empty() )
		return false;

	if( ! getKey( key, nullptr, &entry) )
		return false;


	if( entry.isDir ) {
		resp = cetcd_rmdir(&cli_, key.c_str(), recursive);
	} else {
		resp = cetcd_delete(&cli_, key.c_str());
	}

	if( !isOk(resp, cerr, string("remove: '")+key+"'"))
		return false;

	cetcd_response_release(resp);

	return true;
}

bool
Etcd::watch(const std::string &key_, unsigned int index_, std::string *val, Entry *entry) {
	using namespace std;
	cetcd_response *resp;
	string key(theKey(key_, false));

	if( key.empty() )
		return false;

	//cerr << "Etcd::watch: key: '" << key << "' index: " << index_+1 << "\n";

	//Seems we must wait with an index that do not exist, yet.
	resp=cetcd_watch(&cli_, key.c_str(), index_+1 );

	if( !isOk(resp, cerr, string("watch: '")+key+"'") ) {
		return false;
	}

	cetcd_response_print(resp);

	bool res = getVal(NodeWrapper(resp), val, entry);
	cetcd_response_release(resp);
	return res;
}


std::string
Etcd::
saveKeyToFile(const std::string &key, const std::string &path_, bool appendKeyAsFilename)
{
	using namespace std;
	string path;
	string fpath(path_ + "/" + key);
	string val;

	if( ! appendKeyAsFilename ) {
		path=miutil::dirname(path_);
		fpath=path+"/"+miutil::basename(path_);
	} else {
		path = miutil::dirname(fpath);
		fpath=path+"/"+miutil::basename(fpath);;
	}

	path = miutil::fixPath(path, false);
	fpath = miutil::fixPath(fpath, false);

	dnmi::file::checkAndMkDirs(path);

	if( ! getKey(key, &val, nullptr) ) {
		throw logic_error(string("Etcd: saveToFile,  key '") + key +"' dont exist or etcd error.");
	}

	ofstream out(fpath);

	if( !out ) {
		throw logic_error(string("Failed to write file '")+fpath+"'.");
	}

	out << val;
	out.close();

	if( out.fail())
		throw logic_error(string("Failed to write value to file '")+fpath+"'.");

	return fpath;
}

bool
Etcd::
setKeyFromFile(const std::string &key, const std::string &path, unsigned int ttl)
{
	using namespace std;
	string buf;

	if( !miutil::readFile(path, buf) ) {
		return false;
	}

	return setKey(key, buf, ttl, nullptr);
}


bool
Etcd::
saveDirTo(const std::string &key_, const std::string &path_){
	using namespace std;
	string key(theKey(string("/")+key_, true));
	string path(miutil::fixPath(path_, true));
	EtcdEntries entries;

	if( key.empty()) {
		cerr << "Etcd: Save config to dir failed. Empty etcd key.\n";
		return false;
	}

	if( ! lsDir(key, &entries, nullptr, true, true) ) {
		cerr << "Etcd: Save config to dir failed. Could NOT get content of '"<< key <<"'.\n";
		return false;
	}

	for( Etcd::Entry &e : entries ) {
		cerr << e.key << "\n";
	}

	for( Etcd::Entry &e : entries ) {
		if(e.isDir)
			continue;

		string fname;
		string::size_type i=e.key.find(key);

		if( i != 0) {
			cerr << "Etcd: Expecting path '" << key << "' at start '" << e.key << "'\n";
			fname = miutil::fixPath(path+e.key,false);
		} else {
			fname = path+e.key.substr(key.size());
		}

		try {
			saveKeyToFile(e.key, fname, false);
		}
		catch ( const exception &ex) {
			cerr << "Etcd: saveToDir failed: " << ex.what() << endl;
			return false;
		}
	}

	return true;
}


std::ostream&
operator<<(std::ostream &o, const Etcd::Entry &e) {
	o << "Etcd: key: '" << e.key << "' index: " << e.index << " ttl: " << e.ttl << "s dir: " << (e.isDir?"t":"f");
	return o;
}

}

