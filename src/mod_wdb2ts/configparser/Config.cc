#include "Config.h"


using namespace std;

namespace wdb2ts {
namespace config {

/**
 * TODO: Implement it.
 */
bool 
Config::
validate( std::ostream &message ) const
{
	return true;
}

Config::Query 
Config::
query( const std::string &query ) const
{
	QueryDefs::const_iterator it = querys.find( query );
	
	if( it == querys.end() )
		return Query();
	
	return it->second;
}



std::ostream& 
operator<<( std::ostream& output, const Config& res)
{
	output<< endl << endl << "*****************************************************************" << endl;
	for( wdb2ts::config::RequestMap::const_iterator it=res.requests.begin();
		  it!=res.requests.end();
		  ++it ) {
		output << *it->second << endl;
	}
	
	output << endl << endl << "------------------  Querys -----------------------------------" << endl;
	for( Config::QueryDefs::const_iterator it=res.querys.begin();
	     it!=res.querys.end();
	     ++it ) {
		output << "Query id: " << it->first << endl;
		for( Config::Query::const_iterator itQ=it->second.begin();
			  itQ!=it->second.end();
			  ++itQ )
			output << endl << "query[" << itQ->query() << "]" << endl;
		output << endl;
	}
	
	output << endl << endl << "------------------  ParamDefs -----------------------------------" << endl;
	for( wdb2ts::ParamDefList::const_iterator pit=res.paramDefs.begin();
	     pit != res.paramDefs.end();
	     ++pit )
	{
		for( std::list<wdb2ts::ParamDef>::const_iterator it = pit->second.begin();
	        it != pit->second.end();
	        ++it )
			output << "[" << pit->first << "] " << *it << endl;
	}
		
	return output;	
}


}
}

