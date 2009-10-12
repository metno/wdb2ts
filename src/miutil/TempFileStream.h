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
#ifndef __TEMPFILESTREAM_H__
#define __TEMPFILESTREAM_H__


#include <fstream>
#include <stdexcept>

namespace miutil {

/**
 * TempFileStream creates a temporary filestream and
 * remove the file when the destructor is called, ie the
 * instance goes out of scope.
 */

class TempFileStream
{  
	std::string filename_;
	std::fstream fs;

public:
	TempFileStream();
	~TempFileStream();

	/**
	 * create a file in the directory given with path. Use prefix
	 * as the start of the filename.
	 *
	 * @param prefix Use this as the start of the filename.
	 * @param path Directory to create the temporary file. If path
	 *  is empty the file is created in the "/tmp" directory.
	 * @return a open file stream. The stream is closed and the
	 *  file is removed when the instace goes out of scope.
	 * @throws std::logic_error if the file cant be created.
	 */
	std::fstream& create( const std::string &prefix, const std::string &path="" );
	std::string filename()const { return filename_; }
};

}


#endif
