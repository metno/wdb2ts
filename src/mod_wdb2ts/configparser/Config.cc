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
             bool replace,
             std::ostream &err )
{
	return paramdef.addParamDef( paramdefId, pd, provider, replace, err );
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

   paramdef.merge( &other->paramdef, false );

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
	wdb2ts::ParamDefList paramDefs = res.paramdef.paramDefs();
	for( wdb2ts::ParamDefList::const_iterator pit=paramDefs.begin();
	     pit != paramDefs.end();
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

