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
#include "javacall_time.h"
#include <sys/timeb.h>
#include <unistd.h>
#include <pthread.h>


typedef struct {
	javacall_callback_func func;
	int wakeupInMilliSecondsFromNow;
	pthread_t pid;
	javacall_bool cyclic;
} timer_arg;

void *timer_thrd_func(void *arg){	
	timer_arg *t = (timer_arg*)arg;
	int wakeupInMilliSecondsFromNow = t->wakeupInMilliSecondsFromNow;
	javacall_callback_func func = t->func;
	javacall_bool cyclic = t->cyclic;

	do {
		pthread_testcancel();
		usleep(wakeupInMilliSecondsFromNow*1000);
		pthread_testcancel();
		if (func) {
			func((javacall_handle)arg);
		}
	} while (cyclic);
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

  pthread_t tid;
  timer_arg* t;

  if (!handle || !func) {
		return JAVACALL_INVALID_ARGUMENT;
  }
	
  t = (timer_arg*)javacall_malloc(sizeof(timer_arg));
  if (t == NULL) {
	return JAVACALL_OUT_OF_MEMORY;
  }

  t->wakeupInMilliSecondsFromNow = wakeupInMilliSecondsFromNow;
  t->func = func;
  t->cyclic = cyclic;
	
  if (pthread_create(&tid, NULL, timer_thrd_func, (void*)t)!=0) {
  	javacall_free(t);
  	return JAVACALL_FAIL;
  }

  t->pid = tid;

  *handle = (javacall_handle)t;
  
  return JAVACALL_OK;
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

	pthread_t pid = (pthread_t)((timer_arg*)handle)->pid;
	pthread_cancel(pid);
	pthread_join(pid, NULL);
	javacall_free((void*)handle);
	
	return JAVACALL_OK;
}

/**
 * Return local timezone ID string. This string is maintained by this
 * function internally. Caller must NOT try to free it.
 *
 *
 * @return Local timezone ID string pointer. The ID string should be in the
 *         format of GMT+/-??:??. For example, GMT-08:00 for PST.
 */
char* javacall_time_get_local_timezone(void){
	static char tzstring[128];
	int diff_in_hours,diff_in_minutes;
	struct timeb tstruct;
	tzset();
	ftime( &tstruct );

	diff_in_hours=-(int)(tstruct.timezone/60);
	diff_in_minutes=tstruct.timezone%60;
	if (diff_in_minutes<0) {diff_in_minutes=-diff_in_minutes;}

	if (tstruct.dstflag) { diff_in_hours++; }
	if (diff_in_hours<-12) {diff_in_hours+=24;}

	if (diff_in_hours<0) {
	   snprintf(tzstring, 128, "GMT%d:%02d",diff_in_hours,diff_in_minutes);
	}else{
	   snprintf(tzstring, 128, "GMT+%d:%02d",diff_in_hours,diff_in_minutes);
	}
	return tzstring;
}


/**
 * returns number of milliseconds elapsed since midnight(00:00:00), January 1, 1970,
 *
 * @return milliseconds elapsed since midnight (00:00:00), January 1, 1970
 */
javacall_int64 /*OPTIONAL*/ javacall_time_get_milliseconds_since_1970(void){
	struct timeb tp;
    ftime(&tp);
	return ((javacall_int64)tp.time)*1000LL + (javacall_int64)tp.millitm;
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
}

/*
 * Suspend the current process sleep for ms milliseconds
 */
void javacall_time_sleep(javacall_uint64 ms) {
}

