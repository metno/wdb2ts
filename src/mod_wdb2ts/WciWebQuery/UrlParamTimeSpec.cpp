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
 * Implementation of the UrlParamTimeSpec class.
  */
#ifdef HAVE_CONFIG_H
#include <wdb2ts_config.h>
#endif
// CLASS
#include <UrlParamTimeSpec.h>
// PROJECT INCLUDES
//
// SYSTEM INCLUDES
#include <iostream>
#include <sstream>
#include <vector>
#include <splitstr.h>
#include <trimstr.h>
#include <ptimeutil.h>

using namespace std;

namespace wdb2ts {

using namespace boost::posix_time;

UrlParamTimeSpec::
UrlParamTimeSpec( ) : isDecoded( false )
{
	
    valid_ = true;
    selectPart_ = "NULL";
}

UrlParamTimeSpec::
UrlParamTimeSpec( int protocol ) 
	: UrlParam( protocol ), isDecoded( false )
{
	
    valid_ = true;
    selectPart_ = "NULL";
}


void 
UrlParamTimeSpec::
clean() 
{
	isDecoded = false;
	valid_ = true;
   selectPart_ = "NULL";
   fromTime = ptime();
   toTime = ptime();
   indCode.erase();
   sameTimespecAsProvider.erase();
   useProviderList_ = false;
     
}
   
/**
 * @TODO implement this properly.
 */
bool 
UrlParamTimeSpec::
validDate( const std::string &dateString )
{
    if( dateString.empty() )
	return false;
    
    string::size_type i=dateString.find_first_not_of("1234567890-:TtZ ");
    
    if( i != string::npos || ! isdigit( dateString[0] ) )
	return false;
    
    return true;
}
    
bool 
UrlParamTimeSpec::
validIndCode( const std::string &indCode )
{
	
    if ( indCode == "exact" ) 
    	return true;
    else if ( indCode == "any" && protocol == 1 ) 
    	return true;
    else if ( indCode == "inside" ) 
    	return true;
    else if ( indCode == "intersect" && protocol == 1 ) 
    	return true;
    
    if( protocol > 1 ) { 
   	 if ( indCode == "before" )
   		 return true;
   	 else if ( indCode == "after" )
   		 return true;
    }
  
  	return false;
}

void 
UrlParamTimeSpec::
decodeProtocol1( const std::string &toDecode )
{
	const char *defIndCode="exact";
	ostringstream ost;
	string from;
	string to;
	string::size_type iSameTimeSpec;
	string ind(defIndCode);
	bool fromValid, toValid;

	isDecoded = true;
 
	iSameTimeSpec = toDecode.find( "${" );
 
	if( iSameTimeSpec != string::npos ) {
		string::size_type end= toDecode.find( "}", iSameTimeSpec );
	 
		if( end == string::npos ) 
			throw logic_error("TimeSpec: Expecting '}' in provider specification!");
	 
		sameTimespecAsProvider = toDecode.substr( iSameTimeSpec+2, end-iSameTimeSpec-2 );
		miutil::trimstr( sameTimespecAsProvider );
		
		if( sameTimespecAsProvider == "$dataprovider" ) {
			sameTimespecAsProvider.erase();
			useProviderList_ = true;
		}
		
		selectPart_ = "NULL";
		return;
	}
 
	fromTime = ptime();
	toTime = ptime();
	indCode.erase();

	string *tmp[]={ &from, &to, &ind };
	vector<string> vals=miutil::splitstr(toDecode, ',');

	if ( vals.empty() ) {
		valid_ = true;
		selectPart_ = "NULL";
		return;
	}

	if ( vals.size() > 3 )
		throw logic_error("TimeSpec: To many values!");

	for ( vector<string>::size_type i=0; i<vals.size(); ++i )
		*tmp[i] = vals[i]; 

	miutil::trimstr( from );
	miutil::trimstr( to );
	miutil::trimstr( ind );

	if ( from.empty() && to.empty() && ind.empty() ){
		valid_ = true;
		selectPart_ = "NULL";
		return;
	}

	if( from == "NULL" || to == "NULL" ) {
		valid_ = true;
		selectPart_ = "NULL";
		isDecoded = true;
		return;
	}
	
	fromValid = validDate( from );
	toValid = validDate( to );

	if ( vals.size() == 1 && ! fromValid)
		throw logic_error("TimeSpec: Invalid fromtime!");

	if ( ! fromValid ) {
		from = to;
	}else if ( ! toValid ) {
		if ( vals.size() == 2 ) {
			ind = to;
		}
   
		to = from;
	}

	if ( ! validDate( from ) && ! validDate( to ))
		throw logic_error("TimeSpec: Invalid fromtime or totime!");

	if ( ! validIndCode( ind ) && ! ind.empty() )
		throw logic_error("TimeSpec: Expecting valid indtermination code!");

	if ( ind.empty() )
		ind = defIndCode;

	fromTime = miutil::ptimeFromIsoString( from );
	toTime = miutil::ptimeFromIsoString( to );
	indCode = ind;

	ost << "('" << from << "', '" << to << "', '" << ind << "')";
	valid_ = true;
	selectPart_ = ost.str();
}

void 
UrlParamTimeSpec::
decodeProtocol2( const std::string &toDecode )
{
	const char *defIndCode="exact";
	ostringstream ost;
	string from;
	string to;
	string::size_type iSameTimeSpec;
	string ind(defIndCode);
	bool fromValid, toValid;

	isDecoded = true;
 
	iSameTimeSpec = toDecode.find( "${" );
 
	if( iSameTimeSpec != string::npos ) {
		string::size_type end= toDecode.find( "}", iSameTimeSpec );
	 
		if( end == string::npos ) 
			throw logic_error("TimeSpec: Expecting '}' in provider specification!");
	 
		sameTimespecAsProvider = toDecode.substr( iSameTimeSpec+2, end-iSameTimeSpec-2 );
		miutil::trimstr( sameTimespecAsProvider );
		
		if( sameTimespecAsProvider == "$dataprovider" ) {
			sameTimespecAsProvider.erase();
			useProviderList_ = true;
		}

		selectPart_ = "NULL";
		return;
	}
 
	fromTime = ptime();
	toTime = ptime();
	indCode.erase();

	string *tmp[]={ &from, &to, &ind };
	vector<string> vals=miutil::splitstr(toDecode, ',');

	if ( vals.empty() ) {
		valid_ = true;
		selectPart_ = "NULL";
		return;
	}

	if ( vals.size() > 3 )
		throw logic_error("TimeSpec: To many values!");

	for ( vector<string>::size_type i=0; i<vals.size(); ++i )
		*tmp[i] = vals[i]; 

	miutil::trimstr( from );
	miutil::trimstr( to );
	miutil::trimstr( ind );

	if ( from.empty() && to.empty() && ind.empty() ){
		valid_ = true;
		selectPart_ = "NULL";
		return;
	}

	if( from == "NULL" || to == "NULL" ) {
		valid_ = true;
		selectPart_ = "NULL";
		isDecoded = true;
		return;
	}
	
	fromValid = validDate( from );
	toValid = validDate( to );

	if ( vals.size() == 1 && ! fromValid)
		throw logic_error("TimeSpec: Invalid fromtime!");

	if ( ! fromValid ) {
		from = to;
	}else if ( ! toValid ) {
		if ( vals.size() == 2 ) {
			ind = to;
		}
   
		to = from;
	}

	if ( ! validDate( from ) && ! validDate( to ))
		throw logic_error("TimeSpec: Invalid fromtime or totime!");

	if ( ! validIndCode( ind ) && ! ind.empty() )
		throw logic_error("TimeSpec: Expecting valid indtermination code!");

	if ( ind.empty() )
		ind = defIndCode;

	fromTime = miutil::ptimeFromIsoString( from );
	toTime = miutil::ptimeFromIsoString( to );
	indCode = ind;

	if( fromTime == toTime ) {
		if( ! (ind == "exact" || ind == "before" || ind == "after")  ) {
			valid_ = false;
			throw logic_error("TimeSpec: '" + ind +"' without meaning, only one timestamp given!");
		}
		
		ost << "'" << ind << " " << from <<"'";
	} else {
		ost << "'inside " << from << " TO " << to << "'";
	}
	
	valid_ = true;
	selectPart_ = ost.str();
}


void 
UrlParamTimeSpec::
decode( const std::string &toDecode )
{
	useProviderList_ = false;
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
