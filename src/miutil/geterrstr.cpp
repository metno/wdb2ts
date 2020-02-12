/*
 * geterrstr.cpp
 *
 *  Created on: Sep 14, 2017
 *      Author: borgem
 */

/*
#ifdef _GNU_SOURCE
#undef _GNU_SOURCE
#endif
*/

#include <memory>
#include <string.h>
#include <errno.h>


namespace miutil {

#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600) && ! _GNU_SOURCE
std::string geterrstr(int error_number)
{
	using namespace std;
	int myError;
	int n=128;

	do {
		unique_ptr<char> buf(new char[n]);

		myError = strerror_r(error_number, buf.get(), n);

		//The error returned is depending on the libc (glibc) version.

		if( myError == 0 ) {
			return buf.get();
		} else if( myError<0) { // version  < glibc 2.13
			myError = errno;
		}

		if( myError == ERANGE) {
			n *= 2; //Double the buffer size, and try again;
		} else {
			return "Unknown error number.";
		}
	} while(true);

	return "";
}
#else
std::string geterrstr(int error_number)
{
	using namespace std;
	int n=1024;

	unique_ptr<char> buf(new char[n]);

	return strerror_r(error_number, buf.get(), n);
}


#endif

}



