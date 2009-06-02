#ifndef __PROFILING_H__
#define __PROFILING_H__


#ifdef __MI_PROFILE__
#include <iostream>
#include <boost/thread/tss.hpp> 
#include "Timer.h"

#ifndef MI_PROFILE_NAMESPACE
#define MI_PROFILE_NAMESPACE mi_profile__
#endif

#define DECLARE_MI_PROFILE namespace MI_PROFILE_NAMESPACE {\
	boost::thread_specific_ptr<miutil::Timer> my_mi_profile_timer__;\
}



#define INIT_MI_PROFILE(nTimers){  \
							       	miutil::Timer *my_mi_profile_timer_ptr__=MI_PROFILE_NAMESPACE::my_mi_profile_timer__.get();\
									\
									if(!my_mi_profile_timer_ptr__) {\
										MI_PROFILE_NAMESPACE::my_mi_profile_timer__.reset(new miutil::Timer((nTimers))); \
										std::cerr << "Profile timer created!" << std::endl;	\
									}else\
										my_mi_profile_timer_ptr__->reset();\
                                }
                                     
#define DEFINE_MI_PROFILE namespace MI_PROFILE_NAMESPACE {\
	extern  boost::thread_specific_ptr<miutil::Timer> my_mi_profile_timer__;\
}

#define USE_MI_PROFILE miutil::Timer *my_mi_profile_timer_tmp_ptr__=MI_PROFILE_NAMESPACE::my_mi_profile_timer__.get();
#define MI_PROFILE_PTR my_mi_profile_timer_tmp_ptr__;


#define MARK_MI_PROFILE  my_mi_profile_timer_tmp_ptr__->mark()
#define MARK_ID_MI_PROFILE(id)  my_mi_profile_timer_tmp_ptr__->mark((id))
#define START_MARK_MI_PROFILE(id)  my_mi_profile_timer_tmp_ptr__->markStart((id))
#define STOP_MARK_MI_PROFILE(id)  my_mi_profile_timer_tmp_ptr__->markStop((id))
#define PRINT_MI_PROFILE std::cerr << *my_mi_profile_timer_tmp_ptr__
#define PRINT_MI_PROFILE_SUMMARY( ostrm ) my_mi_profile_timer_tmp_ptr__->print( (ostrm) )
#else
#define DECLARE_MI_PROFILE
#define DEFINE_MI_PROFILE
#define INIT_MI_PROFILE(nTimers)
#define USE_MI_PROFILE
#define MARK_MI_PROFILE
#define MARK_ID_MI_PROFILE(id)
#define START_MARK_MI_PROFILE(id)
#define STOP_MARK_MI_PROFILE(id)
#define PRINT_MI_PROFILE
#define PRINT_MI_PROFILE_SUMMARY( ostrm )
#define MI_PROFILE_PTR
#endif

#endif /*__PROFILING_H__*/
