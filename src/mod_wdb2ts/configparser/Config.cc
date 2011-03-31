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

bool
Config::
addParamDef( const std::string &paramdefId,
             const wdb2ts::ParamDef    &pd,
             const std::list<std::string> &provider,
             std::ostream &err )
{
   if( paramdefId.empty() ) {
      if( provider.empty() ) {
         if( ! paramDefs.addParamDef( pd, "" ) ) {
            err << "addParam: ParamDef '" << pd.alias() << "' provider: 'default' allready defined.";
            return false;
         }
      } else {
         for( std::list<std::string>::const_iterator it = provider.begin();
              it != provider.end();
              ++it )
         {
            if( ! paramDefs.addParamDef( pd, *it ) ) {
               err << "addParam: ParamDef '" << pd.alias() <<"' provider: '" << *it << "' allready defined.";
               return false;
            }
         }
      }
   } else {
      if( provider.empty() ) {
          if( ! idParamDefs[paramdefId].addParamDef( pd, "" ) ) {
             err << "addParam: ParamDef '" << pd.alias() << "' provider: 'default' allready defined.";
             return false;
          }
       } else {
          for( std::list<std::string>::const_iterator it = provider.begin();
               it != provider.end();
               ++it )
          {
             if( ! idParamDefs[paramdefId].addParamDef( pd, *it ) ) {
                err << "addParam: ParamDef '" << pd.alias() << "' provider: '" << *it << "' allready defined.";
                return false;
             }
          }
       }
   }

   return true;
}


bool
Config::
merge( Config *other, std::ostream &err, const std::string &file )
{
   if( ! other ) {
      err << "Config::merge: other == 0. Expecting a pointer to Config.";
      return false;
   }

   for( wdb2ts::config::RequestMap::const_iterator it=other->requests.begin();
        it != other->requests.end();
        ++it ) {
      wdb2ts::config::RequestMap::const_iterator itTmp = requests.find( it->first );

      if( itTmp != requests.end() ) {
         err << "WARNING: request <" << it->first
             << "> allready defined. Ignoring redefinition in file <"
             << file << ">." << endl;
         continue;
      }

      requests[ it->first] = it->second;
   }

   for( Config::QueryDefs::const_iterator it= other->querys.begin();
        it != other->querys.end();
        ++it ) {
      Config::QueryDefs::const_iterator itTmp = querys.find( it->first );

      if( itTmp != querys.end() ) {
         err << "WARNING: querydef <" << it->first
             << "> allready defined. Ignoring redefinition in file <"
             << file << ">." << endl;
         continue;
      }

      querys[ it->first ] = it->second;
   }

   for( wdb2ts::ParamDefList::iterator pit =other->paramDefs.begin();
        pit != other->paramDefs.end();
        ++pit ) {
      wdb2ts::ParamDefList::iterator pitTmp = paramDefs.find( pit->first );

      //No params is defined for provider pit->first.
      if( pitTmp == paramDefs.end() ) {
         paramDefs[ pit->first ] = pit->second;
         continue;
      }

      for( std::list<wdb2ts::ParamDef>::const_iterator it = pit->second.begin();
           it != pit->second.end();
           ++it ) {
         if( ! paramDefs.addParamDef( *it, pit->first ) ) {
            err << "WARNING: paramdef <" << it->alias()
                << "> allready defined. Ignoring redefinition in file <"
                << file << ">." << endl;
         }
      }
   }

   return true;
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

