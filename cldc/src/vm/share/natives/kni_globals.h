/*
 *   
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

#ifndef KNI_GLOBALS_H
#define KNI_GLOBALS_H

/**
 * @file
 * @ingroup core_kni
 *
 * @brief KNI global declarations
 */
#ifdef __cplusplus
extern "C" {
#endif
/**
 * The size of gKNIBuffer
 * @see gKNIBuffer
 */
#define KNI_BUFFER_SIZE 512

/**
 * Internal utility buffer for use in KNI functions.
 * Integrity of information stored there across MIDP
 * function calls is NOT guaranteed.
 * Use it if you need a temporary buffer, but do not want to
 * allocate memory dynamically, nor to create a dedicated static buffer.
 *
 * This buffer is not used to pass data from one function to another.
 */
extern unsigned char gKNIBuffer[KNI_BUFFER_SIZE];

/**
 * 'C' string for java.lang.OutOfMemoryError.
 */
extern const char* const KNIOutOfMemoryError;

/**
 * 'C' string for java.lang.IllegalArgumentException.
 */
extern const char* const KNIIllegalArgumentException;

/**
 * 'C' string for java.lang.IllegalStateException.
 */
extern const char* const KNIIllegalStateException;

/**
 * 'C' string for java.lang.RuntimeException.
 */
extern const char* const KNIRuntimeException;

/**
 * 'C' string for java.lang.ArrayIndexOutOfBoundsException.
 */
extern const char* const KNIArrayIndexOutOfBoundsException;

/**
 * 'C' string for java.lang.StringIndexOutOfBoundsException.
 */
extern const char* const KNIStringIndexOutOfBoundsException;

/**
 * 'C' string for java.lang.NullPointerException.
 */
extern const char* const KNINullPointerException;

/**
 * 'C' string for java.lang.ClassNotFoundException.
 */
extern const char* const KNIClassNotFoundException;

/**
 * 'C' string for java.io.IOException.
 */
extern const char* const KNIIOException;

/**
 * 'C' string for java.io.InterruptedIOException.
 */
extern const char* const KNIInterruptedIOException;

/**
 * 'C' string for javax.microedition.io.ConnectionNotFoundException.
 */
extern const char* const KNIConnectionNotFoundException;

/** 'C' string for org.joshvm.j2me.dio.InvalidDeviceConfigException */
extern const char* const KNIInvalidDeviceConfigException;

/** 'C' string for org.joshvm.j2me.dio.UnsupportedAccessModeException */
extern const char* const KNIUnsupportedAccessModeException;

/** 'C' string for org.joshvm.j2me.dio.UnavailableDeviceException */
extern const char* const KNIUnavailableDeviceException;

/** 'C' string for org.joshvm.j2me.dio.DeviceNotFoundException */
extern const char* const KNIDeviceNotFoundException;

#ifdef __cplusplus
}
#endif
#endif /* KNI_GLOBALS_H */

