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
		int prognosisLengthSeconds_;
	public:
		QueryElement( const std::string &query, bool probe, bool stopIfData,
				      const std::string &wdbdb, int prognosisLengthSeconds )
			: query_( query ), probe_( probe ), stopIfData_( stopIfData ), wdbdb_( wdbdb ),
			  prognosisLengthSeconds_( prognosisLengthSeconds ){}
		QueryElement( const QueryElement &qe )
         : query_( qe.query_), probe_( qe.probe_ ), stopIfData_( qe.stopIfData_ ), wdbdb_( qe.wdbdb_ ),
		   prognosisLengthSeconds_( qe.prognosisLengthSeconds_ ){}
		
		std::string query()const { return query_;}
		bool probe()const { return probe_; }
		bool stopIfData() const { return stopIfData_; }
		std::string wdbdb()const { return wdbdb_; }
		int prognosisLengthSeconds()const { return prognosisLengthSeconds_;}
	};
	
	typedef std::list< QueryElement >      QueryList;

	class Query {
		QueryList querys_;
		int dbRequestsInParalells_;

	public:
		Query():
			dbRequestsInParalells_(0)
			{}

		Query( const Query &q )
         : querys_( q.querys_ ), dbRequestsInParalells_( q.dbRequestsInParalells_ ) {}

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
	
	///ParamDefs is a map where the key is a paramdefid and the val is wdb2ts::ParamDefList.
	typedef std::map< std::string, wdb2ts::ParamDefList > ParamDefs;
	Config(){};
	~Config(){};
	
	QueryDefs            querys;
	ParamDefConfig       paramdef;
	//ParamDefs            idParamDefs;
	//wdb2ts::ParamDefList paramDefs;
	RequestMap           requests;
	
	bool validate( std::ostream &message ) const;
	Query query( const std::string &query ) const;
	
	/**
	 * Add a paramdef to either paramDefs or idParamDefs.
	 *
	 * It is added to idParamDefs if paramdefId is set and to
	 * paramdefs if peramdefId is not set.
	 *
	 * Return false if a paramdef with the same alias allready exist.
	 * Return true if the paramdef was added to the paramsDefs.
	 */
	bool
	addParamDef( const std::string &paramdefId,
	             const wdb2ts::ParamDef    &pd,
	             const std::list<std::string> &provider,
	             bool replace,
	             std::ostream &err );


	friend 
	std::ostream& operator<<( std::ostream& output,
							        const Config& v);

	bool merge( Config *other, std::ostream &err, const std::string &file );

};

std::ostream& 
operator<<( std::ostream& output, const Config& v);

}
}

#endif 
