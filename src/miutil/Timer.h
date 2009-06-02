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
