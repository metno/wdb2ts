/*
    wdb - weather and water data storage

    Copyright (C) 2008 met.no

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

/**
 * @addtogroup wdb2ts 
 * @{
 * @addtogroup mod_wdb2ts
 * @{
 */
/**
 * @file
 * Implementation of the UrlParamLevelSpec class.
  */
#ifdef HAVE_CONFIG_H
#include <wdb2ts_config.h>
#endif
// CLASS
#include <UrlParamLevelSpec.h>
// PROJECT INCLUDES
//
// SYSTEM INCLUDES
#include <iostream>
#include <sstream>
#include <vector>
#include <splitstr.h>
#include <trimstr.h>

using namespace std;

namespace wdb2ts {
    
UrlParamLevelSpec::
UrlParamLevelSpec( )
{
   valid_ = true;
   selectPart_ = "NULL";
}

UrlParamLevelSpec::
UrlParamLevelSpec( int protocol )
	: UrlParam( protocol )
{
   valid_ = true;
   selectPart_ = "NULL";
}

void
UrlParamLevelSpec::
clean()
{
   valid_ = true;
   selectPart_ = "NULL";
}


bool 
UrlParamLevelSpec::
validLevel( const std::string &level )
{
    for ( string::size_type i=0; i<level.size(); ++i ) {
    	if ( ! isdigit( level[i] ) )
    		return false;
    }    
    return true;
}
    

bool 
UrlParamLevelSpec::
validIndCode( const std::string &indCode )
{
	if( indCode == "exact" ) 
    	return true;
	else if( indCode == "any" && protocol == 1) 
		return true;
	else if( indCode == "inside" ) 
		return true;
	else if( indCode == "intersect" && protocol == 1 ) 
		return true;   
    
	if( protocol > 2 ) {
		if( indCode == "below" ) 
			return true;
		else if( indCode == "above" ) 
			return true;   
	}

	return false;
}

/**
 * @TODO proper implementation.
 */
bool 
UrlParamLevelSpec::
validLeveldomain( const std::string &leveldomain )
{
    return ! leveldomain.empty();
}

void 
UrlParamLevelSpec::
decodeProtocol2( const std::string &toDecode )
{
   const char *defIndCode="exact";
   ostringstream ost;
   string from;
   string to;
   string leveldomain;
   string ind(defIndCode);
   bool fromValid, toValid, indValid, leveldomainValid;
   
   string *tmp[]={ &from, &to, &leveldomain, &ind };
   vector<string> vals=miutil::splitstr(toDecode, ',');
   
   if( vals.empty() ) {
		valid_ = true;
		selectPart_ = "NULL";
		return;
   }
   
   if( vals.size() < 2 )
   	throw logic_error("LevelSpec: To few values!");
   
   if( vals.size() > 4 )
   	throw logic_error("LevelSpec: To many values!");
   
  for( vector<string>::size_type i=0; i<vals.size(); ++i )
      *tmp[i] = vals[i]; 
  
  miutil::trimstr( from, miutil::TRIMBOTH, " \t\r\n\"" );
  miutil::trimstr( to, miutil::TRIMBOTH, " \t\r\n\"" );
  miutil::trimstr( leveldomain,  miutil::TRIMBOTH, " \t\r\n\"" );
  miutil::trimstr( ind, miutil::TRIMBOTH, " \t\r\n\"");
  
  if( from.empty() && to.empty() && leveldomain.empty() && ind.empty() ){
      valid_ = true;
      selectPart_ = "NULL";
      return;
  }
  
  fromValid = validLevel( from );
  toValid = validLevel( to );
  leveldomainValid = validLeveldomain( leveldomain );
  indValid = validIndCode( ind );
  
  if(!fromValid && !toValid )
      throw logic_error("LevelSpec: Missing required levelfrom or levelto!");
  
  if( ! fromValid ) 
      from = to;
  else if( ! toValid ) {
      if( vals.size() == 3 ) 
	   ind = leveldomain;
      
      leveldomain = to;
      to = from;
      leveldomainValid = validLeveldomain( leveldomain );
  }
  
  if( ! leveldomainValid )
      throw logic_error("LevelSpec: Expecting valuedomain!");
  
  
  if( ! validIndCode( ind ) && ! ind.empty() )
      throw logic_error("LevelSpec: Invalid indtermination code <" + ind + ">!");
  
  if( ind.empty() )
      ind = defIndCode;
  
  if( from == to ) {
	  if( !( ind == "exact" || ind == "below" || ind == "above" ) )  {
			valid_ = false;
			throw logic_error("LevelSpec: '" + ind +"' without meaning, only one level is given!");
	  }
	  
	  ost << "'" << ind << " " << from << " " << leveldomain << "'";
  } else {
	  ost << "'inside " << from << " TO " << to << " " << leveldomain << "'";
  }
  
  valid_ = true;
  selectPart_ = ost.str();
}

void 
UrlParamLevelSpec::
decodeProtocol1( const std::string &toDecode )
{
	const char *defIndCode="exact";
   ostringstream ost;
   string from;
   string to;
   string leveldomain;
   string ind(defIndCode);
   bool fromValid, toValid, indValid, leveldomainValid;
   
   string *tmp[]={ &from, &to, &leveldomain, &ind };
   vector<string> vals=miutil::splitstr(toDecode, ',');
   
   if( vals.empty() ) {
		valid_ = true;
		selectPart_ = "NULL";
		return;
   }
   
   if( vals.size() < 2 )
   	throw logic_error("LevelSpec: To few values!");
   
   if( vals.size() > 4 )
   	throw logic_error("LevelSpec: To many values!");
   
  for( vector<string>::size_type i=0; i<vals.size(); ++i )
      *tmp[i] = vals[i]; 
  
  miutil::trimstr( from, miutil::TRIMBOTH, " \t\r\n\"" );
  miutil::trimstr( to, miutil::TRIMBOTH, " \t\r\n\"" );
  miutil::trimstr( leveldomain,  miutil::TRIMBOTH, " \t\r\n\"" );
  miutil::trimstr( ind, miutil::TRIMBOTH, " \t\r\n\"");
  
  if( from.empty() && to.empty() && leveldomain.empty() && ind.empty() ){
      valid_ = true;
      selectPart_ = "NULL";
      return;
  }
  
  fromValid = validLevel( from );
  toValid = validLevel( to );
  leveldomainValid = validLeveldomain( leveldomain );
  indValid = validIndCode( ind );
  
  if(!fromValid && !toValid )
      throw logic_error("LevelSpec: Missing required levelfrom or levelto!");
  
  if( ! fromValid ) 
      from = to;
  else if( ! toValid ) {
      if( vals.size() == 3 ) 
	   ind = leveldomain;
      
      leveldomain = to;
      to = from;
      leveldomainValid = validLeveldomain( leveldomain );
  }
  
  if( ! leveldomainValid )
      throw logic_error("LevelSpec: Expecting valuedomain!");
  
  
  if( ! validIndCode( ind ) && ! ind.empty() )
      throw logic_error("LevelSpec: Invalid indtermination code <" + ind + ">!");
  
  if( ind.empty() )
      ind = defIndCode;
  
  ost << "(" << from << ", " << to << ", '" << leveldomain << "', '" << ind << "')";
  valid_ = true;
  selectPart_ = ost.str();
	
}
	

    
void 
UrlParamLevelSpec::
decode( const std::string &toDecode )
{
	if( protocol == 1 )
		decodeProtocol1( toDecode );
	else if( protocol == 2 )
		decodeProtocol2( toDecode );
	else if( protocol > 2 )
		decodeProtocol2( toDecode );
	else
		decodeProtocol1( toDecode );
}

} // namespace

/**
 * @}
 *
 * @}
 */
