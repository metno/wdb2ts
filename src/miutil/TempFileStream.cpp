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
#include <unistd.h>
#include <TempFileStream.h>

namespace miutil {

using namespace std;

TempFileStream::
TempFileStream()
{
}

TempFileStream::
~TempFileStream()
{
	if( fs.is_open() )
		fs.close();

	if( ! filename_.empty() )
		unlink( filename_.c_str() );
}

std::fstream*
TempFileStream::
create(const std::string &prefix, const std::string &path )
{
	if( path.empty() )
		filename_="/tmp";
	else if( path[path.length()-1] != '/' )
		filename_ = path + "/";
	else
		filename_ = path;

	filename_ += prefix + "-XXXXXX";

	char buf[filename_.size()+1];

	strcpy( buf, filename_.c_str() );
	int fd = mkstemp( buf );

	if( fd < 0 ) {
		filename_.erase();
		throw logic_error("Cant create temporary file: " + filename_ );
	}

	filename_ = buf;

	//I can't find any standard way to use the filedescriptor to open a stream. The
	//work around is to close the file and reopen it with a fstream. This is a bit
	//dangerous as it is possible for another process to open the file and use it.

	close( fd );
	fs.open( filename_.c_str(), ios::out | ios::in | ios::trunc );

	if( ! fs ) {
		unlink( filename_.c_str() );
		filename_.erase();
		throw logic_error("Cant reopen temporary file as an fstream: " + filename_ );
	}

	return &fs;
}


}


