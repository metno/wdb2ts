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
#ifndef __RWMutex_h__
#define __RWMutex_h__

#include <iostream>
#include <boost/thread/thread.hpp>
#include <boost/thread/condition.hpp>
#include <string>

/**
 * \addtogroup threadutil
 * @{
 */


namespace miutil {
  namespace thread{
 
   
    /**
     * A reader/writer lock.
     * 
     * Allow multiple readers but only one writer
     * at a time.
     * 
     * This implementation gives priority to writers.
     * It is so eager to serve writers that readers my
     * starve to death. 
     */
    class RWMutex: private boost::noncopyable 
    {
      protected:
        typedef boost::mutex::scoped_lock  Lock;
	
        boost::mutex     mutex;
        boost::condition cond;
        int              readers_;  //Number of active readers
        int              waitingWriters_;  //Number of waiting writers.
        bool             writerIsActiv_; //An writer is activ. 

      public:
        explicit  RWMutex();
        ~RWMutex();
	
        bool readLock(int timeoutInSeconds=0);
        bool tryReadLock();
        void readUnlock();
      
        bool writeLock(int timeoutInSeconds=0);
        bool tryWriteLock();
        void writeUnlock();
        
        int waitingWriters();
        int readers();
        bool writerIsActiv();
    };
    
    class RWReadLock
    {
      RWMutex &mutex;
      
      public:
        RWReadLock(RWMutex &rwmutex)
          :mutex(rwmutex)
          {  mutex.readLock(); }
          
        ~RWReadLock() 
          {   mutex.readUnlock();}
    };
    
    
    class RWWriteLock
    {
      RWMutex &mutex;
      
      public:
        RWWriteLock(RWMutex &rwmutex)
          :mutex(rwmutex)
          {  mutex.writeLock(); }
          
        ~RWWriteLock() 
          {   mutex.writeUnlock();}
    };
  }
}
/** @} */
#endif
