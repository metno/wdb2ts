#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <FileLockHelper.h>
#include <iostream>
#include <sstream>

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
		ret = fcntl( fd, F_SETLKW, &lock );
		
		if( ret < 0 && ( errno == EACCES || errno == EAGAIN) )
			wasLocked = true;
	}else
		ret = fcntl( fd, F_SETLK, &lock );
	
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
		n = ::write( fd, buf.data(), buf.size() );
		
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
		ret = fcntl( fd, F_SETLKW, &lock );
		
		if( ret < 0 && ( errno == EACCES || errno == EAGAIN) )
			wasLocked = true;
	}else
		ret = fcntl( fd, F_SETLK, &lock );
	
	if( ret < 0 ) {
		cerr << "FileLockHelper::read: Cant lock '" << filename_ << "'. Already locked: "  
			  << (wasLocked?"true":"false") <<  endl;
	
		close( fd );
		return false;
	}
	
	int n; 
	char cBuf[512];
	ostringstream ost;
	
	while( (n = ::read( fd, cBuf , 512 )) > 0 ) 
		ost.write( cBuf, n );
		


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


boost::posix_time::ptime 
FileLockHelper::
modifiedTime() const
{
	struct stat statBuf;
	
	if( filename_.empty() ) 
		throw runtime_error("No filename is given.");
	
	int ret = stat( filename_.c_str(), &statBuf );
	
	if( ret < 0 ) {
		if( errno == ENOENT ) 
			return boost::posix_time::ptime();
		
		char errBuf[512];
		
		if( ::strerror_r( errno, errBuf, 512 ) < 0 ) { 
			throw runtime_error( "stat: Unknown error!");
		} else {
			errBuf[511] = '\0';
			throw runtime_error( errBuf );
		}
	}
			
	return  boost::posix_time::from_time_t( statBuf.st_mtim.tv_sec );
}

}

