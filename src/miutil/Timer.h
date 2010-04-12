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

#ifndef __TIMER_H__
#define __TIMER_H__

namespace miutil{
	
	class Timer{
		
		typedef struct { double start;
		                 double sum;
							} SumMark;
		
		double *timers;
		char   **id;
		int    nTimers;
		int    nextMark;
		
		SumMark *sumMark;
		char    **idSumMark;
		int     nSumMark;
		int     nextSumMark;
		
		int findSumMark( const char *id_, bool mustExist );
				
		public:
			Timer(int nTimers_=1, int nSumMarkers = 10);
			~Timer();
			
			void reset();
		
			/**
			 * Set a timestamp with at the next 
			 * mark slot. 
			 * 
			 * If an id is given, set the name of the 
			 * mark to id.
			 */
			void mark(const char *id=0);
			

			/**
			 * Set a timestamp with at the next 
			 * mark slot. 
			 * 
			 * If an id is given, set the name of the 
			 * mark to id.
			 */
			void mark(const std::string & id) {
				if( id.empty() )
					mark();
				else
					mark( id.c_str() );
			}
			
			void markStart( const char *id_ );
			void markStop( const char *id_ );

			
			/**
			 * Return the index of the mark that was last
			 * set.
			 */
			int getLastmark()const;
			
			/**
			 * return the number of slot in the mark table.
			 */
			int size()const { return nTimers;};
			
			/**
			 * Get the value to mark i.
			 */
			double getMark(int i)const;
			
			
			/**
			 * Return the difference between the first mark
			 * and the mark with index i. If i is -1 the difference
			 * between the first and last mark is returned.
			 */
			double getDiff(int i=-1)const;
			
			/**
			 * Return the difference between the first mark with id
			 * and the last mark with id. 
			 */
			double getTimer(const char *id_)const;
			
			
			
			/**
			 * Return the difference between the mark at
			 * index i and the mark at i-1.
			 */
			double getTimer(int i)const;
			
			/**
			 * print pretty summary.
			 */
			std::ostream& print( std::ostream &o ) const;
			
			friend std::ostream& operator<<(std::ostream& output,
                                            const miutil::Timer& t);
	};

std::ostream& operator<<(std::ostream &output,
                         const miutil::Timer &t);
					
};



#endif /*TIMERS_H_*/
