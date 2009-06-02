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
	public:
		QueryElement( const std::string &query, bool probe )
			: query_( query ), probe_( probe ) {}
		
		std::string query()const { return query_;}
		bool probe()const { return probe_; }
	};
	
	typedef std::list< QueryElement >      Query;
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
