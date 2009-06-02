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

#ifndef __SYMBOLCONTEXT_H__
#define __SYMBOLCONTEXT_H__

#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <SymbolHolder.h>

namespace wdb2ts {

class SymbolContext 
{
	struct symTmp_ {
		boost::posix_time::ptime time;
		int symNumber;
		float latitude;
		float probability;
	} symTmp;
	
	ProviderSymbolHolderList symbols;
	std::string              prevProvider;
	boost::posix_time::ptime prevTime;
	float                    prevTemperature;
	bool                     first;
	
	void 	doAddSymbol( int symNumber, float probability, const boost::posix_time::ptime &time, 
			             const std::string &provider, float latitude, int min );

	int correctSymbol( float temperature, int symNumber );

public:
	SymbolContext();
	SymbolContext( const SymbolContext &sc );

	void addSymbol( int symNumber, float probability, 
			          const boost::posix_time::ptime &time,
			          float temperatureAtTime,
			          const boost::posix_time::ptime &fromTime,
			          const std::string &provider, float latitude );
	
	void update( ProviderSymbolHolderList &symbolList );
	
};

}


#endif 
