/*
 * fileutil.h
 *
 *  Created on: Jun 20, 2013
 *      Author: borgem
 */

#ifndef __FILEUTIL_H__
#define __FILEUTIL_H__

#include <string>
#include <boost/date_time/posix_time/ptime.hpp>

namespace miutil {

bool
renamefile( const std::string &from, const std::string &to );

bool
removefile( const std::string &path );

bool
setmtime( const std::string &file,
          const boost::posix_time::ptime &newModificationTime );

bool
setatime( const std::string &file,
          const boost::posix_time::ptime &newAccessTime );

}



#endif
