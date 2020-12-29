/*
 *
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

/*
 * OsMisc_javacall.cpp:
 */

#include "incls/_precompiled.incl"
#include "incls/_OsMisc_javacall.cpp.incl"
#if defined(_WIN32_WCE)
#include <windows.h>
#endif

#include <javacall_os.h>

#ifdef __cplusplus
extern "C" {
#endif

const JvmPathChar *OsMisc_get_classpath() {
  /*
   * Returns a system-wide defined classpath. This function is called only
   * if a classpath is not given in parameters to JVM_Start() or JVM_Start2().
   * On Win32 and Linux this function calls getenv("CLASSPATH"). This function
   * is generally not needed in real devices.
   */
  return NULL;
}


void OsMisc_flush_icache(address start, int size) {
#if defined(_WIN32_WCE)
  /* Currently the PocketPC API doesn't seem to support selective
     flushing of the icache => ignore start, size for now */
  BOOL ret = FlushInstructionCache(GetCurrentProcess(), 0, 0);
#else
  javacall_os_flush_icache((unsigned char*)start, size);
#endif
}

#if !defined(PRODUCT) || ENABLE_TTY_TRACE || USE_DEBUG_PRINTING
const char *OsMisc_jlong_format_specifier() {
#if defined(_WIN32_WCE)
  return "%I64d";
#endif
  /*
   * Return jlong-specifier prefixes are used with type characters in
   * printf functions or wprintf functions to specify interpretation
   * of jlong e.g. for win32 is "%I64d", for linux is "%lld"
   */
  return "%lld";
}

const char *OsMisc_julong_format_specifier() {
#if defined(_WIN32_WCE)
  return "%I64u";
#endif
  /*
   * Return julong-specifier prefixes are used with type characters in
   * printf functions or wprintf functions to specify interpretation
   * of julong e.g. for win32 is "%I64u", for linux is "%llu"
   */
  return "%llu";
}
#endif // PRODUCT

void OsMisc_hardware_power_reset() {
	javacall_os_hardware_reset();
}

int OsMisc_set_system_time(jlong time) {
  return javacall_os_set_system_time(time);
}

#ifdef __cplusplus
}
#endif
