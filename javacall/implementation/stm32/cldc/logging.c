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

#include "javacall_logging.h"


#ifdef __cplusplus
extern "C" {
#endif

#include "javacall_serial.h"

static int JAVACALL_REPORT_LEVEL = JAVACALL_LOGGING_INFORMATION;
    
static int _enable_log_output = 1;
static int _init_log = 0;
static char outputbuf[256] = {'\0'};

/**
* Prints out a string to a system specific output stream
*
* @param s a NULL terminated character buffer to be printed
*/
void javacall_print(const char *s) {
	static javacall_handle hPort = JAVACALL_INVALID_HANDLE;

	if (!_init_log) {
		_init_log = 1;

		if (JAVACALL_OK != javacall_serial_open_start("COM0", 115200, JAVACALL_SERIAL_BITS_PER_CHAR_8, &hPort, NULL)) {
			hPort = JAVACALL_INVALID_HANDLE;
			_init_log = 0;
		}
	}
	int tmp;
	javacall_serial_write_start(hPort,s, strlen(s), &tmp,NULL);
}

void javacall_printf(const char * format,...) {
#if 1
   va_list args;
   va_start (args, format);
   vsnprintf(outputbuf, 255, format, args);
   javacall_print(outputbuf);
   va_end (args);
#endif
}

void javacall_vprintf(const char *format, va_list vl) {
#if 1
   vsnprintf(outputbuf, 255, format, vl);
   javacall_print(outputbuf);
#endif
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
