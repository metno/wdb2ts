#ifndef __WDB2TS_CONFIG_H__
#define __WDB2TS_CONFIG_H__

#include <ostream>
#include <map>
#include <ParamDef.h>
#include <RequestConf.h>

namespace wdb2ts {
namespace config {



class Config 
{
	Config( const Config & );
	Config& operator=(const  Config & );


public:
	class QueryElement
	{
		std::string query_;
		bool probe_;
		bool stopIfData_;
		std::string wdbdb_;
	public:
		QueryElement( const std::string &query, bool probe, bool stopIfData, const std::string &wdbdb )
			: query_( query ), probe_( probe ), stopIfData_( stopIfData ), wdbdb_( wdbdb ) {}
		
		std::string query()const { return query_;}
		bool probe()const { return probe_; }
		bool stopIfData() const { return stopIfData_; }
		std::string wdbdb()const { return wdbdb_; }
	};
	
	typedef std::list< QueryElement >      QueryList;

	class Query {
		QueryList querys_;
		int dbRequestsInParalells_;

	public:
		Query():
			dbRequestsInParalells_(0)
			{}

		typedef QueryList::iterator iterator;
		typedef QueryList::const_iterator const_iterator;

		QueryList querys()const{  return querys_; }
		QueryList::iterator begin(){ return querys_.begin(); }
		QueryList::const_iterator begin()const { return querys_.begin(); }

		QueryList::iterator end(){ return querys_.end(); }
		QueryList::const_iterator end()const { return querys_.end(); }

		int empty() const { return querys_.empty(); }
		void push_back( const QueryElement &val ) { querys_.push_back( val ); }
		int  dbRequestsInParalells()const { return dbRequestsInParalells_; }
		void dbRequestsInParalells( int n ){ dbRequestsInParalells_ = n; }
	};


	typedef std::map< std::string, Query > QueryDefs;
	
	Config(){};
	~Config(){};
	
	QueryDefs            querys;
	wdb2ts::ParamDefList paramDefs;
	RequestMap           requests;
	
	bool validate( std::ostream &message ) const;
	Query query( const std::string &query ) const;
	
	friend 
	std::ostream& operator<<( std::ostream& output,
							        const Config& v);

};

std::ostream& 
operator<<( std::ostream& output, const Config& v);

}
}

#endif 
