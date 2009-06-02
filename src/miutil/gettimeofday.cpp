#include <sys/time.h>
#include <unistd.h>
#include "gettimeofday.h"

double
miutil::
gettimeofday()
{
	struct timeval tv;
	
	if(gettimeofday(&tv, 0)!=0)
		return -1.0l;
	
	return (double)tv.tv_sec+((double)tv.tv_usec)/1000000.0;
}
