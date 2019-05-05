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

/**
 * @file
 *
 * This file is for utility function that depend on KNI VM functions
 * so that other files do not have to be dependent on the VM.
 */
#include "incls/_precompiled.incl"
#include "incls/_PCSLString_Util.cpp.incl"

#if ENABLE_PCSL
#include <pcsl_string.h>
#include <pcsl_memory.h>

/**
 * Create pcsl_string from the specified Java String object.
 * The caller is responsible for freeing the created pcsl_string when done.
 *
 * @param java_str pointer to the Java String instance
 * @param pcsl_str pointer to the pcsl_string instance
 * @return status of the operation
 */
pcsl_string_status midp_jstring_to_pcsl_string(jstring java_str,
					       pcsl_string * pcsl_str) {
  if (pcsl_str == NULL) {
    return PCSL_STRING_EINVAL;
  }

  if (KNI_IsNullHandle(java_str)) {
    * pcsl_str = PCSL_STRING_NULL;
    return PCSL_STRING_OK;
  } else {
    const jsize length  = KNI_GetStringLength(java_str);

    if (length < 0) {
      * pcsl_str = PCSL_STRING_NULL;
      return PCSL_STRING_ERR;
    } else if (length == 0) {
      * pcsl_str = PCSL_STRING_EMPTY;
      return PCSL_STRING_OK;
    } else {
      jchar * buffer = (jchar*)pcsl_mem_malloc(length * sizeof(jchar));

      if (buffer == NULL) {
	* pcsl_str = PCSL_STRING_NULL;
	return PCSL_STRING_ENOMEM;
      }

      KNI_GetStringRegion(java_str, 0, length, buffer);

      {
	pcsl_string_status status =
	  pcsl_string_convert_from_utf16(buffer, length, pcsl_str);

	pcsl_mem_free(buffer);

	return status;
      }
    }
  }
}

/**
 * Create pcsl_string from the specified KNI CharArray object.
 * The caller is responsible for freeing the created pcsl_string when done.
 *
 * @param java_arr pointer to the KNI CharArray instance
 * @param length length of the text in the CharArray
 * @param pcsl_str pointer to the pcsl_string instance
 * @return status of the operation
 */
pcsl_string_status
midp_jchar_array_to_pcsl_string(jcharArray java_arr, jint length,
                                pcsl_string * pcsl_str) {
    if (pcsl_str == NULL) {
        return PCSL_STRING_EINVAL;
    }

    if (KNI_IsNullHandle(java_arr)) {
        *pcsl_str = PCSL_STRING_NULL;
        return PCSL_STRING_OK;
    } else if (length < 0) {
        *pcsl_str = PCSL_STRING_NULL;
        return PCSL_STRING_ERR;
    } else if (length == 0) {
        *pcsl_str = PCSL_STRING_EMPTY;
        return PCSL_STRING_OK;
    } else {
        jchar * buffer = (jchar*)pcsl_mem_malloc(length * sizeof(jchar));

        if (buffer == NULL) {
              * pcsl_str = PCSL_STRING_NULL;
            return PCSL_STRING_ENOMEM;
        }

          KNI_GetRawArrayRegion(java_arr, 0,
                                          length * sizeof(jchar),
                                          (jbyte *) buffer);

        {
            pcsl_string_status status =
                  pcsl_string_convert_from_utf16(buffer, length, pcsl_str);

              pcsl_mem_free(buffer);

            return status;
        }
    }
}

/**
 * Create Java String object from the specified pcsl_string.
 *
 * @param pcsl_str pointer to the pcsl_string instance
 * @param java_str pointer to the Java String instance
 * @return status of the operation
 */
pcsl_string_status midp_jstring_from_pcsl_string(KNIDECLARGS
						 const pcsl_string * pcsl_str,
						 jstring java_str) {
  if (pcsl_str == NULL) {
    KNI_ReleaseHandle(java_str);
    return PCSL_STRING_EINVAL;
  } else {
    const jsize length = pcsl_string_utf16_length(pcsl_str);

    if (length < 0) {
      KNI_ReleaseHandle(java_str);
      return PCSL_STRING_EINVAL;
    } else {
      const jchar * buffer = pcsl_string_get_utf16_data(pcsl_str);

      if (buffer == NULL) {
	KNI_ReleaseHandle(java_str);
	return PCSL_STRING_ERR;
      } else {
	KNI_NewString(buffer, length, java_str);
	return PCSL_STRING_OK;
      }
    }
  }
}

/**
 * Convert a C string to a pcsl_string string.
 *
 * @param in C string specifying the text to be copied to the out parameter
 * @param out pcsl_string to receive a copy of the text specified by the in parameter
 *
 * @return jchar string
 */
pcsl_string_status pcsl_string_from_chars(const char* in, pcsl_string* out) {
    return pcsl_string_convert_from_utf8((jbyte*)in, strlen(in), out);
}

#endif
