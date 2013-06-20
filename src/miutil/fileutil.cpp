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
namespace fs = boost::filesystem;
namespace s = boost::system;
#else
#  undef USE_BOOST
#  include <sys/stat.h>
#  include <sys/types.h>
#  include <utime.h>
#  include <unistd.h>
#  include <stdio.h>
#endif

namespace pt = boost::posix_time;

namespace {


#ifdef USE_BOOST
bool
renamefileImpl( const std::string &from, const std::string &to )
{
    boost::system::error_code ec;
    fs::rename( from, to, ec);

    return ec.value() == s::errc::success;
}

bool
removefileImpl( const std::string &path )
{
    boost::system::error_code ec;
    fs::remove( path.c_str(), ec);

    return ec.value() == s::errc::success;
}

bool
truncateImpl( const std::string &file )
{
    boost::system::error_code ec;

    fs::resize_file( file, 0, ec );
    return ec.value() == s::errc::success;
}


bool
setmtimeImpl( const std::string &file,
              const boost::posix_time::ptime &newModificationTime )
{
    try {
        boost::system::error_code ec;
        time_t t = miutil::to_time_t( newModificationTime );

        fs::last_write_time( file, t, ec );
        return ec.value() == s::errc::success;
    }
    catch( const std::range_error &er ) {
        return false;
    }
}

boost::posix_time::ptime
getmtimeImpl( const std::string &file )
{
    boost::system::error_code ec;
    time_t t = fs::last_write_time( file, ec );

    if( ec.value()!= s::errc::success )
        return pt::ptime(); //return undefined.
    else
        return pt::from_time_t( t );
}



#else
bool
renamefileImpl( const std::string &from, const std::string &to )
{
    return rename( from.c_str(), to.c_str() ) != -1;
}

bool
removefileImpl( const std::string &path )
{
    return unlink( path.c_str() ) != -1;
}

bool
truncateImpl( const std::string &file )
{
    return truncate( file.c_str(), 0 ) != 0;
}



bool
setmtimeImpl( const std::string &file,
          const boost::posix_time::ptime &newModificationTime )
{
    try {
        time_t t = miutil::to_time_t( newModificationTime );
        struct utimbuf toUtime;

        toUtime.modtime =t;
        toUtime.actime = t;
        return utime( file.c_str(), &toUtime) != -1;
    }
    catch( const std::range_error &er ) {
        return false;
    }
}

boost::posix_time::ptime
getmtimeImpl( const std::string &file )
{
    struct stat fstat;

    if( stat( file.c_str(), &fstat) < 0 )
        return pt::ptime();

    return pt::from_time_t( fstat.st_mtime );
}

#endif
}

namespace miutil {
namespace file {

bool
renamefile( const std::string &from, const std::string &to )
{
    return renamefileImpl( from, to );
}

bool
removefile( const std::string &path )
{
    return removefileImpl( path );

}

bool
truncate( const std::string &file )
{
    return truncateImpl( file );
}


bool
setmtime( const std::string &file,
          const boost::posix_time::ptime &newModificationTime )
{
    return setmtimeImpl( file, newModificationTime );
}

boost::posix_time::ptime
getmtime( const std::string &file )
{
    return getmtimeImpl( file );
}


}
}
