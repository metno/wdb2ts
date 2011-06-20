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
#include <utility>
#include <map>
#include <set>
#include <list>
//#include <puMet/miSymbol.h>
#include <puMet/symbolMaker.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "LocationElem.h"

namespace wdb2ts {




class SymbolHolder
{
   SymbolHolder(const SymbolHolder &);
   SymbolHolder& operator=(const SymbolHolder &);

public:
   struct Symbol
   {
      int min, max;
      miSymbol symbol;
      float probability;
      bool withOutStateOfAgregate;

      Symbol() : min(-1), max( -1 ), probability( FLT_MAX ), withOutStateOfAgregate( true ) {}
      Symbol( int min_, int max_, const miSymbol &symbol_, float probability_=FLT_MAX, bool withOutStateOfAgregate_=true )
         : min( min_ ), max( max_ ), symbol ( symbol_ ), probability( probability_ ),
           withOutStateOfAgregate( withOutStateOfAgregate_ ) {}

      Symbol( const Symbol &s )
         : min( s.min ), max( s.max ), symbol( s.symbol ), probability( s.probability ),
           withOutStateOfAgregate( s.withOutStateOfAgregate ){}

      Symbol& operator=( const Symbol &rhs ) {
                    if ( this != &rhs ) {
                        min    = rhs.min;
                        max    = rhs.max;
                        symbol = rhs.symbol;
                        probability = rhs.probability;
                        withOutStateOfAgregate = rhs.withOutStateOfAgregate;
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

      bool hasThunder() const { return symbolMaker::hasThunder( symbol ); }
      void turnOffThunder() { symbolMaker::turnOffThunder( symbol ); }
      std::string idname()const;
      int         idnumber()const { return const_cast<Symbol*>(this)->symbol.customNumber(); }
      int timespanInHours() const { return max + min + 1; }
      boost::posix_time::ptime toAsPtime()const;
      boost::posix_time::ptime fromAsPtime()const;
      miutil::miTime to()const { miutil::miTime t(symbol.getTime()); t.addHour( max ); return t; }
      miutil::miTime from()const { miutil::miTime t(symbol.getTime()); t.addHour( -1*min-1 ); return t; }

      void fromAndToTime( boost::posix_time::ptime &fromTime, boost::posix_time::ptime &toTime) const;

      boost::posix_time::ptime getTime() const;

   };

   typedef std::vector<Symbol> SymbolList;
   typedef std::pair<SymbolList::const_iterator,SymbolList::const_iterator> SymbolRange;

private:
   SymbolList symbols_;
   int         min_, max_;
   int         index_;
   
   SymbolHolder( int min, int max, const std::vector<Symbol> &s ) 
        : symbols_( s ), min_( min ), max_( max ), index_( 0 ) {}
         
   public:

      SymbolHolder(): min_( 0 ), max_( 0 ), index_(0) {}
      SymbolHolder( int timespan );

      ///This constructor reserve room for 100 symbols with range min-max.
      SymbolHolder( int min, int max );
      SymbolHolder(int min, int max, const std::vector<miSymbol> &symbols, bool withOutStateOfAgregate=true);
      ~SymbolHolder();
      
      void addSymbol( const boost::posix_time::ptime &time, int custNumber, float latitude, bool withOutStateOfAgregate, float proability=FLT_MAX);
      
      int size() const { return symbols_.size(); }
      
      int timespanInHours()const { return max_+min_+1; }
      
      bool findSymbol( const boost::posix_time::ptime &fromTime, SymbolHolder::Symbol &symbol ) const;

      SymbolRange findSymbolsInRange(const boost::posix_time::ptime &fromTime,
                                     const boost::posix_time::ptime &toTime,
                                     bool exact = true );

      void initIndex(){ index_=0;}
            
      /**
       * initialize the get index so we start at an periode starting
       * at from.
       * 
       * @param from Start at an index with fromtime equal to from.
       * @return true if such a period is found and false otherwise. 
       */
      bool initIndex(const boost::posix_time::ptime &from);
      bool next( SymbolHolder::Symbol &symbol );

      bool next( int &symbolid, 
      			  std::string &name, 
      			  std::string &idname,
      			  boost::posix_time::ptime &time, 
      			  boost::posix_time::ptime &from, 
      			  boost::posix_time::ptime &to,
      			  float &probability );
      
      /**
       * consistentCheckThuder, consistent check the symbols in this container against
       * the symbols in the \em otherSymbols.
       *
       * Turn of thunder in symbols in this container if the symbols in the otherSymbols
       * do not have thunder in the same range as this symbols covers.
       *
       * The timespan for the symbols in the \em otherSymbols container must be less than
       * the timespan in this container and (this->timespan % otherSymbols.timespan == 0).
       */
      void consistentCheckThunder( SymbolHolder &otherSymbols );

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
	   void setSymbolProbability( SymbolHolder::Symbol &symbol ) const;
	   std::map<boost::posix_time::ptime, PartialData> partialData;

public:
		ProviderSymbolHolderList(){};

		bool findSymbol( const std::string &provider,
				         const boost::posix_time::ptime &fromtime,
				         int timespanInHours,
				         SymbolHolder::Symbol &symbol ) const;

		bool findSymbol( const std::string &provider,
		                 const boost::posix_time::ptime &fromtime,
		                 const boost::posix_time::ptime &totime,
		                 SymbolHolder::Symbol &symbol ) const;


		void findAllFromtimes( const boost::posix_time::ptime &toTime,
				               const std::string &provider,
				               std::set<boost::posix_time::ptime> &fromtimes )const;

		void addPartialData( const LocationElem &elem ) ;
		bool getPartialData( const boost::posix_time::ptime &time, PartialData &pd )const;

		/**
		 * consistentCheck check the consistent between symbols with different time span for
		 * each provider.
		 *
		 * The following consistent checks is implemented.
		 *  - Thunder - Symbols with thunder of longer time span also have symbols
		 *              with thunder in symbols of shorter time span. If not remove
		 *              the thunder from the symbols of the longer time span.
		 */
		void consistentCheck();
};


}
#endif
