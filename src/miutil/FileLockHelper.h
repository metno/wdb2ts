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

#ifndef __FILELOCKHELPER_H__
#define __FILELOCKHELPER_H__

#include <stdexcept>
#include <string>
#include <boost/thread/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace miutil {

class FileLockHelper
{
	FileLockHelper( const FileLockHelper &);
	FileLockHelper& operator=(const FileLockHelper &);
	FileLockHelper();
	
	std::string filename_;
	boost::mutex  mutex;
	
public:
	FileLockHelper( const std::string &filename );
	~FileLockHelper();

	std::string filename() const;
		
	bool write( const std::string &buf, bool wait, bool &wasLocked );
	bool read( std::string &buf, bool wait, bool &wasLocked );
		
	/**
	 * Stat the file and return the modification time. 
	 * 
	 * If the file dont exist the returned time is set to
	 * is_special.
	 * 
	 * On other error a runtime_error exception is thrown.
	 * 
	 * @exception std::runtime_error If stat fails.
	 */ 
	boost::posix_time::ptime modifiedTime() const;
};


}


#endif 
