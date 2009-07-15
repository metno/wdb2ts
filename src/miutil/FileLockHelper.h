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
