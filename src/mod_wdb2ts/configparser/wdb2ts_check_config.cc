#include <iostream>
#include <ConfigParser.h>
#include "miutil/pathutil.h"


using namespace std;
using namespace wdb2ts::config;


void printParamdefs( const ParamDefConfig &paramdef, const std::string &heading )
{
	cerr << endl << endl << "------------------  ParamDefs [" << heading << "]----------------------------------" << endl;

	for( ParamDefConfig::ParamDefs::const_iterator itId = paramdef.idParamDefs.begin();
			itId != paramdef.idParamDefs.end(); ++itId )
	{
		cerr << "[" << itId->first << "] : id" << endl;

		for( wdb2ts::ParamDefList::const_iterator pit = itId->second.begin();
				pit != itId->second.end(); ++pit )
		{
			cerr << "   [" << pit->first << "] " << endl;
			for( std::list<wdb2ts::ParamDef>::const_iterator it = pit->second.begin();
					it != pit->second.end();  ++it )
			{
				cerr << "      " <<  it->alias() << " : " << it->valueparametername()<<endl;
			}
		}
	}
}

int
main(int argn, char **argv )
{
	if( argn != 2 ) {
		cerr << "Use: wdb2ts_check_config configfile" << endl << endl;
		return 1;
	}
	
	wdb2ts::config::Config       *res;
	wdb2ts::config::ConfigParser config(miutil::dirname(argv[1]));

	cerr << "Filename: '" << argv[1] << "'\n";
	cerr << "basedir:  '"<< miutil::dirname(argv[1]) << "'\n";

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
			cerr << "Query id: " << it->first << " Paralells: " << it->second.dbRequestsInParalells()
				 << endl;
			for( Config::Query::const_iterator itQ=it->second.begin();
				  itQ!=it->second.end();
				  ++itQ )
				cerr << endl << "wdbdb: "<< itQ->wdbdb() << "\n  query[" << itQ->query() << "]" << endl;
			cerr << endl;
		}

		printParamdefs( res->paramdef, "Global");
		
		cerr << "******************  WARNINGS  *************************************" << endl;
		cerr << config.getErrMsg() << endl;
	}else
		cout << "NOT ok\n";
	
}
