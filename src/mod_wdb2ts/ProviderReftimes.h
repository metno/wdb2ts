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


#ifndef __PROVIDERREFTIMES_H__
#define __PROVIDERREFTIMES_H__

#include <map>
#include <list>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/shared_ptr.hpp>
#include <ProviderList.h>

#if 0
#include <boost/date_time/posix_time/time_serialize.hpp>
#include <boost/serialization/split_member.hpp>
#endif



namespace wdb2ts {

struct ProviderTimes {
	boost::posix_time::ptime refTime;
	boost::posix_time::ptime updatedTime;
	/**
	 * disableEnableRequest and dataversionRequest.
	 * This is only used when the class is used to pass information from the
	 * url request query to the decoding logic. It is not saved to file.
	 */
	bool disableEnableRequest;
	bool dataversionRequest;
	bool reftimeUpdateRequest;
	bool disabled;
	int dataversion;

	ProviderTimes( ) :
	   disableEnableRequest( false ),
	   dataversionRequest( false ),
	   reftimeUpdateRequest( false ),
	   disabled( false),
	   dataversion( -1 ) {}
	
	ProviderTimes( const boost::posix_time::ptime &refTime_,
	               const boost::posix_time::ptime &updatedTime_=boost::posix_time::second_clock::universal_time(),
	               bool disabled_ = false,
	               int dataversion_ = -1,
	               bool disableEnableRequest_ = false,
	               bool dataversionRequest_ = false,
	               bool reftimeUpdateRequest_ = false )
		: refTime( refTime_),
		  updatedTime( updatedTime_),
		  disableEnableRequest( disableEnableRequest_ ),
		  dataversionRequest( dataversionRequest_ ),
		  reftimeUpdateRequest( reftimeUpdateRequest_ ),
		  disabled( disabled_ ),
		  dataversion( dataversion_ ) {}
	
	ProviderTimes( const ProviderTimes &pt) 
		: refTime( pt.refTime ), updatedTime( pt.updatedTime ),
		  disableEnableRequest( pt.disableEnableRequest ),
		  dataversionRequest( pt.dataversionRequest ),
		  reftimeUpdateRequest( pt.reftimeUpdateRequest ),
		  disabled( pt.disabled ),
		  dataversion( pt.dataversion ){}
	
	ProviderTimes& operator=( const ProviderTimes &rhs ) 
	{
		if( this != &rhs ) {
			refTime = rhs.refTime;
			updatedTime = rhs.updatedTime;
			disableEnableRequest = rhs.disableEnableRequest;
			dataversionRequest = rhs.dataversionRequest;
			reftimeUpdateRequest = rhs.reftimeUpdateRequest;
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

	bool providerReftimeDisabledAndDataversion( const std::string &provider,
	                                            boost::posix_time::ptime &refTime,
	                                            bool &disabled,
	                                            int &dataversion ) const;

	/**
	 * updateDisableStatus updates the disable status for the requested provider.
	 * If the requested provider do not have a placename all providers with
	 * provider name is updated.
	 *
	 * @param provider The provider to update, possibly with a nameplace.
	 * @param disable The disable status to set.
	 * @return The number of providers updated.
	 */
	int updateDisableStatus( const std::string &provider, bool disable );

	/**
    * updateDataversion updates the dataversion for the requested provider.
 	 * If the requested provider do not have a placename all providers with
	 * provider name is updated.
	 *
	 * @param provider The provider to update, possibly with a nameplace.
	 * @param dataversion The dataversion to set.
	 * @return The number of providers updated.
	 */
	int updateDataversion( const std::string &provider, int dataversion );

	bool disabled( const std::string &provider, bool &disabled ) const;
	bool dataversion( const std::string &provider, int &dataversion ) const;

};

//typedef std::map<std::string, ProviderTimes > ProviderRefTimeList;
//typedef std::map<std::string, std::string > TopoProviderMap;

//typedef std::map<std::string, std::list<std::string> > TopoProviderMap;
typedef boost::shared_ptr<ProviderRefTimeList> PtrProviderRefTimes;


void removeDisabledProviders( ProviderList &providers, const ProviderRefTimeList &reftimes );

}


#endif 
