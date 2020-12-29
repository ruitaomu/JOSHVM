/*
 * Copyright  1990-2009 Sun Microsystems, Inc. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version
 * 2 only, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License version 2 for more details (a copy is
 * included at /legal/license.txt).
 *
 * You should have received a copy of the GNU General Public License
 * version 2 along with this work; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 * Please contact Sun Microsystems, Inc., 4150 Network Circle, Santa
 * Clara, CA 95054 or visit www.sun.com if you need additional
 * information or have any questions.
 */

#ifndef __JAVACALL_OS_H
#define __JAVACALL_OS_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file javacall_os.h
 * @ingroup OS
 * @brief Javacall interfaces for underlying Operating System
 */

#include "javacall_defs.h"

/**
 * @defgroup OS OS API
 * @ingroup CLDC
 * @{
 */
 
/** 
 * @defgroup MandatoryOS Mandatory OS API 
 * @ingroup OS
 *
 * OS APIs define the functionality for:
 *   - Initialize OS resources, if necessary
 *   - Clean-up of OS resources, if necessary
 *   - Flush i-cache
 * 
 * @{
 */
 
/**
 * Initialize the OS structure.
 * This is where timers and threads get started for the first
 * real_time_tick event, and where signal handlers and other I/O
 * initialization should occur.
 *
 */
void javacall_os_initialize(void);

/**
 * Performs a clean-up of all threads and other OS related activity
 * to allow for a clean and complete restart.  This should undo
 * all the work that initialize does.
 */
void javacall_os_dispose();

/** 
 * javacall_os_flush_icache is used, for example, to flush any caches used by a
 * code segment that is deoptimized or moved during a garbage collection.
 * flush at least [address, address + size] (may flush complete icache).
 * Could be empty implementation if JIT compiler is NOT enabled.
 *
 * @param address   Start address to flush
 * @param size      Size to flush
 */
void javacall_os_flush_icache(unsigned char* address, int size);

void javacall_os_hardware_reset();

int javacall_os_set_system_time(javacall_int64 time);

/** @} */

/** @} */

#ifdef __cplusplus
}
#endif

#endif


