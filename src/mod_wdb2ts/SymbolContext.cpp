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
#include <Logger4cpp.h>

namespace wdb2ts {


SymbolContext::	
SymbolContext( bool useTempInFromtime_ )
	: symbols( 0 ), symbolOwner( true ), prevTemperature( FLT_MAX ), first( true ),
	  useTempInFromtime( useTempInFromtime_ )
{
	
}


SymbolContext::
~SymbolContext()
{
	if( symbolOwner && symbols )
		delete symbols;
}

void
SymbolContext::
setSymbols( ProviderSymbolHolderList *symbols_ )
{
	if( symbols && symbolOwner )
		delete symbols;

	symbols = symbols_;
	symbolOwner = false;
}


void 
SymbolContext::
doAddSymbol( int symNumber, float probability, const boost::posix_time::ptime &time,
		       const std::string &provider, float latitude, int min, bool withOutStateOfAgregate )
{
	using namespace boost::posix_time;
	
	int timespan( min + 1 );
	
	if( ! symbols ) {
		try {
			symbols = new ProviderSymbolHolderList();
		}
		catch( ... ) {
			symbols = 0;
			return;
		}
		symbolOwner = true;
	}

	ProviderSymbolHolderList::iterator it = symbols->find( provider );
	
//	WEBFW_LOG_DEBUG( "SymbolContext: provider: " << provider << " min: " << min );
	
	if( it == symbols->end() ) {
//		WEBFW_LOG_DEBUG( "SymbolContext: Add new SymbolHolder: provider: " << provider << " min: " << min );
		(*symbols)[provider].push_back( boost::shared_ptr<SymbolHolder>( new SymbolHolder( min, 0 ) ) );
	}
	
	SymbolHolderList::iterator lit = (*symbols)[provider].begin();
	SymbolHolderList::iterator litEnd = (*symbols)[provider].end();
	boost::shared_ptr<SymbolHolder> symHolder;
	
	for( ; lit != litEnd && (*lit)->timespanInHours() != timespan ; ++lit );
	
	if( lit == litEnd ) {
//		WEBFW_LOG_DEBUG( "SymbolContext: Add new SymbolHolder: provider: " << provider << " min: " << min );
		symHolder.reset( new SymbolHolder( min, 0 ) );
		(*symbols)[provider].push_back( symHolder );
	} else {
		symHolder = *lit;
	}
//	WEBFW_LOG_DEBUG( "SymbolContext: Add symbol: provider: " << provider << " timespan: " << symHolder->timespanInHours() );
	
	symHolder->addSymbol( time, symNumber, latitude, withOutStateOfAgregate, probability );
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
		   const std::string &provider, float latitude)
{
	using namespace boost::posix_time;
	time_duration diff;
	time_duration h;
	bool withOutStateOfAgregate=true;

	WEBFW_USE_LOGGER( "handler" );
	
	if( symNumber == INT_MAX )
		return;
	
	if( ! fromTime.is_special() )
		diff = time - fromTime;
	else
		diff = time_duration(0, 0, 0);
	 
	//WEBFW_LOG_DEBUG( "addSymbol: " << time << " " << fromTime << " " << prevTime << " "<< (first?"true":"false") );
	if( first && prevTime.is_special() && diff.hours() == 0 ) {
		WEBFW_LOG_DEBUG( "SymbolContext: addSymbol: tmp: " << time << " " << fromTime << " " << prevTime  );
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
	
	prevTime = time;
	
	if( h.is_negative() )
		h.invert_sign();
	
	int min = h.hours();
	
	if( min > 0 )
		min--;
	
	
	//May have a symbol for the first iteration.
	if( first && ! symTmp.time.is_special() && prevProvider == provider ) { 
		WEBFW_LOG_DEBUG( "addSymbol: tmp " << symTmp.time << " " <<  min );

		if( useTempInFromtime ) {
			withOutStateOfAgregate=false;
			symTmp.symNumber = correctSymbol( prevTemperature, symTmp.symNumber );
		}
		
		doAddSymbol( symTmp.symNumber, symTmp.probability, symTmp.time, provider, symTmp.latitude, min, withOutStateOfAgregate );
		symTmp.time = ptime(); //Undef
	}
	
	first = false;

	if( useTempInFromtime ) {
		withOutStateOfAgregate=false;
		symNumber = correctSymbol( prevTemperature, symNumber );
	}
	
	doAddSymbol( symNumber, probability, time, provider, latitude, min, withOutStateOfAgregate );
	prevTemperature = temperatureAtTime;
}


void 
SymbolContext::
update( ProviderSymbolHolderList &symbolList )
{
	if( ! symbols )
		return;

	if( symbols == &symbolList )
		return;

	for( ProviderSymbolHolderList::iterator it = symbols->begin(); it != symbols->end(); ++it )
		symbolList[it->first] = it->second;
}

}
