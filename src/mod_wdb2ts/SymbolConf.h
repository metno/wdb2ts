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

#ifndef __SYMBOLCONF_H__
#define __SYMBOLCONF_H__

#include <string>
#include <vector>
#include <map>
#include <RequestConf.h>
#include <DbManager.h>

namespace wdb2ts {

class Wdb2TsApp;
class SymbolConf;

typedef std::vector<SymbolConf> SymbolConfList;

class SymbolConf
{
	int min_;
	int max_;
	int precipHours_;
	
public:
	SymbolConf();
	SymbolConf( int min, int max, int precipHours );
	SymbolConf( const SymbolConf &sf );
	
	SymbolConf& operator=( const SymbolConf &rhs );
	int min()const { return min_; }
	int max()const { return max_; }
	int precipHours()const  { return precipHours_; }
	
	bool operator==( const SymbolConf &rhs );
	bool operator!=( const SymbolConf &rhs );
	
	static bool parse( const std::string &buf, SymbolConfList &conf );
	friend std::ostream &operator<<( std::ostream &ost, const SymbolConf &conf );
};

std::ostream &operator<<( std::ostream &ost, const SymbolConf &conf );

class SymbolConfProvider :
		protected std::map<std::string, SymbolConfList>
{
public:

	typedef std::map<std::string, SymbolConfList> Conf;
	typedef Conf::value_type value_type;
	typedef Conf::key_type key_type;
	typedef Conf::const_pointer const_pointer;
	typedef Conf::const_reference const_reference;
	typedef std::map<std::string, SymbolConfList>::iterator iterator;
	typedef std::map<std::string, SymbolConfList>::const_iterator const_iterator;

	SymbolConfProvider();
//	SymbolConfProvider( const SymbolConfProvider &conf )
//		: defaultConf( conf.defaultConf), maxHours_( conf.maxHours_){}
//
//	SymbolConfProvider& operator=( const SymbolConfProvider &rhs ) {
//		if( this != &rhs ) {
//			defaultConf = rhs.defaultConf;
//			maxHours_ = rhs.maxHours_;
//			*static_cast<Conf*>(this) = static_cast<const Conf&>(rhs);
//		}
//		return *this;
//	}


	void setDefaultConf( const SymbolConfList &conf ) { defaultConf = conf; }
	SymbolConfList getDefaultConf( const SymbolConfList &conf ) { return defaultConf; }


	void clear(){ maxHours_ = 0; Conf::clear(); }
	void add( const std::string &provider, const SymbolConfList &conf );
	bool empty()const { return  std::map<std::string, SymbolConfList>::empty(); }

	/**
	 * get try to find a symbol configuration for the provider. If
	 * it do not find the configuration and the provider has a placename,
	 * the placename is stripped from the provider, and it is searched for
	 * a configuration without the placename. If no configuration is
	 * found the default configuration is returned.
	 */
	SymbolConfList get( const std::string &provider )const;

	const_iterator begin()const { return Conf::begin(); }
	const_iterator end()const { return Conf::end(); }

	const_iterator find( const std::string &provider )const { return Conf::find( provider); }
	/**
	 * Return the maximum numbers of hours we generate a symbol over
	 * independent of providers.
	 */
	int maxHours() const { return maxHours_; }

	void merge(const SymbolConfProvider &other);

private:
	SymbolConfList defaultConf;
	int maxHours_;
};



void
configureSymbolconf( const wdb2ts::config::ActionParam &params, 
		               SymbolConfProvider &symbolConfProvider );
}

#endif 
