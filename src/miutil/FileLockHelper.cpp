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

#include <boost/version.hpp>
#include "ptimeutil.h"
#include "fileutil.h"

//#define BOOST_VERSION 103100

#if BOOST_VERSION >= 104400
#  define USE_BOOST 1
#  define OLD_BOOST_FILESYSTEM_VERSION BOOST_FILESYSTEM_VERSION
#  if BOOST_VERSION < 104800
#    define BOOST_FILESYSTEM_VERSION 3
#  endif
#  include <boost/system/error_code.hpp>
#  include <boost/filesystem.hpp>
#  include <boost/interprocess/sync/file_lock.hpp>
namespace fs = boost::filesystem;
namespace s = boost::system;
namespace ip = boost::interprocess;
#else
#  undef USE_BOOST
#  include <sys/types.h>
#  include <sys/stat.h>
#  include <unistd.h>
#  include <errno.h>
#  include <fcntl.h>
#endif


#include <FileLockHelper.h>
#include <iostream>
#include <sstream>
#include <fstream>

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 600
#define WDB2TS_DO_UNDEF_XOPEN_SOURCE 1
#endif

#include <string.h>

#ifdef WDB2TS_DO_UNDEF_XOPEN_SOURCE 
#undef _XOPEN_SOURCE
#endif


namespace miutil {


using namespace std;
	
FileLockHelper::
FileLockHelper( const std::string &filename )
	: filename_( filename )
{
}

FileLockHelper::
~FileLockHelper()
{
}

std::string 
FileLockHelper::
filename() const
{
	return filename_;
}

boost::posix_time::ptime
FileLockHelper::
modifiedTime() const
{
    if( filename_.empty() )
        throw runtime_error("No filename is given.");

    return miutil::file::getmtime( filename_ );
}


#ifndef USE_BOOST

bool 
FileLockHelper::
write( const std::string &buf_, bool wait, bool &wasLocked )
{
	boost::mutex::scoped_lock mylock( mutex );
	std::string buf( buf_ );
	
	wasLocked = false;
	
	if( filename_.empty() )
		return false;
	
	int fd = open( filename_.c_str(), 
			         O_WRONLY | O_CREAT, 
			         S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
	
	if( fd < 0 ) {
		cerr << "FileLockHelper::write: Cant open '" << filename_ << "' for writing." << endl;
		return false;
	}
	
	int ret;
	struct flock lock;
	
	lock.l_type = F_WRLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start = 0;
	lock.l_len = 0;
	
	if( wait ) {
	   do {
	      ret = fcntl( fd, F_SETLKW, &lock );
	   } while( ret<0 && errno == EINTR );
	}else {
		ret = fcntl( fd, F_SETLK, &lock );
		
		if( ret < 0 && ( errno == EACCES || errno == EAGAIN) )
		   wasLocked = true;
	}
	
	if( ret < 0 ) {
		cerr << "FileLockHelper::write: Cant lock '" << filename_ << "'. Already locked: "  
			  << (wasLocked?"true":"false") <<  endl;
	
		close( fd );
		return false;
	}
	
	if( ftruncate( fd, 0 ) < 0 )
		cerr << "FileLockHelper::write: '" << filename_ << "' truncate failed!" << endl;
	else if( lseek( fd, 0, SEEK_SET) < 0 )
		cerr << "FileLockHelper::write: '" << filename_ << "' lseek failed!" << endl;
	
	
	cerr << "FileLockHelper::write: buf[" << buf << "]" << endl;
		
	int n; 
	
	do{
	   do {
	      n = ::write( fd, buf.data(), buf.size() );
	   } while( n<0 && errno == EINTR );

		if( n < 0 )
			break;
		
		buf.erase( 0, n );
		
	}while( !buf.empty() ); 
		
	lock.l_type = F_UNLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start = 0;
	lock.l_len = 0;
	
	ret = fcntl( fd, F_SETLK, &lock );
	
	if( ret < 0 ) 
		cerr << "FileLockHelper::write: Cant unlock '" << filename_ << "'." <<  endl;

	
	//close remove the locks on the file.
	close( fd );
	
	if( ! buf.empty() ) {
		cerr << "FileLockHelper::write: '" << filename_ << "' write error. File deleted.!" << endl;
		unlink( filename_.c_str() );
		return false;
	}
	
	return true;
}

bool
FileLockHelper::
read( std::string &buf, bool wait, bool &wasLocked )
{
	boost::mutex::scoped_lock mylock( mutex );
	wasLocked = false;
		
	if( filename_.empty() )
		return false;
		
	int fd = open( filename_.c_str(), O_RDONLY );
		
	if( fd < 0 ) {
		cerr << "FileLockHelper::read: Cant open '" << filename_ << "' for reading." << endl;
		return false;
	}


	int ret;
	struct flock lock;
	
	lock.l_type = F_RDLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start = 0;
	lock.l_len = 0;
	
	if( wait ) {
	   do{
	      ret = fcntl( fd, F_SETLKW, &lock );
	   } while( ret<0 && errno == EINTR );
	}else {
		ret = fcntl( fd, F_SETLK, &lock );

      if( ret < 0 && ( errno == EACCES || errno == EAGAIN) )
         wasLocked = true;
	}
	
	if( ret < 0 ) {
		cerr << "FileLockHelper::read: Cant lock '" << filename_ << "'. Already locked: "  
			  << (wasLocked?"true":"false") <<  endl;
	
		close( fd );
		return false;
	}
	
	int n; 
	char cBuf[512];
	ostringstream ost;
	

	do {
	   do {
	      n = ::read( fd, cBuf , 512 );
	   } while( n<0 && errno==EINTR );

	   if( n > 0 )
	      ost.write( cBuf, n );

	}while( n > 0 );

	lock.l_type = F_UNLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start = 0;
	lock.l_len = 0;
	
	ret = fcntl( fd, F_SETLK, &lock );
	
	if( ret < 0 ) 
		cerr << "FileLockHelper::write: Cant unlock '" << filename_ << "'." <<  endl;
	
	close( fd );
	
	if( n < 0 ) 
		return false;
	
	buf = ost.str();
		
	return true;
}



#else

bool
FileLockHelper::
write( const std::string &buf, bool wait, bool &wasLocked )
{
    boost::mutex::scoped_lock mylock( mutex );
    wasLocked = false;

    if( filename_.empty() )
        return false;

    ofstream fdTmp;
    fdTmp.open( filename_.c_str() );

    if( ! fdTmp.is_open() ) {
        cerr << "FileLockHelper::write: Cant open '" << filename_ << "' for writing." << endl;
        return false;
    }

    ip::file_lock flock( filename_.c_str() );

    try {
        if( wait ) {
            flock.lock();
        }else {
            if( ! flock.try_lock() )
                wasLocked = true;
        }
    }
    catch( const ip::interprocess_exception &ex ) {
        fdTmp.close();
        cerr << "FileLockHelper::write: lock failed '" << filename_
              << "'. Reason: " << ex.what() << endl;
        return false;
    }

    if( wasLocked ) {
        cerr << "FileLockHelper::write: Cant lock '" << filename_
             << "'. Already locked." <<  endl;

        fdTmp.close();
        return false;
    }

    ofstream fd;
    fd.open( filename_.c_str(), ios_base::out | ios_base::trunc | ios_base::ate );

    if( ! fd.is_open() )  {
        cerr << "FileLockHelper::write: Cant truncate the file '" << filename_ << "'." << endl;
        fdTmp.close();
        flock.unlock();
    }

    fdTmp.close(); //We dont need this anymore

    cerr << "FileLockHelper::write: buf[" << buf << "]" << endl;

    fd.write( buf.c_str(), buf.size() );
    fd.flush();
    fd.close();

    try {
        flock.unlock();
    }
    catch( const ip::interprocess_exception &ex ) {
        cerr << "FileLockHelper::write: unlock failed '" << filename_
             << "'. Reason: " << ex.what() << endl;
    }

    return true;
}

bool
FileLockHelper::
read( std::string &buf, bool wait, bool &wasLocked )
{
    boost::mutex::scoped_lock mylock( mutex );
    int n;

    wasLocked = false;

    if( filename_.empty() )
        return false;

    ifstream fd( filename_.c_str() );

    if( ! fd.is_open() ) {
        cerr << "FileLockHelper::read: Cant open '" << filename_ << "' for reading." << endl;
        return false;
    }

    ip::file_lock flock( filename_.c_str() );

    try {
        if( wait ) {
            flock.lock_sharable();
        }else {
            if( ! flock.try_lock_sharable() )
                wasLocked = true;
            }
        }
    catch( const ip::interprocess_exception &ex ) {
        fd.close();
        cerr << "FileLockHelper::read: lock failed '" << filename_
             << "'. Reason: " << ex.what() << endl;
        return false;
    }

    if( wasLocked ) {
        cerr << "FileLockHelper::read: Cant lock '" << filename_
             << "'. Already locked." <<  endl;

        fd.close();
        return false;
    }

    fd.seekg (0, fd.end);
    n = fd.tellg();
    fd.seekg (0, fd.beg);

    char *tmp = new char [n+1];

    std::cout << "FileLockHelper::read: Reading " << n << " characters from file '"
              << filename_ << "'." <<endl;

    fd.read( tmp, n );
    tmp[n]='\0';

    buf = tmp;
    delete[] tmp;

    try {
        flock.unlock_sharable();
    }
    catch( const ip::interprocess_exception &ex ) {
        cerr << "FileLockHelper::read: unlock failed '" << filename_
             << "'. Reason: " << ex.what() << endl;
    }


    return true;
}




#endif
}

