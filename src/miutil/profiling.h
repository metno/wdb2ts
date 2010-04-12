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
