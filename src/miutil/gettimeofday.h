#ifndef __GETTIMEOFDAY_H__
#define __GETTIMEOFDAY_H__

namespace miutil {

/**
 * gettimeofday returns the number of second since 1970.1.1 00:00:00 as a double
 * with microseconds resolution.
 * 
 * @return a number > 0 on success and a number <0 on failure.
 */
double
gettimeofday();

}

#endif /*GETTIMEOFDAY_H_*/
