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

/*
 *
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

#include <josh_logging.h>

/**
 * @file
 *
 * Logging and tracing API implementation used to output text to the
 * default display device. In the VM, terminal output for Java methods
 * <tt>System.out.println()</tt> and
 * <tt>Java.lang.Throwable.printStackTrace()</tt> happen via a call to
 * the <tt>JVMSPI_PrintRaw()</tt> method. This implementation emulates
 * the mechanism used there.
 *
 * To redirect the logging output to a file or a stream other than
 * stdout modify the behavior of the <tt>JVMSPI_PrintRaw()</tt> method.
 */

/**
 * Report a message to the Logging service.  On the linux emulator
 * this will end up going to stdout.  On the Zaurus device it will
 * be written to a file.
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
 * <code> reportToLog(severity, chanID, "%s", message); </code>

 * @param severity severity level of report
 * @param channelID area report relates to, from midp_constants_data.h
 * @param message detail message to go with the report
 *                should not be NULL
 */
void reportToLog(int severity, int channelID, const char* message, ...) {
}
    /**
    * Gets the severity per channel using the property mechanism.
    */
int get_allowed_severity_c(int channelID) {
    return(LOG_DISABLED);
}

