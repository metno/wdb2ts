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


#ifndef __PROVIDER_LIST_H__
#define __PROVIDER_LIST_H__

#include <map>
#include <vector>
#include <RequestConf.h>

namespace wdb2ts {

class Wdb2TsApp;

struct ProviderItem {
	std::string provider;
	std::string placename;
	
	ProviderItem(){}
		
	explicit ProviderItem( const std::string &provider_ )
		: provider( provider_ )
		{}
	
	explicit ProviderItem( const std::string &provider_, 
			               const std::string &placename_ )
			: provider( provider_ ), placename( placename_ )
			{}
		
	
	ProviderItem( const ProviderItem &pi )
		: provider( pi.provider ), placename( pi.placename )
		{}
	
	ProviderItem& operator=( const ProviderItem &rhs ) {
		if( this != &rhs ) {
			provider = rhs.provider;
			placename = rhs.placename;
		}
		return *this;
	}
	
	bool operator==( const ProviderItem &rhs ) {
			if( provider == rhs.provider && placename == rhs.placename )
				return true;
			return false;
		}

	bool operator!=( const ProviderItem &rhs ) {
				if( ! (provider == rhs.provider && placename == rhs.placename) )
					return true;
				return false;
			}

	std::string providerWithPlacename() const {
		if( placename.empty() )
			return provider;
		
		return std::string( provider + " [" + placename + "]" );
	}
};

class ProviderList : public std::vector<ProviderItem> 
{
public:
	ProviderList(){};
	
	/**
	 * Find a provider/placename definition.
	 * 
	 * The placename is from the database and is on the form
	 * 
	 *  POINT( ... ) placename,
	 * 
	 * 	we are only intrested in the placename part.
	 * 
	 * If there is no match for the placename, and the placename 
	 * part in the list is unset, the first item in the list
	 * with a provider equal to the searched provider is returned.
	 * 
	 * @param[in] provider from the database.
	 * @param[in] pointPlacename On input 'POINT(...) placename'.
	 * @param[out] providerWithPlacename A string on the form 'provider [placename]'.
	 */

	const_iterator findProvider( const std::string &provider, 
			                       const std::string &pointPlacename, 
			                       std::string &providerWithplacename )const; 

	const_iterator findProvider( const std::string &providerWithPlacename )const; 
			                        

	/**
	 * Decode a string on the form 'provider [placename0, placename1, .., placenameN]'
	 * to a ProviderList. If placename contain the character ',' it must be protected
	 * with a ' characters. Ex if the placename is:  1440,721 res 0.25 start -179.75,-90
	 * it must be giaven as '1440,721 res 0.25 start -179.75,-90'.
	 *
	 */
	static ProviderList decode( const std::string &toDecode );
	static ProviderList decode( const std::string &toDecode, std::string &provider );

	static ProviderItem decodeItem( const std::string &toDecode );

	ProviderList providerListWithoutPlacename() const;
	std::list<std::string> providerWithoutPlacename() const;
};


/**
 * For providers that is not defined with a placename, search the database and 
 * set the placename. The priority for the placename for these providers is
 * unpredictable.
 */
ProviderList
providerPrioritySetPlacename( const ProviderList &pvList, 
										const std::string &wdbDB,
										Wdb2TsApp *app );

ProviderList
configureProviderList( const wdb2ts::config::ActionParam &params, 
		                 const std::string &wdbDB,
		                 Wdb2TsApp *app );


}
#endif 
