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


#ifndef __METAMODELCONF_H__
#define __METAMODELCONF_H__
#include <list>
#include <map>
#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <iostream>
#include <Config.h>

namespace wdb2ts {

typedef std::list<boost::posix_time::time_duration> TimeDurationList;

/**
 * MataModelConf contains informations about the metadata
 * in the meta tag for the met.no XML on the wire XML location
 * forecast format.
 */
class MetaModelConf
{
	std::string name_;	
	
	typedef enum  {Error, Undefined, Absolute, RelativeToLoadTime} SpecType;
	SpecType specType;
	///Sorted list of time_durations.
	TimeDurationList nextrun_; 
	
	bool isnumber( const std::string &val );
	bool parseNextrun( const std::string &val );
	SpecType parseNextrun1( const std::string &val );
	SpecType parseNextrun2( const std::string &val1, const std::string &val2 );

public:
	MetaModelConf();
	MetaModelConf( const std::string &name );
	MetaModelConf( const MetaModelConf &m );
	
	MetaModelConf& operator=( const MetaModelConf &rhs );
	
	/**
	 * Parse conf on the form
	 *   name=YR;nextrun=(18:00,14:00)
	 */
	bool parseConf( const std::string &conf );

	std::string name()const { return name_; }
	TimeDurationList nextrun() const { return nextrun_; }
	boost::posix_time::ptime findNextrun(const boost::posix_time::ptime &refTime=boost::posix_time::ptime() )const;
	void addTimeDuration( const boost::posix_time::time_duration &td, MetaModelConf::SpecType specType=MetaModelConf::Absolute );
	
	friend std::ostream& operator<<( std::ostream &o, const MetaModelConf &c );
	friend std::ostream& operator<<( std::ostream &o, const MetaModelConf::SpecType c );

	///Only used for testing;
	static void setNowTimeForTest(const boost::posix_time::ptime &nowTime);
};

typedef std::map<std::string, MetaModelConf > MetaModelConfList;

MetaModelConfList
configureMetaModelConf( const wdb2ts::config::ActionParam &params );


std::ostream& 
operator<<( std::ostream &o, const MetaModelConf &c );

std::ostream&
operator<<( std::ostream &o, const MetaModelConf::SpecType c);


}

#endif
