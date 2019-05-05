/*
 *
 * Copyright  1990-2007 Sun Microsystems, Inc. All Rights Reserved.
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

#ifndef __JAVACALL_LOGGING_H_
#define __JAVACALL_LOGGING_H_

/**
 * @file javacall_logging.h
 * @ingroup MandatoryLogging
 * @brief Javacall interfaces for logging
 */

#include "javacall_defs.h"
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

#define JAVACALL_LOGGING_HEADER "JC:<level:%d><channel:%d> "
#define JAVACALL_REPORT_LEVEL_DEFAULT JAVACALL_LOGGING_ERROR

#define javacall_logging_printf		javacall_logging_printf_nodebug
#define javacall_logging_vprintf	javacall_logging_vprintf_nodebug

/**
 * JavaCall logging severity levels, these values are assumed to be equal
 * to similar MIDP logging definitions.
 */

#define JAVACALL_LOGGING_INFORMATION 0
#define JAVACALL_LOGGING_WARNING  1
#define JAVACALL_LOGGING_ERROR  2
#define JAVACALL_LOGGING_CRITICAL  3
#define JAVACALL_LOGGING_DISABLED  4

/**
 * @enum javacall_logging_channel
 * @brief JavaCall logging channels
 *
 */
typedef enum {
    JC_NONE 	= 0,
    JC_TIME 	= 1,
    JC_FILE 	= 2,
    JC_WMA  	= 3,
    JC_MMA  	= 4,
    JC_PIM  	= 5,
    JC_FC   	= 6,
    JC_AMMS 	= 7,
    JC_CHAPI	= 8,
    JC_AMS  	= 9,
    JC_BT   	=10,
    JC_FONT 	=11,
    JC_LCD  	=12,
    JC_SOCKET	=13,
    JC_MMS  	=14,
    JC_MAINLOOP	=15,
    JC_PERFORMANCE=16,
    JC_LIFECYCLE=17,
    JC_EVENTS	=18,
    JC_MEMORY	=19,
    JC_SENSOR	=20,
    JC_CMS  	=21,
    JC_LOCATION	=22,
    JC_SECURITY	=23,
    JC_SERIAL	=24,
    JC_MOBILE	=25,
    JC_SIMCARD	=26,
    JC_SYSTEMEVENT=27,
    JC_CONTACTLESS=28,
    JC_DAAPI	=29,
    JC_PROXY	=30,
    JC_NETWORK	=31,
    JC_DUI		=32,
    JC_TUI		=33,
    JC_CORE		=34,
    JC_PROTOCOL =35,
    JC_MAX

} javacall_logging_channel;


/** @defgroup MandatoryLogging Logging API
 *  @ingroup CLDC
 *
 *  @{
 */

/**
 * Prints out a string to a system specific output stream
 *
 * @param s a NULL terminated character buffer to be printed
 */
void javacall_print(const char *s);

/**
 * Prints out a string to a system specific output stream
 *
 * @param format a string to be used for define the output string format
 * @param ... variable params
 */
void javacall_printf(const char * format,...);

/**
 * Initializes Javacall logging subsytem.
 */
void javacall_logging_initialize(void);

/**
 * Report a message to the Logging service.
 *
 * The <code>message</code> parameter is treated as a format
 * string to the standard C library call printf would be, with
 * conversion specifications (%s, %d, %c, etc) causing the
 * conversion and output of each successive argument after
 * <code>message</code>  As with printf, having a conversion
 * character in <code>message</code> without an associated argument
 * following it is an error.
 *
 * To ensure that no character in <code>message</code> is
 * interpreted as requiring conversion, a safe way to call
 * this method is:
 * <code> javacall_logging_printf(severity, chanID, "%s", message); </code>

 * @param severity severity level of report
 * @param channelID area report relates to
 * @param format detail message to go with the report
 *                should not be NULL
 */
void javacall_logging_printf(int severity, javacall_logging_channel channelID, const char *format, ...);

void javacall_logging_vprintf(int severity, javacall_logging_channel channelID, const char *format, va_list args);

void javacall_vprintf(const char *format, va_list args);

/**
 * Enable/Disable logging output
 *
 * @param switch_on true for enable, false for disable logging output
 */
javacall_bool javacall_logging_enable(javacall_bool switch_on);

void javacall_logging_setLevel(int report_level);

int javacall_logging_getLevel();

/** @} */


#ifdef __cplusplus
}
#endif

#endif
