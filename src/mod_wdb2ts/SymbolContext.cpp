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


#include <SymbolContext.h>

namespace wdb2ts {


SymbolContext::	
SymbolContext(): first( true ), prevTemperature( FLT_MAX )
{
	
}

SymbolContext::
SymbolContext( const SymbolContext &sc )
{
}

void 
SymbolContext::
doAddSymbol( int symNumber, float probability, const boost::posix_time::ptime &time,
		       const std::string &provider, float latitude, int min )
{
	using namespace boost::posix_time;
	
	int timespan( min + 1 );
	
	ProviderSymbolHolderList::iterator it = symbols.find( provider );
	
//	cerr << "SymbolContext: provider: " << provider << " min: " << min << endl;
	
	if( it == symbols.end() ) { 
//		cerr << "SymbolContext: Add new SymbolHolder: provider: " << provider << " min: " << min << endl;
		symbols[provider].push_back( boost::shared_ptr<SymbolHolder>( new SymbolHolder( min, 0 ) ) );
	}
	
	SymbolHolderList::iterator lit = symbols[provider].begin();
	SymbolHolderList::iterator litEnd = symbols[provider].end();
	boost::shared_ptr<SymbolHolder> symHolder;
	
	for( ; lit != litEnd && (*lit)->timespanInHours() != timespan ; ++lit );
	
	if( lit == litEnd ) {
//		cerr << "SymbolContext: Add new SymbolHolder: provider: " << provider << " min: " << min << endl;
		symHolder.reset( new SymbolHolder( min, 0 ) );
		symbols[provider].push_back( symHolder );
	} else {
		symHolder = *lit;
	}
//	cerr << "SymbolContext: Add symbol: provider: " << provider << " timespan: " << symHolder->timespanInHours() << endl;
	
	symHolder->addSymbol( time, symNumber, latitude, probability );
	prevProvider = provider;
}

/* The symbols in the database is just basis symbols, ie they
 * have to be corrected after we have corrected the temperature
 * with hight.
 * 
 * The symbols is corrected after the following table. Where
 * T is the hight adjusted temperature.
 * 
 * SYMBOLS           |       Hight adjusted temperature
 * from db           | 0.5 >= T < 1.5  |    T < 0.5          
 * ------------------+-----------------+----------------  
 *  (5) LIGHTRAINSUN |  (7) SLEETSUN   |  (8) SNOWSUN
 *  (9) LIGHTRAIN    | (12) SLEET      | (13) SNOW
 * (10) RAIN         | (12) SLEET      | (13) SNOW
 */
int
SymbolContext::
correctSymbol( float temperature, int symNumber )
{	
	if( symNumber == INT_MAX ) 
		return symNumber;
	
	// LIGHTRAINSUN (5)  || LIGHTRAIN (9)  ||  RAIN (10)
	if( symNumber == 5 || symNumber == 9 || symNumber == 10 ) {
		if( temperature < 0.5 ) {
			if( symNumber == 5 ) //LIGHTRAINSUN
				symNumber = 8;  //SNOWSUN
			else
				symNumber = 13; //SNOW
		} else if( temperature >= 0.5 && temperature < 1.5 ) {
			if( symNumber == 5 ) //LIGHTRAINSUN
				symNumber = 7;   //SLEETSUN
			else
				symNumber = 12;  //SLEET
		}
	}
	
	return symNumber;
}



void 
SymbolContext::
addSymbol( int symNumber, float probability, const boost::posix_time::ptime &time, 
		     float temperatureAtTime,
			  const boost::posix_time::ptime &fromTime,
		     const std::string &provider, float latitude )
{
	using namespace boost::posix_time;
	time_duration diff;
	time_duration h;
	
	if( symNumber == INT_MAX )
		return;
	
	if( ! fromTime.is_special() )
		diff = time - fromTime;
	else
		diff = time_duration(0, 0, 0);
	 
	//cerr << "addSymbol: " << time << " " << fromTime << " " << prevTime << " "<< (first?"true":"false") << endl;
	if( first && prevTime.is_special() && diff.hours() == 0 ) {
		prevTime = time;
		prevTemperature = temperatureAtTime;
		prevProvider = provider;
		
		symTmp.symNumber = symNumber;
		symTmp.probability = probability;
		symTmp.latitude = latitude;
		symTmp.time = time;
		return;
	}
	
	if( diff.hours() == 0 ) 
		h = time - prevTime;
	else
		h = diff;

	if( fromTime != prevTime )
		cerr << "SymbolContext::addSymbol: WARNING: fromTime!=prevtime '" << fromTime << " != " << prevTime <<"'. [" << provider << "]" <<endl;
	
	prevTime = time;
	
	if( h.is_negative() )
		h.invert_sign();
	
	int min = h.hours();
	
	if( min > 0 )
		min--;
	
	
	//May have a symbol for the first iteration.
	if( first && ! symTmp.time.is_special() && prevProvider == provider ) { 
		//cerr << "addSymbol: tmp " << symTmp.time << " " <<  min << endl;
		symTmp.symNumber = correctSymbol( prevTemperature, symTmp.symNumber );
		
		doAddSymbol( symTmp.symNumber, symTmp.probability, symTmp.time, provider, symTmp.latitude, min );
		symTmp.time = ptime(); //Undef
	}
	
	first = false;
	symNumber = correctSymbol( prevTemperature, symNumber );
	
	doAddSymbol( symNumber, probability, time, provider, latitude, min );
	prevTemperature = temperatureAtTime;
}


void 
SymbolContext::
update( ProviderSymbolHolderList &symbolList )
{
	for( ProviderSymbolHolderList::iterator it = symbols.begin(); it != symbols.end(); ++it ) 
		symbolList[it->first] = it->second;
}

}
