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


#ifndef __UPDATEPROVIDERREFTIMES_H__
#define __UPDATEPROVIDERREFTIMES_H__

#include <map>
#include <list>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/shared_ptr.hpp>
#include <ProviderList.h>

#if 0
#include <boost/date_time/posix_time/time_serialize.hpp>
#include <boost/serialization/split_member.hpp>
#endif

#include <DbManager.h>

namespace wdb2ts {

struct ProviderTimes {
	boost::posix_time::ptime refTime;
	boost::posix_time::ptime updatedTime;
	bool disabled;
	int dataversion;

	ProviderTimes( ) : disabled( false), dataversion( -1 ) {}
	
	ProviderTimes( const boost::posix_time::ptime &refTime_,
	               const boost::posix_time::ptime &updatedTime_,
	               bool disabled_ = false,
	               int dataversion_ = -1 )
		: refTime( refTime_), updatedTime( updatedTime_), disabled( disabled_ ),
		  dataversion( dataversion_ ) {}
	
	ProviderTimes( const ProviderTimes &pt) 
		: refTime( pt.refTime ), updatedTime( pt.updatedTime ),
		  disabled( pt.disabled ),
		  dataversion( pt.dataversion ){}
	
	ProviderTimes& operator=( const ProviderTimes &rhs ) 
	{
		if( this != &rhs ) {
			refTime = rhs.refTime;
			updatedTime = rhs.updatedTime;
			disabled = rhs.disabled;
			dataversion = rhs.dataversion;
		}
		
		return *this;
	}

#if 0
	template<class Archive>
	void save(Archive & ar, const unsigned int version) const
	{
		ar & const_cast<const boost::posix_time::ptime&>( refTime ) 
		   & const_cast<const boost::posix_time::ptime&>( updatedTime )
		   & const_cast<const int&>( dataversion );	
	}

	template<class Archive>
	void load(Archive & ar, const unsigned int version)
	{
		ar & refTime & updatedTime & dataversion;
	}

	BOOST_SERIALIZATION_SPLIT_MEMBER();
#endif
};

class ProviderRefTimeList : public std::map<std::string, ProviderTimes >
{
public:
	ProviderRefTimeList(){}
	~ProviderRefTimeList(){}
	
	int getDataversion( const std::string &providerWithPlacename ) const;
	bool providerReftime( const std::string &provider,
			              boost::posix_time::ptime &refTime ) const;

	bool disabled( const std::string &provider, bool &disabled ) const;

};

//typedef std::map<std::string, ProviderTimes > ProviderRefTimeList;
//typedef std::map<std::string, std::string > TopoProviderMap;

//typedef std::map<std::string, std::list<std::string> > TopoProviderMap;
typedef boost::shared_ptr<ProviderRefTimeList> PtrProviderRefTimes;


/**
 * @exception std::logic_error on db failure.
 */

bool
updateProviderRefTimes( WciConnectionPtr wciConnection, 
		                ProviderRefTimeList &refTimes,
		                const ProviderList &providers,
		                int wciProtocol );

bool
updateProviderRefTimes( WciConnectionPtr wciConnection, 
						const ProviderRefTimeList &requestedUpdates,
		                ProviderRefTimeList &refTimes,
		                int wciProtocol );

void removeDisabledProviders( ProviderList &providers, const ProviderRefTimeList &reftimes );

}


#endif 
