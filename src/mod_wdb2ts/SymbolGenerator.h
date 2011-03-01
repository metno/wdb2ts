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
#include <puTools/miTime.h>
#include <puMet/miSymbol.h>
#include <puMet/symbolMaker.h>
#include <LocationData.h>
#include <SymbolHolder.h>
#include <SymbolConf.h>

namespace wdb2ts {

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
	
	/*
	bool 
	computeSymbols(std::map<miutil::miTime, std::map <std::string,float> >& data,
	               vector<miSymbol> &symbols,
	               float latitude, int min, int max, std::string &error);
	 */
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
	void correctSymbol( SymbolHolder::Symbol &symbol,  const LocationElem &data );

			                           
};

}


#endif
