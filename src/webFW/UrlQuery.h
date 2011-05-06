#ifndef __URLQUERY_H__
#define __URLQUERY_H__

#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <map>
#include <list>
#include <stdexcept>

namespace webfw {

/**
 * UrlQuery is a helper class to decode the query part 
 * of an url. 
 */
class UrlQuery {
	
	std::map<std::string, std::string> params;
	char seperator;
	std::string path;

public:

	/**
	 * What is used as separation character between the
	 * parameters in the query.
	 * 
	 * Valid values can be given as paramtere 'validSeparationChar'.
	 * The characters '&' or ';' is usual used. The character
	 * ';' is reconmended by W3C. Remeber that entititys starts 
	 * with '&'.
	 * 
	 * @return The separation character on success and
	 *         0 if none of the separation characters is found.
	 */
	static char getParamSeparator( const std::string &urlQuery, 
			                         const char *validSeparationChars=";&" );

	/**
	 * Decodes a string escaped using %xx URL encoding.
	 * NOTE: Must be used AFTER splitting into key/value pairs.
	 * @return The string s with %xx encoded characters replaced 
	 *    with the character.
	 * 
	 * @throws std::range_error if an conversion cant be done because of to few characters in the string s.
	 * @throws std::logic_error if an %xx is found, but xx is not a valid hexadecimal value.
	 */
	static std::string unescape( const std::string &s );

	/**
	 * Escape using %xx URL encoding.
	 * 
	 * @param s the string to escape.
	 * @return the escped string.
	 */
	static std::string escape( const std::string &s );

	UrlQuery( const UrlQuery &uq );
	UrlQuery& operator=( const UrlQuery &rhs );
		
	
	UrlQuery();
	
	/**
	 * @throws std::logic_error
	 * @throws std::range_error
	 */
	UrlQuery( const std::string &urlQuery );
	
	char setParamSeperator( char seperator, bool ifNotSet=true );
	
	char paramSeperator() const { return seperator; }
	
	std::string getPath() const { return path; }
	
	/**
	 * Check if the keys in the paramsList is
	 * in the urlQuery. 
	 * 
	 * @param paramList A list of keys to check for exsistence in the
	 *    urlQuery.
	 * @return an empty string if all keys in the list is in the urlQuery. 
	 *    If not, a string with the first element in paramsList not in the 
	 *    urlquery is returned. 
	 */
	std::string
	hasParams( const std::list<std::string> &paramsList )const;

	
	/**
	 * Check if the key is in the urlQuery. 
	 * 
	 * @param param The key to check for exsistence in the
	 *    urlQuery.
	 * @return true if the param is in the urlQuery and false otherwise.
	 */
	bool	hasParam( const std::string &param )const;


	/**
	 * @returns a list of the keys it the query.
	 */
	std::list<std::string> keys() const;
	
	
	/**
	 * Decodes a query on the form 
	 * 
	 * param=val;param2=val2 or param=val&param2=val2
	 * 
	 * @param withPath Search for an '?' character. The path is the
	 *    string before '?' and the query part is the string after.
	 * @param urlQuery The query to decode.
	 * @see UrlQuery::unescape( const std::string &s )
	 * @throws std::logic_error
	 * @throws std::range_error
	 */
	void decode( const std::string &urlQuery, bool withPath = false );	

	/**
	 * @throws std::logic_error if the paramname dont exist.
	 */
	std::string asString(const std::string &paramname )const;
	
	/**
	 * Return the defValue if the param dos not exist.
	 */
	std::string asString(const std::string &paramname, const std::string &defValue )const;
	
	/**
	 * @throws std::logic_error if the paramname dont exist.
	 * @throws std::bad_cast if the param value can not be converted to 
	 *     an float.
	 */
   float asFloat(const std::string &paramname )const;
   
   /**
    * Return the defValue if the param dos not exist.
    * @throws std::bad_cast if the param value can not be converted to 
    *     an float.
    */
   float asFloat(const std::string &paramname, float defValue )const;

   /**
    * Return the defValue if the param does not exist.
    * 
  	 * @throws std::logic_error if the paramname dont exist.
  	 * @throws std::bad_cast if the param value can not be converted to 
	 *     an int.
  	 */
   int asInt(const std::string &paramname )const;
   
   /**
    * Return the defValue if the param does not exist.
    * @throws std::bad_cast if the param value can not be converted to 
    *     an int.
    */
   int asInt(const std::string &paramname, int defValue )const;


   /**
    * Return the value as an boolean. The following
    * values is accepted as boolean values.
    *
    * true, false, TRUE, FALSE, 0 and 1.
    *
    * Return the defValue if the param does not exist.
    *
    * @throws std::bad_cast if the param value can not be converted to
    *     an bool.
    */
   bool asBool(const std::string &paramname, bool defValue )const;


   /**
    * Return the value as an boolean. The following
    * values is accepted as boolean values.
    *
    * true, false, TRUE, FALSE, 0 and 1.
    *
    * Return the defValue if the param does not exist.
    *
    * @throws std::logic_error if the paramname dont exist.
    * @throws std::bad_cast if the param value can not be converted to
    *     an bool.
    */
   bool asBool(const std::string &paramname )const;

   /**
  	 * @throws std::logic_error if the paramname dont exist.
  	 */
   boost::posix_time::ptime asPTime( const std::string &paramname )const;
   
   /**
    * Return the defValue if the param dos not exist.
    * 
    * @throws std::logic_error if the param value can not be converted to 
    *     an ptime.
    */
   boost::posix_time::ptime asPTime( const std::string &paramname, 
   		                            const boost::posix_time::ptime defValue )const;
   
   void setValue( const std::string &param, float value );
   void setValue( const std::string &param, int value );
   void setValue( const std::string &param, const std::string &value );
   void setValue( const std::string &param, const boost::posix_time::ptime &value );
   
   std::string encode( const std::string &path ) const;
};


}

#endif
