#include <iostream>
#include <ConfigParser.h>


using namespace std;
using namespace wdb2ts::config;

int
main(int argn, char **argv )
{
	wdb2ts::config::Config       *res;
	wdb2ts::config::ConfigParser config;
	
	if( argn != 2 ) {
		cerr << "Use: wdb2ts_check_config configfile" << endl << endl;
		return 1;
	}
	
	res = config.parseFile( argv[1] );
	
	if( res ) {
		cerr << endl << endl << "*****************************************************************" << endl;
		for( wdb2ts::config::RequestMap::const_iterator it=res->requests.begin();
			  it!=res->requests.end();
			  ++it ) {
			cerr << *it->second << endl;
		}
		
		cerr << endl << endl << "------------------  Querys -----------------------------------" << endl;
		for( Config::QueryDefs::const_iterator it=res->querys.begin();
		     it!=res->querys.end();
		     ++it ) {
			cerr << "Query id: " << it->first << endl;
			for( Config::Query::const_iterator itQ=it->second.begin();
				  itQ!=it->second.end();
				  ++itQ )
				cerr << endl << "query[" << itQ->query() << "]" << endl;
			cerr << endl;
		}
		
		cerr << endl << endl << "------------------  ParamDefs -----------------------------------" << endl;
		for( wdb2ts::ParamDefList::iterator pit = res->paramDefs.begin(); 
		     pit != res->paramDefs.end(); 
		     ++pit )
		{
			for( std::list<wdb2ts::ParamDef>::iterator it = pit->second.begin();
		        it != pit->second.end();
		        ++it )
				cerr << "[" << pit->first << "] " << *it << endl;
		}
		
		cerr << "******************  WARNINGS  *************************************" << endl;
		cerr << config.getErrMsg() << endl;
	}else
		cout << "NOT ok\n";
	
}
