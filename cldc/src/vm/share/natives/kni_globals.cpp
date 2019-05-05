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

#include "incls/_precompiled.incl"
#include "incls/_kni_globals.cpp.incl"

/**
 * Global buffers definitions
 */
unsigned char gKNIBuffer[KNI_BUFFER_SIZE];

/** 'C' string for java.lang.OutOfMemoryError */
const char* const KNIOutOfMemoryError = "java/lang/OutOfMemoryError";
/** 'C' string for java.lang.IllegalArgumentException */
const char* const KNIIllegalArgumentException = "java/lang/IllegalArgumentException";
/** 'C' string for java.lang.IllegalStateException */
const char* const KNIIllegalStateException = "java/lang/IllegalStateException";
/** 'C' string for java.lang.RuntimeException */
const char* const KNIRuntimeException = "java/lang/RuntimeException";
/** 'C' string for java.lang.ArrayIndexOutOfBoundsException */
const char* const KNIArrayIndexOutOfBoundsException = "java/lang/ArrayIndexOutOfBoundsException";
/** 'C' string for java.lang.StringIndexOutOfBoundsException */
const char* const KNIStringIndexOutOfBoundsException = "java/lang/StringIndexOutOfBoundsException";
/** 'C' string for java.lang.NullPointerException */
const char* const KNINullPointerException = "java/lang/NullPointerException";
/** 'C' string for java.lang.ClassNotFoundException */
const char* const KNIClassNotFoundException = "java/lang/ClassNotFoundException";
/** 'C' string for java.io.IOException */
const char* const KNIIOException = "java/io/IOException";
/** 'C' string for java.io.InterruptedIOException */
const char* const KNIInterruptedIOException = "java/io/InterruptedIOException";
/** 'C' string for javax.microedition.io.ConnectionNotFoundException */
const char* const KNIConnectionNotFoundException = "javax/microedition/io/ConnectionNotFoundException";
/** 'C' string for org.joshvm.j2me.dio.InvalidDeviceConfigException */
const char* const KNIInvalidDeviceConfigException = "org/joshvm/j2me/dio/InvalidDeviceConfigException";
/** 'C' string for org.joshvm.j2me.dio.UnsupportedAccessModeException */
const char* const KNIUnsupportedAccessModeException = "org/joshvm/j2me/dio/UnsupportedAccessModeException";
/** 'C' string for org.joshvm.j2me.dio.UnavailableDeviceException */
const char* const KNIUnavailableDeviceException = "org/joshvm/j2me/dio/UnavailableDeviceException";
/** 'C' string for org.joshvm.j2me.dio.DeviceNotFoundException */
const char* const KNIDeviceNotFoundException = "org/joshvm/j2me/dio/DeviceNotFoundException";
/** 'C' string for org.joshvm.j2me.dio.UnsupportedOperationException */
const char* const KNIUnsupportedOperationException = "org/joshvm/j2me/dio/UnsupportedOperationException";


