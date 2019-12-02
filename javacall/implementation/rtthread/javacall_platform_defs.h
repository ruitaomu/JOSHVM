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
#ifndef __JAVACALL_PLATFORM_DEFINE_H_
#define __JAVACALL_PLATFORM_DEFINE_H_

/**
 * @file javacall_platform_defs.h
 * @ingroup Common
 * @brief Platform-dependent definitions for javacall
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * @defgroup Platform Platfrom specific Common Javacall API Definitions
 * @ingroup Common
 * The platform specific common javacall definitions are defined in this file
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif 

/**
 * @typedef javacall_utf16
 * @brief general unicode string type
 */
typedef unsigned short javacall_utf16;

/**
 * @typedef javacall_uint8
 * @brief 8 bit unsigned interger type
 */
typedef unsigned char javacall_uint8;

/**
 * @typedef javacall_uint16
 * @brief 16 bit unsigned interger type
 */
typedef unsigned short javacall_uint16;

/**
 * @typedef javacall_uint32
 * @brief 32 bit unsigned interger type
 */
typedef unsigned long javacall_uint32;

/**
 * @typedef javacall_uint64
 * @brief 64 bit unsigned integer type
 */
typedef unsigned long long javacall_uint64;

/**
 * @typedef javacall_int16
 * @brief 16 bit interger type
 */
typedef signed short javacall_int16;

/**
 * @typedef javacall_int32
 * @brief 32 bit interger type
 */
typedef signed long javacall_int32;

/**
 * @typedef javacall_int64
 * @brief 64 bit interger type
 */
typedef long long javacall_int64;

/**
 * @typedef javacall_const_ascii_string
 * @brief general constant eight-bit ASCII string type,
 *        this type is constant null terminated string
 */
typedef const char* javacall_const_ascii_string;

/**
 * @def JAVACALL_MAX_EVENT_SIZE
 * Maximal length of event data
 */
#define JAVACALL_MAX_EVENT_SIZE        512

/**
 * @def JAVACALL_MAX_FILE_NAME_LENGTH
 * Maximal length of filename supported 
 */
#define JAVACALL_MAX_FILE_NAME_LENGTH         256

/**
 * @def JAVACALL_MAX_ILLEGAL_FILE_NAME_CHARS
 * Maximal number of illegal chars
 */
#define JAVACALL_MAX_ILLEGAL_FILE_NAME_CHARS  256

/**
 * @def JAVACALL_MAX_ROOTS_LIST_LENGTH
 * Maximal length of a list of file system roots
 */
#define JAVACALL_MAX_ROOTS_LIST_LENGTH  256

/**
 * @def JAVACALL_MAX_ROOT_PATH_LENGTH
 * Maximal length of a file system root path
 */
#define JAVACALL_MAX_ROOT_PATH_LENGTH   256

/**
 * @def JAVACALL_MAX_LOCALIZED_ROOTS_LIST_LENGTH
 * Maximal length of a list of localized names of file system roots
 */
#define JAVACALL_MAX_LOCALIZED_ROOTS_LIST_LENGTH  8192

/**
 * @def JAVACALL_MAX_LOCALIZED_DIR_NAME_LENGTH
 * Maximal length of a localized name of a special directory
 */
#define JAVACALL_MAX_LOCALIZED_DIR_NAME_LENGTH    512

/**
 * @}
 */

#define javautil_strlen strlen
#define javautil_strcpy strcpy
#define javautil_strncpy strncpy
#define javautil_strcat strcat
#define javautil_strcmp strcmp
#define javautil_memset memset
#define javautil_memcpy memcpy
#define javautil_sprintf                sprintf
#define javautil_snprintf snprintf

typedef enum {
	EVENT_FD_NULL = 0,
	EVENT_FD_READ = 1,
	EVENT_FD_WRITE = 2,
	EVENT_FD_OOB = 3,
	EVENT_FD_ACCEPT = 4,
	EVENT_FD_CONNECT = 5,
	EVENT_FD_CLOSE = 6,
	EVENT_FD_GETHOSTBYNAME = 7,
	EVENT_FD_NETWORKUP = 8
} javacall_event_type;

#define TICKS_TO_MILISECS(x) ((x)*(1000/RT_TICK_PER_SECOND))
#define MILISECS_TO_TICKS(x) ((x)/(1000/RT_TICK_PER_SECOND))

#define JC_NATIVE_ROOT_PATH "/"
#define JC_NATIVE_INTERNAL_ROOT_PATH JC_NATIVE_ROOT_PATH"joshvm/internal/"
#define JC_NATIVE_USER_ROOT_PATH JC_NATIVE_ROOT_PATH"joshvm/user/"

#define JAVACALL_JAMS_JAVA_ROOT_PATH_UNSECURE "/internal/appdb/unsecure/"
#define JAVACALL_JAMS_NATIVE_ROOT_PATH_UNSECURE JC_NATIVE_INTERNAL_ROOT_PATH"appdb/unsecure/"
#define JAVACALL_JAMS_JAVA_ROOT_PATH_SECURE "/internal/appdb/secure/"
#define JAVACALL_JAMS_NATIVE_ROOT_PATH_SECURE JC_NATIVE_INTERNAL_ROOT_PATH"appdb/secure/"
#define JAVACALL_JAMS_JAVA_ROOT_PATH "internal/"

#ifdef __cplusplus
}
#endif

#endif 


