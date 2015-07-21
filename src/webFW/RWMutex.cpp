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
#include <stdlib.h>
#include <boost/thread.hpp>
#include <RWMutex.h>

miutil::thread::
RWMutex::
RWMutex()
  :readers_(0), waitingWriters_(0), writerIsActiv_(false)  
{
}

miutil::thread::
RWMutex::
~RWMutex()
{
}

bool 
miutil::thread::
RWMutex::
readLock(int timeout)
{
  Lock lock(mutex);

  if(waitingWriters_>0 || writerIsActiv_){
    if(timeout==0){
      while(waitingWriters_>0 || writerIsActiv_){
        cond.wait(lock);
      }
    }else{
      cond.timed_wait(lock, boost::posix_time::seconds(timeout));
    
      if(waitingWriters_>0 || writerIsActiv_)
        return false;
    }
  }
  
  readers_++; 
  return true;
}

bool
miutil::thread::
RWMutex::
tryReadLock()
{
  Lock lock(mutex);

  if(waitingWriters_>0 || writerIsActiv_)
    return false;
    
  readers_++;  
  return true;  
}


void
miutil::thread::
RWMutex::
readUnlock()
{
  Lock lock(mutex);

  if(readers_>0) //Safety guard. 
    readers_--;
  
  if(readers_==0)
    cond.notify_all(); //Wakeup all that is waiting on the condition variabel.
}

bool 
miutil::thread::
RWMutex::
writeLock(int timeout)
{
  Lock lock(mutex);

  waitingWriters_++;
  
  if(readers_>0 || writerIsActiv_){
    if(timeout==0){
      while(readers_>0 || writerIsActiv_){
        cond.wait(lock);
      }
    }else{
      cond.timed_wait(lock, boost::posix_time::seconds(timeout));
    
      if(readers_>0 || writerIsActiv_)
        return false;
    }
  }
  
  writerIsActiv_=true;
  waitingWriters_--;
  
  return true;
}

bool
miutil::thread::
RWMutex::
tryWriteLock()
{
  Lock lock(mutex);

  if(readers_>0 || writerIsActiv_)
    return false;
  
  writerIsActiv_=true;
  return true;  
}

void
miutil::thread::
RWMutex::
writeUnlock()
{
  Lock lock(mutex);

  writerIsActiv_=false;
  cond.notify_all(); //Wakeup all that is waiting on the condition variabel.
}

int 
miutil::thread::
RWMutex::
waitingWriters()
{
  Lock lock(mutex);
  return waitingWriters_;
}

int
miutil::thread::
RWMutex:: 
readers()
{
  Lock lock(mutex);
  
  return readers_;
}

    
bool
miutil::thread::
RWMutex:: 
writerIsActiv()
{
  Lock lock(mutex);
  
  return writerIsActiv_;
}
