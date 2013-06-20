#include <boost/version.hpp>
#include "fileutil.h"

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
#  define USE_BOOST 0
#  include <sys/stat.h>
#  include <sys/types.h>
#  include <utime.h>
#  include <unistd.h>
#endif

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


#endif
}


namespace miutil {

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
setmtime( const std::string &file,
          const boost::posix_time::ptime &newModificationTime )
{

}

bool
setatime( const std::string &file,
          const boost::posix_time::ptime &newAccessTime )
{

}

}

