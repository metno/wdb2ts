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
#include <memory>
#include <vector>
#include <mutex>
#include <stdexcept>
#include "src/cetcd/cetcd-master/cetcd.h"



namespace miutil {


class EtcdError : virtual public std::logic_error {
	int error_;
	public:
		EtcdError(int ecode, const std::string &msg)
			:std::logic_error(msg.c_str()), error_(ecode){}
		EtcdError(int ecode, const char *msg)
					:std::logic_error(msg), error_(ecode){}

	int error()const{ return error_;}
	//const char* what() const noexcept { return msg_.c_str();};
};


class Etcd {
	cetcd_client cli_;
	cetcd_array addrs_;
	std::vector<std::shared_ptr<char> > hosts_;
	std::string dir_;
	static std::mutex mutex_;

	Etcd( const std::list<std::string> &hosts, const std::string &dir);
	std::string theKey( const std::string &key, bool pathSepAtEnd=true)const;

public:
	struct NodeWrapper{
		cetcd_reponse_t *resp;
		cetcd_response_node *node;
		unsigned int index;

		NodeWrapper(cetcd_reponse_t *resp_):resp(resp_), node(resp->node), index(resp->etcd_index){
		};
		NodeWrapper(unsigned int index_,cetcd_response_node_t *node_):resp(nullptr),node(node_), index(index_){};
	};


	struct Entry{
		std::string key;
		bool isDir;
		unsigned long index;
		unsigned long modifiedIndex;
		unsigned long createdIndex;
		unsigned long ttl;

		void init(const NodeWrapper &node);

		Entry(){}

		Entry(unsigned int index, const cetcd_response_node *node);
		Entry(const NodeWrapper &nw);
		Entry& operator=(const NodeWrapper &rhs);

		friend std::ostream &operator<<(std::ostream &o, const Entry &e);
	};

	Etcd();
	~Etcd();


	int         lastErrorCode()const;
	std::string lastErrorMsg()const;

	void printSetup(std::ostream &o)const;

	std::string setDir(const std::string &p);
	std::string getDir()const{ return dir_;}

	std::vector<std::shared_ptr<char> > hosts()const { return hosts_; }

	static std::shared_ptr<Etcd> create(const std::list<std::string> &hosts, const std::string &dir);
	static std::shared_ptr<Etcd> create(const std::string &hosts="", const std::string &dir="");

	/**
	 * @throws EtcdError, if connection to the etcd fails.
	 */
	bool setKey(const std::string &key, const std::string &val, int ttl=0, Entry *entry=nullptr);

	/**
	 * @throws EtcdError, if connection to the etcd fails.
	 */
	bool getKey(const std::string &key, std::string *val, Entry *entry=nullptr);

	/**
	 * @throws EtcdError, if connection to the etcd fails.
	 */
	bool lsDir(const std::string &key, std::list<Entry> *keys, unsigned long *index=nullptr, bool recursive=false, bool sort=false);

	/**
	 * Use the content of key as the directory to search.
	 * @throws EtcdError, if connection to the etcd fails.
	 */
	bool lsLink(const std::string &key, std::list<Entry> *keys, unsigned long *index=nullptr, bool recursive=false, bool sort=false);

	/**
	 * Remove a key or directory.
	 * @param recursive remove a directory recursively.
	 * @throws EtcdError, if connection to the etcd fails.
	 */
	bool remove(const std::string &key, bool recursive=false);

	/**
	 * Will not return before the key changes.
	 * @throws EtcdError, if connection to the etcd fails.
	 */
	bool watch(const std::string &key, unsigned int index, std::string *val, Entry *entry);

	/**
	 * @return the file name.
	 * @throws EtcdError, if connection to the etcd fails.
	 * @throws on other error std::logic_error.
	 */
	std::string saveKeyToFile(const std::string &key, const std::string &path, bool appendKeyAsFilename=true);
	/**
	 * @throws EtcdError, if connection to the etcd fails.
	 */
	bool setKeyFromFile(const std::string &key, const std::string &path, unsigned int ttl=0);

	/**
	 * @throws EtcdError, if connection to the etcd fails.
	 */
	bool saveDirTo(const std::string &key, const std::string &path);
};



typedef std::list<Etcd::Entry> EtcdEntries;

}




#endif /* SRC_MIUTIL_ETCD_H_ */
