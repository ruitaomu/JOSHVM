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


static int _timer_cyclic_period = 0;
static javacall_callback_func _timer_callback_func = NULL;
static int _timer_countdown = -1;
static javacall_bool _timer_callback_enabled = JAVACALL_FALSE;

void TIM6_PeriodElapsedCallback()
{  
  if (_timer_callback_enabled == JAVACALL_TRUE) {
    if (_timer_countdown > 0) {
		_timer_countdown--;
	} else {
		//Invoking callback func
		_timer_callback_func(1);
		
		if (_timer_cyclic_period > 0) {
			//Restore timer count
			_timer_countdown = _timer_cyclic_period;
		} else {
			//No more timer count down
			_timer_callback_enabled = JAVACALL_FALSE;
		}
	}
  }
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

  _timer_cyclic_period = (cyclic==JAVACALL_TRUE)?wakeupInMilliSecondsFromNow:0;
  _timer_callback_func = func;  
  _timer_countdown = wakeupInMilliSecondsFromNow;
  _timer_callback_enabled = JAVACALL_TRUE;
  
  *handle = (javacall_handle)1;
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
	_timer_callback_enabled = JAVACALL_FALSE;
	return JAVACALL_OK;
}

/**
 * Return local timezone ID string. This string is maintained by this
 * function internally. Caller must NOT try to free it.
 *
 * This function should handle daylight saving time properly. For example,
 * for time zone America/Los_Angeles, during summer time, this function
 * should return GMT-07:00 and GMT-08:00 during winter time.
 *
 * @return Local timezone ID string pointer. The ID string should be in the
 *         format of GMT+/-??:??. For example, GMT-08:00 for PST.
 */
char* javacall_time_get_local_timezone(void){
	return "GMT+08:00";
}


/**
 * returns number of milliseconds elapsed since midnight(00:00:00), January 1, 1970,
 *
 * @return milliseconds elapsed since midnight (00:00:00), January 1, 1970
 */
javacall_int64 /*OPTIONAL*/ javacall_time_get_milliseconds_since_1970(void){
	//TODO: FIX ME! Will overflow in days!
	return (javacall_int64)HAL_GetTick();
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

