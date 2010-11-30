/*
 * Mutex.h
 *
 *  Created on: Nov 15, 2010
 *      Author: borgem
 */

#ifndef __MUTEX_H__
#define __MUTEX_H__

#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>

namespace wdb2ts {
typedef boost::mutex              Mutex;
typedef boost::mutex::scoped_lock ScopedLock;

//typedef boost::recursive_mutex    Mutex;
//typedef boost::recursive_mutex::scoped_lock ScopedLock;
}

#endif /* MUTEX_H_ */
