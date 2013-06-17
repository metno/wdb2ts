
#ifndef __VISIBILITY_H__
#define __VISIBILITY_H__

#define DSO_NOEXPORT __attribute__ ((visibility ("hidden")))
#define DSO_EXPORT __attribute__ ((visibility ("default")))

#endif
