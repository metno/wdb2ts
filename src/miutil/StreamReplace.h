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
#ifndef __STREAMREPLACE_H__
#define __STREAMREPLACE_H__


#include <iostream> 
#include <iosfwd>    //streamsize
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/categories.hpp>  // source_tag

namespace miutil {

class StreamReplaceSource
{  
public:
	typedef char        char_type;
	typedef boost::iostreams::source_tag  category;

private:

	std::istream *ist;
	std::string what;
	std::string with;
	int nToReplace;
	int nCount;
	bool eofile;
	char *iBuf;            ///ist inputbuffer
	char *gibuf;           ///get pointer
	char *ibufend;         ///inputbuffer end pointer
	char *gibufend;        ///inputbuffer, end of data pointer.
	std::string::size_type gWhat;
	std::string::size_type gWith;
	int length;

	std::streamsize	fillBufFromStream();
	std::streamsize getDirect( char_type *buf, std::streamsize N );
	std::streamsize	getWith( char_type *buf, std::streamsize N );

	std::streamsize get( char_type *buf, std::streamsize N );

public:

	StreamReplaceSource();
	StreamReplaceSource( std::istream &inStream,
			             const std::string &what, const std::string &with,
			             int nToReplace=0,
			             int bufSize=512 );

	std::streamsize read(char_type* s, std::streamsize n);
};

typedef boost::iostreams::stream<StreamReplaceSource> istreamreplace;

}


#endif
