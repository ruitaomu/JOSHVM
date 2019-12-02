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
#ifdef __cplusplus
extern "C" {
#endif

#include "javacall_os.h"
#include <rthw.h>

#define RT_THREAD_LIBC_WORKAROUND

/*
 * Initialize the OS structure.
 * This is where timers and threads get started for the first
 * real_time_tick event, and where signal handlers and other I/O
 * initialization should occur.
 *
*/
void javacall_os_initialize(void){
    return;
}


/*
 * Performs a clean-up of all threads and other OS related activity
 * to allow for a clean and complete restart.  This should undo
 * all the work that initialize does.
 */
void javacall_os_dispose(){
    return;
}

/** 
 * javacall_os_flush_icache is used, for example, to flush any caches used by a
 * code segment that is deoptimized or moved during a garbage collection.
 * flush at least [address, address + size] (may flush complete icache).
 *
 * @param address   Start address to flush
 * @param size      Size to flush
 */
void javacall_os_flush_icache(unsigned char* address, int size) {
}

void javacall_os_hardware_reset() {
	rt_hw_cpu_reset();
}

#ifdef RT_THREAD_LIBC_WORKAROUND
void __libc_fini_array(void)
{
    /* we not use __libc fini_aray to finallize C++ objects */

	/* M@x: It's a workaround against RT-Thread newlib.
	 *      Or, link would fail for being linked with arm libc's
	 *      __libc_fini_array(), which would in turn failed 
	 *      __fini() not found.
	 */
}
#endif

#ifdef __cplusplus
}
#endif



