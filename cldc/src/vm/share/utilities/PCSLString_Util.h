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

#ifndef _PCSLSTRING_UTIL_H_
#define _PCSLSTRING_UTIL_H_

#if ENABLE_PCSL
#include <pcsl_string.h>

#ifdef __cplusplus
extern "C" {
#endif



/**
 * Given variable <code>id</code> of type pcsl_string*,
 * declare: <ul>
 * <li>id_data of type jchar const * const pointing to the string UTF-16 data, </li>
 * <li>id_len of type jint containing the string length.</li>  </ul>
 *
 * The scope of the above two names is the
 *  GET_PCSL_STRING_DATA_AND_LENGTH...RELEASE_PCSL_STRING_DATA_AND_LENGTH
 * construct.
 *
 * @param id the variable name, from which new names are derived.
 */
#define GET_PCSL_STRING_DATA_AND_LENGTH(id) \
    { \
        const jint id##_len = pcsl_string_utf16_length(id); \
        const jchar * const id##_data = pcsl_string_get_utf16_data(id); \
        const jchar * const * const last_pcsl_string_data = & id##_data; \
        const pcsl_string* const last_pcsl_string_itself = id; \
        {

/**
 * closes the
 *  GET_PCSL_STRING_DATA_AND_LENGTH...RELEASE_PCSL_STRING_DATA_AND_LENGTH
 * construct.
 */
#define RELEASE_PCSL_STRING_DATA_AND_LENGTH \
        } pcsl_string_release_utf16_data(*last_pcsl_string_data, last_pcsl_string_itself); \
    }


/**
 * Create pcsl_string from the specified Java platform String object.
 * The caller is responsible for freeing the created pcsl_string when done.
 *
 * Use pcsl_string_free to free the created object.
 *
 * @param java_str pointer to the Java platform String instance
 * @param pcsl_str address of variable to receive the pcsl_string instance
 * @return status of the operation
 */
pcsl_string_status midp_jstring_to_pcsl_string(jstring java_str,
					       pcsl_string * pcsl_str);

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
                                pcsl_string * pcsl_str);

/**
 * Create Java platform String object from the specified pcsl_string.
 *
 * @param pcsl_str pointer to the pcsl_string instance
 * @param java_str pointer to the Java platform String instance
 * @return status of the operation
 */
pcsl_string_status midp_jstring_from_pcsl_string(const pcsl_string * pcsl_str,
						 jstring java_str);

/**
 * Convert a C string to a pcsl_string string.
 *
 * @param in C string specifying the text to be copied to the out parameter
 * @param out pcsl_string to receive a copy of the text specified by the in parameter
 *
 * @return jchar string
 */
pcsl_string_status pcsl_string_from_chars(const char* in, pcsl_string* out);

#ifdef __cplusplus
}
#endif
#endif /* ENABLE_PCSL */
#endif /* _PCSLSTRING_UTIL_H_ */
