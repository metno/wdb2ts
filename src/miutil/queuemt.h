/*
  Kvalobs - Free Quality Control Software for Meteorological Observations

  $Id: readfile.h,v 1.1.2.2 2007/09/27 09:02:28 paule Exp $

  Copyright (C) 2007 met.no

  Contact information:
  Norwegian Meteorological Institute
  Box 43 Blindern
  0313 OSLO
  NORWAY
  email: kvalobs-dev@met.no

  This file is part of KVALOBS

  KVALOBS is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License as
  published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  KVALOBS is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with KVALOBS; if not, write to the Free Software Foundation Inc.,
  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef __QUEUEMT_H__
#define __QUEUEMT_H__

#include <boost/version.hpp>

#if BOOST_VERSION < 103500
#include <boost/thread/xtime.hpp>
#endif

#include <boost/thread/thread.hpp>
#include <boost/thread/condition.hpp>
#include <boost/shared_ptr.hpp>
#include <queue>
#include <exception>


namespace miutil {

class QueueSuspended : std::exception{
public:
   explicit QueueSuspended(){}

   virtual ~QueueSuspended() throw(){};

   const char *what()const throw()
   {
      return "QueueSuspended";
   }
};




template<typename T>
class queuemt
{
protected:
   typedef boost::mutex::scoped_lock  Lock;

   mutable boost::mutex mutex;
   std::queue<T> dataQueue;
   boost::condition cond;
   bool suspended;
   unsigned int maxSize;

public:

   queuemt()
      : suspended( false ), maxSize( 0 )
   {}

   queuemt( unsigned int maxQueSize)
         : suspended( false ), maxSize( maxQueSize )
      {}

   queuemt( const queuemt &que )
   {
      Lock lock( que.mutex );
      suspended = que.suspended;
      maxSize = que.maxSize;
      dataQueue = que.dataQueue;
   }

   void
   push(T value)
   {
      Lock lock( mutex );

      if( suspended )
         throw QueueSuspended();

      while( maxSize>0 && dataQueue.size() >= maxSize ) {
         cond.wait( lock );
      }

      dataQueue.push( value );
      cond.notify_one();
   }

   bool
   timedPush( T value, unsigned int timeoutInMillisecond )
   {
      Lock lock( mutex );

      if( suspended )
         throw QueueSuspended();

      if( maxSize > 0 && dataQueue.size() >= maxSize ) {
#if BOOST_VERSION < 103500
         boost::xtime waitUntil;
         //Round up to the nearest second if timeoutInMillisecond is not a multiple of 1000.
         int sec = (timeoutInMillisecond/1000) + (((timeoutInMillisecond % 1000) == 0)?0:1);
         boost::xtime_get( &waitUntil, boost::TIME_UTC );
         waitUntil.sec += sec;

         if( ! cond.timed_wait( lock, waitUntil ) )
            return false;
#else
         if( ! cond.timed_wait( lock, boost::posix_time::milliseconds( timeoutInMillisecond ) ) )
            return false;
#endif
         if( dataQueue.size() >= maxSize )
            return false;
      }

      dataQueue.push( value );
      cond.notify_one();

      return true;
   }

   void
   waitAndPop(T& value)
   {
      Lock lock( mutex );

      while( dataQueue.empty() ) {
         if( suspended )
            throw QueueSuspended();

         cond.wait( lock );
      }

      value = dataQueue.front();
      dataQueue.pop();

      if( maxSize > 0  )
         cond.notify_one();
   }

   bool
   timedWaitAndPop( int timeoutInMillisecond, T &value )
   {
      Lock lock( mutex );

      if( dataQueue.empty() ) {
         if( suspended )
            throw QueueSuspended();

#if BOOST_VERSION < 103500
         boost::xtime waitUntil;
         //Round up to the nearest second if timeoutInMillisecond is not a multiple of 1000.
         int sec = (timeoutInMillisecond/1000) + (((timeoutInMillisecond % 1000) == 0)?0:1);
         boost::xtime_get( &waitUntil, boost::TIME_UTC );
         waitUntil.sec += sec;

         if( ! cond.timed_wait( lock, waitUntil ) )
            return false;
#else
         if( ! cond.timed_wait( lock, boost::posix_time::milliseconds( timeoutInMillisecond ) ) )
            return false;
#endif
         if( dataQueue.empty() )
            return false;
      }


      value = dataQueue.front();
      dataQueue.pop();

      if( maxSize > 0  )
         cond.notify_one();

      return true;
   }


   bool
   tryPop(T& value)
   {
      Lock lock( mutex );

      if( dataQueue.empty() ) {
         if( suspended )
            throw QueueSuspended();

         return false;
      }

      value = dataQueue.front();
      dataQueue.pop();

      if( maxSize > 0  )
         cond.notify_one();

      return true;
   }

   bool
   empty() const
   {
      Lock lock( mutex );
      return dataQueue.empty();
   }


   void
   suspend( bool clearQue = false)
   {
     Lock lock( mutex );

     if( suspended )
       return;

     suspended = true;

     if( clearQue )
        dataQueue.clear();

     cond.notify_all();
   }

   void
   resume()
   {
     Lock lock( mutex );

     if( !suspended )
       return;

     suspended = false;
     cond.notify_all();
   }

};

template<typename T>
class queuemtPtr
{
protected:
   typedef boost::mutex::scoped_lock  Lock;

   mutable boost::mutex mutex;
   std::queue< T* > dataQueue;
   boost::condition cond;
   bool suspended;
   unsigned int maxSize;

public:

   queuemtPtr()
      : suspended( false ), maxSize( 0 )
   {}

   queuemtPtr( unsigned int maxQueSize )
   : suspended( false ), maxSize( maxQueSize )
   {}

   queuemtPtr( const queuemtPtr &que )
   {
      Lock lock( que.mutex );
      suspended = que.suspended;
      dataQueue = que.dataQueue;
      maxSize = que.maxSize;
   }


   void
   push( T *value)
   {
      Lock lock( mutex );

      if( suspended )
         throw QueueSuspended();

      while( maxSize>0 && dataQueue.size() >= maxSize ) {
         cond.wait( lock );
      }

      dataQueue.push(  value );
      cond.notify_one();
   }

   bool
   timedPush( T *value, unsigned int timeoutInMillisecond )
   {
      Lock lock( mutex );

      if( suspended )
         throw QueueSuspended();

      if( maxSize > 0 && dataQueue.size() >= maxSize ) {
#if BOOST_VERSION < 103500
         boost::xtime waitUntil;
         //Round up to the nearest second if timeoutInMillisecond is not a multiple of 1000.
         int sec = (timeoutInMillisecond/1000) + (((timeoutInMillisecond % 1000) == 0)?0:1);
         boost::xtime_get( &waitUntil, boost::TIME_UTC );
         waitUntil.sec += sec;

         if( ! cond.timed_wait( lock, waitUntil ) )
            return false;
#else
         if( ! cond.timed_wait( lock, boost::posix_time::milliseconds( timeoutInMillisecond ) ) )
            return false;
#endif
         if( dataQueue.size() >= maxSize )
            return false;
      }

      dataQueue.push( value );
      cond.notify_one();

      return true;
   }



   T*
   waitAndPop()
   {
      Lock lock( mutex );

      while( dataQueue.empty() ) {
         if( suspended )
            throw QueueSuspended();

         cond.wait( lock );
      }

      T* res = dataQueue.front();
      dataQueue.pop();

      if( maxSize > 0  )
         cond.notify_one();

      return res;
   }

   T*
   timedWaitAndPop( int timeoutInMillisecond )
   {
      Lock lock( mutex );

      if( dataQueue.empty() ) {
         if( suspended )
            throw QueueSuspended();

#if BOOST_VERSION < 103500
         boost::xtime waitUntil;
         //Round up to the nearest second if timeoutInMillisecond is not a multiple of 1000.
         int sec = (timeoutInMillisecond/1000) + (((timeoutInMillisecond % 1000) == 0)?0:1);
         boost::xtime_get( &waitUntil, boost::TIME_UTC );
         waitUntil.sec += sec;

         if( ! cond.timed_wait( lock, waitUntil ) )
            return 0;
#else
         if( ! cond.timed_wait( lock, boost::posix_time::milliseconds( timeoutInMillisecond ) ) )
            return 0;
#endif
         if( dataQueue.empty() )
            return 0;
      }

      T *res = dataQueue.front();
      dataQueue.pop();

      if( maxSize > 0  )
         cond.notify_one();

      return res;
   }



   T*
   tryPop()
   {
      Lock lock( mutex );

      if( dataQueue.empty() ) {
         if( suspended )
            throw QueueSuspended();

         return 0;
         //return boost::shared_ptr<T>();
      }

      T* res = dataQueue.front();
      dataQueue.pop();

      if( maxSize > 0  )
         cond.notify_one();

      return res;
   }

   bool
   empty() const
   {
      Lock lock( mutex );
      return dataQueue.empty();
   }


   void
   suspend( bool clearQue=false, bool deleteElements=false )
   {
     Lock lock( mutex );

     if( suspended )
       return;

     suspended = true;

     if( clearQue ) {
        if( deleteElements ) {
           for( typename std::queue<T*>::iterator it=dataQueue.begin(); it != dataQueue.end(); ++it )
              delete *it;
        }

        dataQueue.clear();
     }

     cond.notify_all();
   }

   void
   resume()
   {
     Lock lock( mutex );

     if( !suspended )
       return;

     suspended = false;
     cond.notify_all();
   }

};
}

#endif
