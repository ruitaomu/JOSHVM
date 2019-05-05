/*
 * Copyright (C) Max Mu
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2, as published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License version 2 for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 * 
 * Please visit www.joshvm.org if you need additional information or
 * have any questions.
 */
 
#include <rtthread.h>
#include <rtdevice.h>
#include <time.h>
#include <stdio.h>
#include "javacall_time.h"

static int _RT_RTC_Initialized = 0;

typedef struct {
	rt_timer_t handle;
	javacall_callback_func func;	
} javacall_timer;

static void timer_callback(void* handle){
	javacall_timer* p = (javacall_timer*)handle;
    javacall_callback_func func = (javacall_callback_func)p->func;
    func(handle);
}

/**
 *
 * Create a native timer to expire in wakeupInSeconds or less seconds.
 * If a later timer exists, cancel it and create a new timer
 *
 * @param wakeupInMilliSecondsFromNow time to wakeup in milli-seconds
 *                              relative to current time
 *                              if -1, then ignore the call
 * @param cyclic <tt>1</tt> indicates that the timer should be repeated cuclically,
 *               <tt>0</tt> indicates that this is a one-shot timer that should call the callback function once
 * @param func callback function should be called in platform's context once the timer
 *			   expires
 * @param handle A pointer to the returned handle that will be associated with this timer
 *               On success.
 *
 * @return on success returns <tt>JAVACALL_OK</tt>,
 *         or <tt>JAVACALL_FAIL</tt> or negative value on failure
 */
javacall_result javacall_time_initialize_timer(
                    int                      wakeupInMilliSecondsFromNow,
                    javacall_bool            cyclic,
                    javacall_callback_func   func,
                    /*OUT*/ javacall_handle	*handle){

	rt_timer_t  hTimer;
	javacall_uint64 ticks;
	javacall_timer* timer_handle;

    if (!handle || !func) {
        return JAVACALL_INVALID_ARGUMENT;
    }

	timer_handle = (javacall_timer*)javacall_malloc(sizeof(javacall_timer));
	if (timer_handle == NULL) {
		return JAVACALL_FAIL;
	}	

	ticks = wakeupInMilliSecondsFromNow*RT_TICK_PER_SECOND/1000LL;

    hTimer = rt_timer_create("JAVTIMER",  
                        timer_callback, 
                        timer_handle,  
                        (rt_tick_t)ticks, 
                        cyclic?RT_TIMER_FLAG_PERIODIC:RT_TIMER_FLAG_ONE_SHOT); 

    if (RT_NULL == hTimer) {
		javacall_free(timer_handle);
        *handle = NULL;
        return JAVACALL_FAIL;
    } else {
    	timer_handle->handle = hTimer;
		timer_handle->func = func;
        *handle = (javacall_handle)timer_handle;
		rt_timer_start(hTimer); // start timer right now
        return JAVACALL_OK;
    }
}


/**
 *
 * Disable a set native timer
 * @param handle The handle of the timer to be finalized
 *
 * @return on success returns <tt>JAVACALL_OK</tt>,
 *         <tt>JAVACALL_FAIL</tt> or negative value on failure
 */
javacall_result javacall_time_finalize_timer(javacall_handle handle) {

	if (NULL == handle) {
        return JAVACALL_INVALID_ARGUMENT;
    }

    if (RT_EOK == rt_timer_delete(((javacall_timer*)handle)->handle)) {
		javacall_free(handle);
        return JAVACALL_OK;
    }

    return JAVACALL_FAIL;
}

/**
 * Return local timezone ID string. This string is maintained by this
 * function internally. Caller must NOT try to free it.
 *
 * @return Local timezone ID string pointer. The ID string should be in the
 *         format of GMT+/-??:??. For example, GMT-08:00 for PST.
 */
char* javacall_time_get_local_timezone(void){
	static char tzstring[128];
/*	struct timezone tz = {0};
	int tz_hour, tz_min;

	gettimeofday(0, &tz);
	tz_hour = (int)(tz.tz_minuteswest / 60);
	tz_min = (int)(tz.tz_minuteswest - tz_hour*60);
	
	if (tz_hour<0){
		rt_snprintf(tzstring, 128, "GMT-%02d:%02d",tz_hour,-tz_min);
	}else{
		rt_snprintf(tzstring, 128, "GMT+%02d:%02d",tz_hour,tz_min);
	}*/

	rt_snprintf(tzstring, 128, "GMT+08:00");
	return tzstring;
}


/**
 * returns number of milliseconds elapsed since midnight(00:00:00), January 1, 1970,
 *
 * @return milliseconds elapsed since midnight (00:00:00), January 1, 1970
 */
javacall_int64 /*OPTIONAL*/ javacall_time_get_milliseconds_since_1970(void){
#if 0
	struct timespec tp;
	
	if (!_RT_RTC_Initialized) {
		clock_time_system_init();
		_RT_RTC_Initialized = 1;
	}
	clock_gettime(CLOCK_REALTIME, &tp); 
	return (javacall_int64)tp.tv_sec*1000LL+tp.tv_nsec/1000LL;
#endif
	return (javacall_int64)TICKS_TO_MILISECS(rt_tick_get());
}


/*
 *
 * Temporarily disable timer interrupts. This is called when
 * the VM is about to sleep (when there's no Java thread to execute)
 *
 * @param handle timer handle to suspend
 *
 */
void javacall_time_suspend_ticks(javacall_handle handle) {
	rt_timer_stop(((javacall_timer*)handle)->handle);
}

/*
 *
 * Enable  timer interrupts. This is called when the VM
 * wakes up and continues executing Java threads.
 *
 * @param handle timer handle to resume
 *
 */
void javacall_time_resume_ticks(javacall_handle handle) {
	rt_timer_start(((javacall_timer*)handle)->handle);
}

/*
 * Suspend the current process sleep for ms milliseconds
 */
void javacall_time_sleep(javacall_uint64 ms) {
	rt_thread_delay(RT_TICK_PER_SECOND*ms/1000);
}

