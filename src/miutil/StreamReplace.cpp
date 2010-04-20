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

#include <iostream>
#include <stdexcept>
#include "StreamReplace.h"
#include <algorithm>
/*
 * 	std::istream *ist;
	std::string what;
	std::string with;
	int nToReplace;
	int nCount;
	bool eofile;
	char *iBuf;            ///ist inputbuffer
	char *gibuf;           ///get pointer
	char *ibufend;         ///inputbuffer end pointer
	char *gibufend;        ///inputbuffer, end of data pointer.
	std::string::size_type gWath;
	std::string::size_type gWith;
 *
 *
*/

using namespace std;

namespace miutil {

StreamReplaceSource::
StreamReplaceSource()
	: ist( 0 ), eofile( false ), iBufRefCount(0), iBuf( 0 ), gibuf( 0 ), ibufend( 0 )
{
}

StreamReplaceSource::
StreamReplaceSource( std::istream &inStream,
		             const std::string &what_, const std::string &with_,
			         int nToReplace_,
			         int bufSize )
	: ist( &inStream ), what( what_ ),
	  with( with_ ), nToReplace( nToReplace_ ), nCount( nToReplace_ ),
     eofile( false ), iBufRefCount( 0 ), iBuf( 0 ), gWhat( 0 ), gWith( 0 ), length( 0 )
{
	int ibufsize=std::max( bufSize, static_cast<int>(what.size()) );

	//cerr << "StreamReplaceSource: ibufsize: " << ibufsize << endl;

	try {
		iBuf = new char[ibufsize];
		iBufRefCount++;
		ibufend = iBuf + ibufsize;
		gibufend = iBuf;
		gibuf = iBuf;
	}
	catch( ... ) {
		iBuf = 0;
	}
}
StreamReplaceSource::
StreamReplaceSource( const StreamReplaceSource &sr )
   : ist( sr.ist ),
     what( sr.what ),
     with( sr.with ),
     nToReplace( sr.nToReplace ),
     nCount( sr.nCount ),
     eofile( sr.eofile ),
     iBufRefCount( sr.iBufRefCount ),
     iBuf( sr.iBuf ),
     gibuf( sr.gibuf ),
     ibufend( sr.ibufend ),
     gibufend( sr.gibufend ),
     gWhat( sr.gWhat ),
     gWith( sr.gWith ),
     length( sr.length )
{
   cerr << "StreamReplaceSource::CTOR:";

   if( iBuf ) {
      cerr << " refcount before: " << iBufRefCount;
      iBufRefCount++;
      cerr << " After: " << iBufRefCount;
   }

   cerr << endl;
}

StreamReplaceSource::
~StreamReplaceSource()
{
   cerr << "StreamReplaceSource::DTOR:";
	if( iBuf ) {
	   cerr << " refcount before: " << iBufRefCount;
	   iBufRefCount--;
	   cerr << " After: " << iBufRefCount;


	   if( iBufRefCount == 0 ) {
	      cerr << " Delete iBuf.";
	      delete[] iBuf;
	   }

	}
	cerr << endl;
}


std::streamsize
StreamReplaceSource::
fillBufFromStream()
{
	int N;

	if( eofile )
		return -1;

	if( gibuf != gibufend ) {
		N = gibufend - gibuf;
		//cerr << "StreamReplaceSource::fillBufFromStream: move and fill. N: " << N << endl;
		memmove( iBuf, gibuf, N );
		gibuf = iBuf;
		gibufend = iBuf + N;
		N = ibufend - gibufend;
	} else {
		N = ibufend - iBuf;
		//cerr << "StreamReplaceSource::fillBufFromStream: fill from start. N: " << N << endl;
		gibuf = iBuf;
		gibufend = iBuf;
	}

	//cerr << "StreamReplaceSource::fillBufFromStream: N: " << N << endl;

	if( ist->read( gibufend, N ) ) {
		N = ist->gcount();
	} else if( ist->eof() ) {
		N = ist->gcount();
		eofile = true;
	} else {
		throw std::logic_error( "EXCEPTION: StreamReplaceSource::fillBufFromStream: failed to read input stream" );
	}

	//cerr << "StreamReplaceSource::fillBufFromStream: N (return): " << N <<  " eof: "
	//	 << (eofile?"true":"false")  << endl;

	gibufend += N;
	return N;
}

std::streamsize
StreamReplaceSource::
getDirect( char_type *buf, std::streamsize N )
{
	int n=0;

//	cerr << "StreamReplaceSource::getDirect: N (in): " << N << endl;

	if( gibuf == gibufend ) {
		//If the buffer is empty we read directly from the stream.

		if( eofile )
			return -1;

		//cerr << "StreamReplaceSource::getDirect: direct: " << N << endl;
		if( ist->read( buf, N ) ) {
			n = ist->gcount();
		} else if( ist->eof() ) {
			n = ist->gcount();
			eofile = true;
		} else {
			throw std::logic_error( "EXCEPTION: StreamReplaceSource::fillBufFromStream: failed to read input stream" );
		}
	} else {
		//Empty the streambuffer before we start to read directly from
		//the stream.

		//cerr << "StreamReplaceSource::getDirect: empty: " << N << endl;
		while( n<N && gibuf != gibufend ) {
			buf[n] = *gibuf;
			n++;
			gibuf++;
		}

		//cerr << "StreamReplaceSource::getDirect: (while) n: " << n << endl;
	}

	//cerr << "StreamReplaceSource::getDirect: n (return): " << n << endl;
	return n;
}



/*
std::streamsize
StreamReplaceSource::
getDirect( char_type *buf, std::streamsize N )
{
	int n=0;
	int nret;

	cerr << "StreamReplaceSource::getDirect: N (in): " << N << endl;

	while( n < N  ) {
		if( gibuf == gibufend ) {
			nret = fillBufFromStream();

			if( nret < 0 ) {
				if( n == 0 )
					n = -1;
				break;
			}
		}

		while( n<N && gibuf != gibufend ) {
			buf[n] = *gibuf;
			n++;
			gibuf++;
		}

		cerr << "StreamReplaceSource::getDirect: (while) n: " << n << endl;
	}

	cerr << "StreamReplaceSource::getDirect: n (return): " << n << endl;
	return n;
}
*/
std::streamsize
StreamReplaceSource::
getWith( char_type *buf, std::streamsize N )
{
	int n=0;

	//cerr << "StreamReplaceSource::getWith: N " << N <<  " gWith: " << gWith << " le: " << with.length() << endl;

	if( gWith < with.length() ) {
		n = with.length()-gWith;
		n = std::min( n, static_cast<int>(N) );
		with.copy( buf, n, gWith );
		gWith += n;
	}

	if( gWith >= with.length() ) {
		//cerr << "StreamReplaceSource::getWith: end " << endl;
		gWhat = 0;
		gWith = 0;
		nCount--;
	}

	//cerr << "StreamReplaceSource::getWith: n (return): " << n << endl;
	return n;
}

std::streamsize
StreamReplaceSource::
get( char_type *buf, std::streamsize N )
{
	int i=0;
	int nret;
	char_type *tmp=0;

	if( nToReplace >0 && nCount == 0 )
		return getDirect( buf, N );

	if( gWhat == what.length() )
		return getWith( buf, N );

	while( i < N  ) {
		if( gibuf == gibufend ) {

			//If we are in the midle of a match but do not
			//have the hole match in the input buffer.
			//Reset the match pointer and set the getpointer
			//to the start of the match. This will move the
			//mathed characters to the front of the buffer
			//and fill the buffer from the inputstream. We have
			//allready, in the constructor, made room for a
			//complete 'what' string.  After the buffer is filled
			//we start the search over again.

			if( gWhat>0 ) {
				gWhat = 0;
				gibuf = tmp;
			}

			nret = fillBufFromStream();

			if( nret < 0 ) {
				if( i == 0 )
					i = -1;
				break;
			}
		}

		if( gibuf != gibufend ) {
			if( what[gWhat] == *gibuf ) {
				if( gWhat == 0 )
					tmp = gibuf;

				gWhat++;
				gibuf++;

				if( gWhat == what.length() ) {
					if( i > 0 )
						return i;
					else
						return getWith( buf, N );
				}
			} else if( gWhat > 0 ) {
				gWhat = 0;

				if( tmp != gibuf && i<N ) {
					buf[i] = *tmp;
					i++;
				} else {
					gibuf = tmp;
				}
			} else {
				buf[i] = *gibuf;
				gibuf++;
				i++;
			}
		}
	}

	return i;
}


std::streamsize
StreamReplaceSource::
read( char_type *buf, std::streamsize N)
{
	int nTot=0;
	int n=N;
	char_type *tmp=buf;
    bool myEof(false);

	if( !ist  )
		return -1;

	try {
		while( nTot<N ) {
			n = get( tmp, n );

		//	cerr << "StreamReplaceSource::read n: " << n << endl;

			if( n < 0 ) {
				myEof = true;
				break;
			}

			tmp += n;
			nTot += n;
			n = N - nTot;
		}
	}
	catch( const std::exception &ex ) {
		cerr << "StreamReplaceSource::read : " << ex.what() << endl;
		return -1;
	}

	//cerr << "StreamReplaceSource::read  nToT (return): " << nTot  << endl;
   //cerr << "StreamReplaceSource::read[" << endl
	//	 << string( buf, nTot ) << "]"<< endl;

	length += nTot;
	//cerr << "StreamReplaceSource::read  length: " << length  << endl;

	if( nTot == 0 && myEof )
		return -1;

	return nTot;
}

}


