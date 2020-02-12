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

#ifndef __NEARESTHEIGHT_H__
#define __NEARESTHEIGHT_H__


#include <string>
#include <map>
#include <list>
#include <ostream>

#include <configparser/RequestConf.h>
#include <DbManager.h>
#include <LocationData.h>
#include <UpdateProviderReftimes.h>
#include <ParamDef.h>

namespace wdb2ts {

class NearestHeightConfElement
{
	std::map< std::string, std::string> paramWithProvider;
	std::string topoProvider_;
	std::string modelTopoProvider_;
	std::string renameTo_;
	std::string wdbid_;

	friend class NearestHeight;

public:
	NearestHeightConfElement(){};

	std::map< std::string, std::string> params()const { return paramWithProvider; }
	bool rename() const { return ! renameTo_.empty(); }
	std::string renameTo() const { return renameTo_;}
	std::string topoProvider() const { return topoProvider_; }
	std::string modelTopoProvider() const { return modelTopoProvider_; }
	std::string wdbid()const { return wdbid_;}
};

typedef std::map< std::string, NearestHeightConfElement > NearestHeights;

class NearestHeight
{
	static void decode( const wdb2ts::config::ActionParam &conf,
			            const std::string &prefix,
			            wdb2ts::NearestHeights &nearestHeight,
						const std::string &defaultDbId,
			            std::ostream &msg );

public:
	static NearestHeights configureNearestHeight( const wdb2ts::config::ActionParam &conf, const std::string &defaultDbId );
	static void processNearestHeightPoint( ConfigData *config,
											const LocationPointList &locationPoints,
	                                       const boost::posix_time::ptime &to,
	                                       LocationPointDataPtr data,
	                                       int altitude,
	                                       const ProviderList &providerPriority,
	                                       const ParamDefList &params,
	                                       const NearestHeights &nearestHeights,
	                                       int wciProtocol);

};

}

#endif
