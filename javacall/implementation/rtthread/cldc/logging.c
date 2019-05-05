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
#include "javacall_logging.h"

#ifdef __cplusplus
extern "C" {
#endif

#define JC_LOG_BUFLEN (80)

#define JC_LOG_USE_UART 1

#if JC_LOG_USE_UART
extern void printToUart(char* buffer);
#define JC_LOG_PRINTBUF(buffer) printToUart(buffer)
#else
#define JC_LOG_PRINTBUF(buffer) rt_kprintf("%s", buffer)
#endif

static int _enable_log_output = 1;
static int JAVACALL_REPORT_LEVEL = JAVACALL_REPORT_LEVEL_DEFAULT;

/**
* Prints out a string to a system specific output stream
*
* @param s a NULL terminated character buffer to be printed
*/
void javacall_print(const char *s) {
    if (!_enable_log_output) {
        return;
    }

#if JC_LOG_USE_UART
    JC_LOG_PRINTBUF((char *)s);
#else
    {
    static char buffer[JC_LOG_BUFLEN + 2] = {0};
    static int buff_index = 0;

    int index = buff_index;
    while (*s != '\0') {
        if ((*s == '\n') || (*s == '\r')) {
            buffer[index] = '\n';
            index++;
            buffer[index] = '\0';
            JC_LOG_PRINTBUF(buffer);
            index = 0;
        } else {
            buffer[index] = *s;
            index++;
        }

        if (index == JC_LOG_BUFLEN) {
            buffer[index] = '\0';
            JC_LOG_PRINTBUF(buffer);
            index = 0;
        }
        s++;
    }
    buff_index = index;
    }
#endif
}

void javacall_printf(const char * format,...) {
    char outputbuf[256] = {'\0'};
    va_list args;
    va_start (args, format);
    rt_vsnprintf(outputbuf, 255, format, args);
    javacall_print(outputbuf);
    va_end (args);
}

void javacall_vprintf(const char *format, va_list vl) {
    char outputbuf[256] = {'\0'};
    rt_vsnprintf(outputbuf, 255, format, vl);
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
        javacall_printf(JAVACALL_LOGGING_HEADER, severity, channelID);
        va_start(args, format);
        javacall_vprintf(format, args);
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
