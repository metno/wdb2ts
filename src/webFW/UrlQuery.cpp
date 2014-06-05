#include <stdio.h>
#include <ctype.h>
#include <sstream>
#include <splitstr.h>
#include <trimstr.h>
#include <ptimeutil.h>
#include "UrlQuery.h"


using namespace std;

namespace webfw {

char  
UrlQuery::
getParamSeparator( const std::string &urlQuery,
						 const char *validSeparationChars)
{
	string::size_type i=urlQuery.find_first_of( validSeparationChars );
	
	if( i != string::npos )
		return urlQuery[i];
	
	return -1;
}

UrlQuery::
UrlQuery( )
	: seperator( -1 )
{
}

UrlQuery::
UrlQuery( const UrlQuery &uq )
	: params( uq.params ), seperator( uq.seperator ), path( uq.path )
{
	
}

UrlQuery& 
UrlQuery::
operator=( const UrlQuery &rhs )
{
	if( this != &rhs ) {
		params = rhs.params;
		seperator = rhs.seperator;
		path = rhs.path;
	}
	
	return *this;
}


UrlQuery::
UrlQuery( const std::string &urlQuery )
	: seperator( -1 )
{
	decode( urlQuery );
}


char 
UrlQuery::
setParamSeperator( char sep, bool ifNotSet )
{
	char oldSep = seperator;
	
	if( ifNotSet ) { 
		if( seperator <= 0 )
			seperator = sep;
	} else {
		seperator = sep;
	}
	
	return oldSep;
}

/**
 * Decodes a string escaped using %xx URL encoding.
 * NOTE: Must be used AFTER splitting into key/value pairs.
 */
std::string
UrlQuery::
unescape( const std::string &s )
{  
	string result;
	string::size_type i = 0;
	char buf[4];
	char *end;
	long number;
	
	for( i = 0; i < s.length(); ++i ) {
		if( s[i] == '%' ) {
			if( (i+3) > s.length() )
				throw std::range_error("Format error: Expecting at least 2 chars after an % character.");
		
			s.copy( buf, 2, i+1 );
			buf[2] = '\0';
			number = strtol( buf, &end, 16 );
			
			if( *end != '\0')
				throw std::logic_error("Format error: Expecting an hexadecimal value on the form %xx. Given value '" + string( buf ) +"'.");
		
			result += static_cast<char>( number );
			i += 2;
		} else {
			result += s[i];
		}
	}

	return result;
}

std::string
UrlQuery::
escape( const std::string &s )
{
	ostringstream ost;
	char ch;
	char buf[4];
	
	for( string::size_type i = 0; i < s.length(); ++i ) {
		ch = s[i];
		
		if( isalnum( ch ) ) {
			ost << ch;
		} else {
			sprintf( buf, "%%%02x", ch );
			ost << buf;
		}
	}
	
	return ost.str();
}

void 
UrlQuery::
decode( const std::string &urlQuery_, bool withPath )
{
	vector<string> paramList;
	vector<string> keyVal;
	string urlQuery( urlQuery_ );

	if( withPath ) {
		path.erase();
		string::size_type iPath = urlQuery.find("?");

		if( iPath != string::npos ) {
			path = urlQuery.substr(0, iPath );
			miutil::trimstr( path );

			if( ! path.empty() && path[ path.length() -1 ] == '/' )
				path.erase( path.length() - 1 );

			urlQuery.erase( 0, iPath + 1 );
		} else {
			path = urlQuery;
			urlQuery.erase();
		}
	}

	if( urlQuery.empty() )
		return;

	seperator = getParamSeparator( urlQuery );

	if( seperator > 0 )
		paramList = miutil::splitstr( urlQuery, seperator );
	else
		paramList.push_back( urlQuery );

	for( vector<string>::size_type i=0; i<paramList.size(); ++i ) {
		string val;
		string key;
		if( paramList[i].length() == 0 )
			continue;

		keyVal = miutil::splitstr( paramList[i], '=');

		if( keyVal.size() == 0 )
			continue;

		key = unescape( keyVal[0] );

		if( keyVal.size() <= 2 ) {
			if( keyVal.size() == 2 ) {
				val = unescape( keyVal[1] );
			}
		} else {
			ostringstream ost;

			ost << "Invalid key=val spec: " << paramList[i] << ". Number of '=' ";
			if( keyVal.size() == 1)
				ost << "0";
			else
				ost << keyVal.size()-1;

			ost << ".";

			throw logic_error( ost.str() );
		}

		params[ key ] = val;
	}
}
	
bool
UrlQuery::
hasParam( const std::string &param )const
{
	map<string,string>::const_iterator it=params.find( param );
	
	if( it != params.end() )
		return true;
	
	return false;
}

std::string
UrlQuery::
hasParams( const std::list<std::string> &paramsList )const
{
	std::list<string>::const_iterator itList;
	map<string,string>::const_iterator itParams;
	
	for( itList=paramsList.begin(); itList!=paramsList.end(); ++itList ) {
		itParams = params.find( *itList );

		if( itParams == params.end() ) 
			return *itList;
	}
		
	return "";
}


std::list<std::string> 
UrlQuery::
keys() const
{
	std::list<string> retList;
	map<string,string>::const_iterator itParams;
	
	for( itParams = params.begin(); itParams != params.end(); ++itParams ) 
		retList.push_back( itParams->first );		
	
	return retList;
}


std::string 
UrlQuery::
asString(const std::string &paramname )const
{
	map<string,string>::const_iterator it=params.find( paramname );
	
	if( it == params.end() )
		throw logic_error("Param '"+paramname+"' missising in query.");
	
	return it->second;
}


std::string
UrlQuery::
asString(const std::string &paramname, const std::string &defValue )const
{
	map<string,string>::const_iterator it=params.find( paramname );
		
	if( it == params.end() )
		return defValue;
		
	return it->second;
	
}

float
UrlQuery::
asFloat(const std::string &paramname )const
{
	std::string val = asString( paramname );
	
	try {
		return boost::lexical_cast<float>(val);
	}
	catch( ... ) {
		throw logic_error("Param '"+paramname+"' value ("+val+") not convertibel to float!");
	}
}


float 
UrlQuery::
asFloat(const std::string &paramname, float defValue )const
{
	std::string val = asString( paramname, "" );
		
	try {
		if( val.empty() )
			return defValue;
		
		return boost::lexical_cast<float>(val);
	}
	catch( ... ) {
		throw logic_error("Param '"+paramname+"' value ("+val+") not convertibel to float!");
   }
}

int
UrlQuery::
asInt(const std::string &paramname )const
{
	std::string val = asString( paramname );
		
	try {
		return boost::lexical_cast<int>(val);
	}
	catch( ... ) {
		throw logic_error("Param '"+paramname+"' value ("+val+") not convertibel to int!");
   }
}

int
UrlQuery::
asInt(const std::string &paramname, int defValue )const
{
	std::string val = asString( paramname, "" );
	
	try {
		if( val.empty() )
			return defValue;
	
		return boost::lexical_cast<int>(val);
	}
	catch( ... ) {
		throw logic_error("Param '"+paramname+"' value ("+val+") not convertibel to int!");
	}
}









bool
UrlQuery::
asBool(const std::string &paramname )const
{
   std::string val = asString( paramname );

   if( val.empty() || val=="T" || val == "t" || val == "true" || val=="TRUE"
       || val == "1" )
      return true;
   else if( val == "F" || val == "f" || val == "false" ||
         val == "FALSE" || val == "0" )
      return false;
   else
      throw std::bad_cast();
}

bool
UrlQuery::
asBool(const std::string &paramname, bool defValue )const
{
   std::string val = asString( paramname, "__NO_PARAM__" );

   if( val == "__NO_PARAM__" )
      return defValue;

   if( val.empty() || val=="T" || val == "t" || val == "true" || val=="TRUE"
       || val == "1" )
      return true;
   else if( val == "F" || val == "f" || val == "false" ||
         val == "FALSE" || val == "0" )
      return false;
   else
      throw std::bad_cast();
}







boost::posix_time::ptime 
UrlQuery::
asPTime( const std::string &paramname )const
{
	std::string val = asString( paramname );

	if( val.empty() )
		throw logic_error("Param '"+paramname+"' no value.");
	
	try {
		return miutil::ptimeFromIsoString( val );
	}
	catch( logic_error &ex) {
		throw logic_error("Param '"+paramname+"' value ("+val+") not a valid time! ("+ex.what()+")");
	}
	catch( ... ) {
		throw logic_error("Param '"+paramname+"' value ("+val+") not a valid time!");
	}
}


boost::posix_time::ptime 
UrlQuery::
asPTime( const std::string &paramname, 
         const boost::posix_time::ptime defValue )const
{
	try {
		return asPTime( paramname );
	}
	catch( ... ) {
		return defValue;
	}
}


void 
UrlQuery::
setValue( const std::string &param, float value )
{
	ostringstream ost;
	
	ost.precision( 6 );
	ost.setf( ios::floatfield, ios::fixed );
	
	ost << value;
	
	setValue( param, ost.str() );
}

void 
UrlQuery::
setValue( const std::string &param, int value )
{
	ostringstream ost;
	
	ost << value;
	
	setValue( param, ost.str() );
}

void 
UrlQuery::
setValue( const std::string &param, const std::string &value )
{
	if( ! param.empty() )
		params[ param ] = value;
}

void 
UrlQuery::
setValue( const std::string &param, const boost::posix_time::ptime &value )
{
	setValue( param, miutil::isotimeString( value, true, true ) );	
}
   
std::string 
UrlQuery::
encode( const std::string &path ) const
{
	ostringstream query;
	
	query << path;
	
	if( params.empty() )
		return query.str();
	else 
		query << "?";

	char sep;
	
	if( seperator <= 0 )
		sep = ';';
	
	map<string,string>::const_iterator itParams;
		
	for( itParams = params.begin(); itParams != params.end(); ++itParams ) {
		if( itParams != params.begin() )
			query << sep;
		
		query << escape( itParams->first ) << "=" << escape( itParams->second );
	}

	return query.str();
}


} //namespace webfw


