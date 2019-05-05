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

/**
* @file
*
* win32 implementation for logging javacall functions
*/

#include <stdio.h>
#include <stdarg.h>

#include "javacall_logging.h"

#ifdef __cplusplus
extern "C" {
#endif
    
static int _enable_log_output = 1;
static char outputbuf[256] = {'\0'};

static int JAVACALL_REPORT_LEVEL = JAVACALL_REPORT_LEVEL_DEFAULT;

/**
* Prints out a string to a system specific output stream
*
* @param s a NULL terminated character buffer to be printed
*/
void javacall_print(const char *s) {
	printf(s);
}

void javacall_printf(const char * format,...) {
   va_list args;
   va_start (args, format);
   vsnprintf(outputbuf, 255, format, args);
   javacall_print(outputbuf);
   va_end (args);
}

void javacall_vprintf(const char *format, va_list vl) {
   vsnprintf(outputbuf, 255, format, vl);
   javacall_print(outputbuf);
}

javacall_bool javacall_logging_enable(javacall_bool switch_on) {
	javacall_bool prev = _enable_log_output;
	_enable_log_output = switch_on;
	return prev;
}

void javacall_logging_setLevel(int report_level) {
	JAVACALL_REPORT_LEVEL = report_level;
}

int javacall_logging_getLevel() {
	return JAVACALL_REPORT_LEVEL;
}

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
void javacall_logging_printf(int severity, javacall_logging_channel channelID, const char *format, ...) {
	if (JAVACALL_REPORT_LEVEL <= severity) {
		va_list args;
	   	va_start(args, format);
		javacall_logging_vprintf(severity, channelID, format, args);
		va_end(args);
	}
}

void javacall_logging_vprintf(int severity, javacall_logging_channel channelID, const char *format, va_list args) {
	if (JAVACALL_REPORT_LEVEL <= severity) {
		javacall_printf(JAVACALL_LOGGING_HEADER, severity, channelID);
		javacall_vprintf(format, args);
	}
}

 
#ifdef __cplusplus
}
#endif

