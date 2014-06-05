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

#ifndef __SYMBOLGENERATOR_H__
#define __SYMBOLGENERATOR_H__

#include <map>
#include <vector>
#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <puTools/miTime.h>
#include <puMet/miSymbol.h>
#include <puMet/symbolMaker.h>
#include <LocationData.h>
#include <SymbolHolder.h>
#include <SymbolConf.h>
#include <weather_symbol/Factory.h>
#include "Precipitation.h"


namespace wdb2ts {

namespace symbol {
struct SymbolData
		: virtual public weather_symbol::WeatherData
{
	float thunderProbability;
	float fogCover;
	boost::posix_time::ptime from;

	SymbolData()
		: thunderProbability( FLT_MAX ), fogCover( FLT_MAX ){}

	bool valid() const {
		return precipitation != FLT_MAX && totalCloudCover != FLT_MAX && temperature != FLT_MAX;
	}
};


//struct SymbolData
//{
//	float precip;
//	float totalClouds;
//	float highClouds;
//	float mediumClouds;
//	float lowClouds;
//	float fog;
//	float thunderProbability;
//	float temperature;
//	boost::posix_time::ptime from;
//	SymbolData():
//		precip( FLT_MAX ), totalClouds( FLT_MAX ), highClouds( FLT_MAX ), mediumClouds( FLT_MAX ), lowClouds( FLT_MAX ),
//		fog( 0 ),	thunderProbability( 0 ), temperature( FLT_MAX ){}
//
//	bool valid() const {
//		return precip != FLT_MAX && totalClouds != FLT_MAX && temperature != FLT_MAX;
//	}
//};


std::ostream&
operator<<( std::ostream &o, const SymbolData &data);

typedef std::map<boost::posix_time::ptime, SymbolData> SymbolDataContainer;

void
loadBaseData( SymbolDataContainer &dataContainer,
		            LocationData& data,
			        const std::string &provider,
			        int dataTimeStep );

void
computeSymbolData( const SymbolDataContainer &baseData,
		           SymbolDataContainer &data,
		           int hours );

struct SymbolDataExtra {
	SymbolData symbolData;
	int count;
	SymbolDataExtra(): count( 0 ) {}
	SymbolDataExtra( const SymbolDataExtra &s )
		: symbolData( s.symbolData ),count( s.count ) {}
	SymbolDataExtra& operator=(const SymbolDataExtra &rhs ) {
		if( this != &rhs ) {
			count = rhs.count;
			symbolData = rhs.symbolData;
		}
		return *this;
	}

	SymbolDataExtra& operator=(const SymbolData &rhs ) {
		if( &this->symbolData != &rhs ) {
			++count;
			symbolData = rhs;
		}
		return *this;
	}
};

typedef std::map<boost::posix_time::ptime, SymbolDataExtra> SymbolContainerElement;
typedef std::map<boost::posix_time::ptime, SymbolContainerElement > SymbolContainer;

void
computeSymbolData( const SymbolDataContainer &baseData,
		           SymbolContainer &data,
		           int hours );


}

class SymbolGenerator 
{
	std::map<miutil::miString, int> IDlist;
	static symbolMaker sm;
	
public:
	SymbolGenerator();
	
	static miSymbol getSymbol(int custNumber ) { return sm.getSymbol( custNumber ) ;}
	static bool isShower(int custNumber) { return sm.isShower( custNumber );}
	static bool isPrecip(int custNumber) { return sm.isPrecip( custNumber ); }
	static bool isDry(int custNumber)    { return sm.isDry( custNumber ); }

	bool readConf( const std::string &confile );
	
	static
	SymbolHolder*
	computeSymbolsWithPuMet( LocationData& data,
	                         const SymbolConf &symbolConf,
	                         const std::string &provider,
	                         bool withoutStateOfAgregate,
	                         std::string &error );

	static
	SymbolHolder*
	getSymbolsFromData( LocationData& data,
	                    const SymbolConf &symbolConf,
	                    const std::string &provider,
	                    std::string &error );



	static
	void
	computeSymbolBaseData( LocationData& data,
			               const std::string &provider,
		                   int hors, int dataTimeStep, std::string &error );

	
	static
	SymbolHolder*
	computeSymbols( LocationData& data,
			          const std::string &provider,
		             int min, int max, int precipHours, bool withoutStateOfAgregate, std::string &error );

	static
	ProviderSymbolHolderList computeSymbols( LocationData& data, 
			                                 const SymbolConfProvider &symbolConf,
			                                 bool withoutStateOfAgregate,
			                                 std::string &error );
	static
	void correctSymbol( SymbolHolder::Symbol &symbol,
	                    const LocationElem &data,
	                    const Precipitation &precip=Precipitation() );

	static
   void correctSymbol( SymbolHolder::Symbol &symbol,
                       const PartialData &pd,
                       const Precipitation &precip=Precipitation());

			                           
};

}


#endif
