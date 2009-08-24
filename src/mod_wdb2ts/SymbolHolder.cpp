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


#include <iostream>
#include <algorithm>
#include "SymbolHolder.h"
#include <SymbolGenerator.h>
#include <Logger4cpp.h>

using namespace std;

namespace {

std::string 
symbolidToName(int id);

}

namespace wdb2ts {

SymbolHolder::
SymbolHolder(int min, int max, const std::vector<miSymbol> &symbols)
:  min_( min ), max_( max ), index_(0)
{
	symbols_.reserve( symbols.size() );
   for ( std::vector<miSymbol>::size_type i=0; i<symbols.size(); ++i )
      symbols_.push_back( Symbol( min, max, symbols[i] ) );
}

SymbolHolder::
SymbolHolder( int min, int max )
: min_( min ), max_( max ), index_(-1)
{
	symbols_.reserve( 100 );
	
}

SymbolHolder::      
~SymbolHolder()
{
}


void 
SymbolHolder::
addSymbol( const boost::posix_time::ptime &time, int custNumber, float latitude, float proability )
{
	using namespace boost::posix_time;
	
	WEBFW_USE_LOGGER( "handler" );

	boost::gregorian::date datePart( time.date() );
	time_duration timePart( time.time_of_day() );
	miutil::miTime miT( datePart.year(), datePart.month(), datePart.day(),
			              timePart.hours(), timePart.minutes(), timePart.seconds() );
	
	miSymbol sym = SymbolGenerator::getSymbol( custNumber );
	
	if( symbolMaker::getErrorSymbol() == sym ) {
		WEBFW_LOG_ERROR( "SymbolHolder::addSymbol: " << custNumber << " not a valid miSymbol." );
		return;
	}
	
	sym.setTime( miT );
	sym.setLightStat( miT, latitude );
	
	if( symbols_.empty() ) {
		symbols_.push_back( SymbolHolder::Symbol( min_, max_, sym, proability ) ); 
		return;
	}
	
	miSymbol tmpSymbol = symbols_[symbols_.size() -1].symbol;
	
	//Should we add at the back
	if( tmpSymbol.getTime() < miT ) {
		symbols_.push_back( SymbolHolder::Symbol( min_, max_, sym, proability ) ); 
		return;
	}
		
	std::vector<Symbol>::iterator it = symbols_.begin();
	for( ; it != symbols_.end() && it->symbol.getTime()<miT; ++it  );
	
	if( it != symbols_.end() )
		symbols_.insert( it, SymbolHolder::Symbol( min_, max_, sym, proability ) );
	else
		symbols_.push_back( SymbolHolder::Symbol( min_, max_, sym, proability ) );
}


bool 
SymbolHolder::
initIndex( const boost::posix_time::ptime &fromtime_ )
{
	using namespace boost::posix_time;
	ptime fromtime( fromtime_ );
   ptime time;
   ptime from;
   ptime to;
   string name;
   string idname;
   float prob;
   int symbolid;
   
   
   if( symbols_.empty() ) 
   	return false;
   
   if( fromtime.is_special() ) {
   	miutil::miTime t = symbols_[0].from();
   	fromtime = ptime( boost::gregorian::date( t.year(), t.month(), t.day() ),
                        time_duration( t.hour(), t.min(), t.sec() ) );
   }
        
   index_=0;
   
   while ( next( symbolid, name, idname, time, from, to, prob ) ) {
   	//WEBFW_LOG_DEBUG( "initIndex: ft: " << fromtime << " from: " << from );
      if ( from < fromtime ) 
      	continue;

      //Must adjust the index_ one element back because next 
      //advance the index to the next index.
         
      index_--;
      return true;
   }
   
   return false;
}
   

bool 
SymbolHolder::
next( int &symbolid, 
	   std::string &name,
	   std::string &idname,
	   boost::posix_time::ptime &time, 
	   boost::posix_time::ptime &from, 
	   boost::posix_time::ptime &to,
	   float &probability )
{
	using namespace boost::posix_time;
	
   if (index < 0 || static_cast<std::vector<Symbol>::size_type>( index_) >= symbols_.size())
      return false;
   
   miTime t = symbols_[index_].symbol.getTime(); 
   time = ptime( boost::gregorian::date( t.year(), t.month(), t.day() ),
                 time_duration( t.hour(), t.min(), t.sec() ) );
   
   t = symbols_[index_].from();
   from = ptime( boost::gregorian::date( t.year(), t.month(), t.day() ),
                 time_duration( t.hour(), t.min(), t.sec() ) );
   
   t = symbols_[index_].to();
   to = ptime( boost::gregorian::date( t.year(), t.month(), t.day() ),
               time_duration( t.hour(), t.min(), t.sec() ) );
   
   symbolid = symbols_[index_].symbol.customNumber();
   name     = symbols_[index_].symbol.customName();
   idname   = symbolidToName( symbolid );
   probability = symbols_[index_].probability;
   index_++;
   
   return true;
}
      

   


SymbolHolder* 
SymbolHolder::
merge( const SymbolHolder &symbol)const
{
   if ( this->timespanInHours() != symbol.timespanInHours() )
        return 0;
        
   vector<Symbol>           retSymbols( this->symbols_.size() + symbol.symbols_.size() );
   vector<Symbol>::iterator it;
   
   it = std::merge(this->symbols_.begin(), this->symbols_.end(),
                   symbol.symbols_.begin(), symbol.symbols_.end(),
                   retSymbols.begin() );
   
   retSymbols.erase( it, retSymbols.end() );
   
   it=std::unique(retSymbols.begin(), retSymbols.end() );
   retSymbols.erase( it, retSymbols.end() );
   
   try{
      return new SymbolHolder( min_, max_, retSymbols );
   }
   catch ( ... ) {
      return 0;
   }  
}


std::ostream& 
operator<<(std::ostream &o, SymbolHolder &sh )
{
	using namespace boost::posix_time;
   ptime time, from, to;
   int sid;
   float prob;
   string name;
   string idname;
   ostringstream tost;
 
   o << "Symbols:  timespan: " << sh.timespanInHours() << endl;
  
   sh.initIndex();
      
   while ( sh.next( sid, name, idname, time, from, to, prob ) ) {
   	tost.str("");
   	
   	if( prob == FLT_MAX )
   		tost << " Prob: N/A";
   	else
   		tost << " Prob: " << prob;
   	
      o << from << " - " << to << " (" << time << ") " << sid << "    " << name << tost.str() << endl;
   }
   
   return o;    
}


}


namespace {

std::string 
symbolidToName(int id) {
   if ( id<0 ) 
      return "";
      
   switch( id ){
   case  1:
   case 16: return "SUN";
   case  2:
   case 17: return "LIGHTCLOUD"; 
   case  3: return "PARTLYCLOUD"; 
   case  4: return "CLOUD";
   case  5: return "LIGHTRAINSUN"; 
   case 18: return "LIGHTRAINSUN"; 
   case  6: return "LIGHTRAINTHUNDERSUN";
   case  7: return "SLEETSUN";
   case  8: return "SNOWSUN"; 
   case  9: return "LIGHTRAIN";
   case 10: return "RAIN"; 
   case 11: return "RAINTHUNDER";
   case 12: return "SLEET";
   case 13: return "SNOW"; 
   case 14: return "SNOWTHUNDER";
   case 15: return "FOG";
   case 19: return "SNOWSUN"; 
   default:
      return "";         //Unknown symbol
   }
}

}
