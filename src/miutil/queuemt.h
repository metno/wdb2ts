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

public:

	queuemt()
		: suspended( false )
	{}

	queuemt( const queuemt &que )
	{
		Lock lock( que.mutex );
		suspended = que.suspended;
		dataQueue = que.dataQueue;
	}

	void
	push(T value)
	{
		Lock lock( mutex );

		if( suspended )
			throw QueueSuspended();

		dataQueue.push( value );
		cond.notify_one();
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
	}

	boost::shared_ptr<T>
	waitAndPop()
	{
		Lock lock( mutex );


		while( dataQueue.empty() ) {
			if( suspended )
				throw QueueSuspended();

			cond.wait( lock );
		}

		boost::shared_ptr<T> res( new T( dataQueue.front() ) );
		dataQueue.pop();
		return res;
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
		return true;
	}

	boost::shared_ptr<T>
	tryPop()
	{
		Lock lock( mutex );

		if( dataQueue.empty() ) {
			if( suspended )
				throw QueueSuspended();

			return boost::shared_ptr<T>();
		}

		boost::shared_ptr<T> res( new T( dataQueue.front() ) );
		dataQueue.pop();

		return res;
	}

	bool
	empty() const
	{
		Lock lock( mutex );
		return dataQueue.empty();
	}


	void
	suspend()
	{
	  Lock lock( mutex );

	  if( suspended )
	    return;

	  suspended = true;
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

public:

	queuemtPtr()
		: suspended( false )
	{}

	queuemtPtr( const queuemtPtr &que )
	{
		Lock lock( que.mutex );
		suspended = que.suspended;
		dataQueue = que.dataQueue;
	}

	void
	push( T *value)
	{
		Lock lock( mutex );

		if( suspended )
			throw QueueSuspended();

		dataQueue.push(  value );
		cond.notify_one();
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
		return res;
	}

	T*
	tryPop()
	{
		Lock lock( mutex );

		if( dataQueue.empty() ) {
			if( suspended )
				throw QueueSuspended();

			return boost::shared_ptr<T>();
		}

		T* res = dataQueue.front();
		dataQueue.pop();

		return res;
	}

	bool
	empty() const
	{
		Lock lock( mutex );
		return dataQueue.empty();
	}


	void
	suspend()
	{
	  Lock lock( mutex );

	  if( suspended )
	    return;

	  suspended = true;
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
