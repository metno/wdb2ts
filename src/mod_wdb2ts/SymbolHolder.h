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

#ifndef __SYMBOLHOLDER_H__
#define __SYMBOLHOLDER_H__

#include <float.h>
#include <ostream>
#include <vector>
#include <map>
#include <list>
#include <puMet/miSymbol.h>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace wdb2ts {




class SymbolHolder
{
   SymbolHolder(const SymbolHolder &);
   SymbolHolder& operator=(const SymbolHolder &);

public:
   struct Symbol
   {
      int min, max;
      miSymbol  symbol;
      float     probability;

      Symbol() : min(-1), max( -1 ), probability( FLT_MAX ) {}
      Symbol( int min_, int max_, const miSymbol &symbol_, float probability_=FLT_MAX )
         : min( min_ ), max( max_ ), symbol ( symbol_ ), probability( probability_ ) {}

      Symbol( const Symbol &s )
         : min( s.min ), max( s.max ), symbol( s.symbol ), probability( s.probability ) {}

      Symbol& operator=( const Symbol &rhs ) {
                    if ( this != &rhs ) {
                        min    = rhs.min;
                        max    = rhs.max;
                        symbol = rhs.symbol;
                        probability = rhs.probability;
                    }

                    return *this;
               }

      //needed by std::merge  (algorithm)
      bool operator < ( const Symbol &rhs ) const {
                  return this->from() < rhs.from();
           }

      //needed by std::unique (algorithm)
      bool operator == ( const Symbol &rhs ) const {
                  return this->from() == rhs.from();
           }

      std::string idname()const;
      int timespanInHours() const { return max + min + 1; }
      miutil::miTime to()const { miTime t(symbol.getTime()); t.addHour( max ); return t; }
      miutil::miTime from()const { miTime t(symbol.getTime()); t.addHour( -1*min-1 ); return t; }

      void fromAndToTime( boost::posix_time::ptime &fromTime, boost::posix_time::ptime &toTime) const;

      boost::posix_time::ptime getTime() const;

   };


private:
   std::vector<Symbol> symbols_;
   int                 min_, max_; 
   int                 index_;
   
   SymbolHolder( int min, int max, const std::vector<Symbol> &s ) 
        : symbols_( s ), min_( min ), max_( max ), index_( 0 ) {}
         
   public:
      SymbolHolder(): min_( 0 ), max_( 0 ), index_(0) {}
      SymbolHolder( int min, int max );
      SymbolHolder(int min, int max, const std::vector<miSymbol> &symbols);
      ~SymbolHolder();
      
      void addSymbol( const boost::posix_time::ptime &time, int custNumber, float latitude, float proability=FLT_MAX );
      
      int size() const { return symbols_.size(); }
      
      int timespanInHours()const { return max_+min_+1; }
      
      bool findSymbol( const boost::posix_time::ptime &fromTime, SymbolHolder::Symbol &symbol ) const;

      void initIndex(){ index_=0;}
            
      /**
       * initialize the get index so we start at an periode starting
       * at from.
       * 
       * @param from Start at an index with fromtime equal to from.
       * @return true if such a period is found and false otherwise. 
       */
      bool initIndex(const boost::posix_time::ptime &from);
      
      bool next( int &symbolid, 
      			  std::string &name, 
      			  std::string &idname,
      			  boost::posix_time::ptime &time, 
      			  boost::posix_time::ptime &from, 
      			  boost::posix_time::ptime &to,
      			  float &probability );
      
      /**
       * Merge two list of symbols and remove duplicates.
       * A symbol is a duplicate if the timespan is equal and the  
       * \e from and \e to times is equal. Symbols in this list
       * will take precedence over symbols in the list it is merged 
       * with when there is a duplicate.
       * 
       * Requirements: The two list to merge must have an equal 
       * timespan. Strictly, we should require that the min and max 
       * also was equal before we merged the two list, but this requirement
       * would in some instance lead to duplicates. The lists must be sorted. 
       * 
       * A SymbolHolder is returned with the merged result.
       * 
       * @param symbol the list of symbols to merge with this list of symbols.
       * @return The merged list.
       */
      SymbolHolder *merge(const SymbolHolder &symbol)const;
      
      friend std::ostream& operator<<(std::ostream &o, SymbolHolder &sh ); 
};

std::ostream& 
operator<<(std::ostream &o, SymbolHolder &sh );

typedef std::list<boost::shared_ptr<SymbolHolder> > SymbolHolderList;

class  ProviderSymbolHolderList :
	public std::map<std::string, SymbolHolderList >
{
public:
		ProviderSymbolHolderList(){};

		bool findSymbol( const std::string &provider,
				         const boost::posix_time::ptime &fromtime,
				         int timespanInHours,
				         SymbolHolder::Symbol &symbol ) const;
};


}
#endif
