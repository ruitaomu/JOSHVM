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

/**
 * @file
 *
 * win32 implemenation for  FileConnection API
 */

#ifdef __cplusplus
extern "C" {
#endif

/*
 *
 *             JSR075's FileConnection API
 *            =============================
 *
 *
 * The following API definitions are required by JSR075.
 * These APIs are not required by standard JTWI implementations.
 * These APIs are file related only. Additional APIs required by JSR075
 * which are directory related can be found in javacall_dir.h
 *
 * These extensions include:
 * - javacall_fileconnection_init()
 * - javacall_fileconnection_finalize()
 * - javacall_fileconnection_is_hidden()
 * - javacall_fileconnection_is_readable()
 * - javacall_fileconnection_is_writable()
 * - javacall_fileconnection_set_hidden()
 * - javacall_fileconnection_set_readable()
 * - javacall_fileconnection_set_writable()
 * - javacall_fileconnection_get_last_modified()
 * - javacall_fileconnection_get_illegal_filename_chars()
 * - javacall_fileconnection_is_directory()
 * - javacall_fileconnection_create_dir()
 * - javacall_fileconnection_delete_dir()
 * - javacall_fileconnection_dir_exists()
 * - javacall_fileconnection_rename_dir()
 * - javacall_fileconnection_get_free_size()
 * - javacall_fileconnection_get_total_size()
 * - javacall_fileconnection_get_mounted_roots()
 * - javacall_fileconnection_get_photos_dir()
 * - javacall_fileconnection_get_videos_dir()
 * - javacall_fileconnection_get_graphics_dir()
 * - javacall_fileconnection_get_tones_dir()
 * - javacall_fileconnection_get_music_dir()
 * - javacall_fileconnection_get_recordings_dir()
 * - javacall_fileconnection_get_private_dir()
 * - javacall_fileconnection_get_localized_mounted_roots()
 * - javacall_fileconnection_get_localized_photos_dir()
 * - javacall_fileconnection_get_localized_videos_dir()
 * - javacall_fileconnection_get_localized_graphics_dir()
 * - javacall_fileconnection_get_localized_tones_dir()
 * - javacall_fileconnection_get_localized_music_dir()
 * - javacall_fileconnection_get_localized_recordings_dir()
 * - javacall_fileconnection_get_localized_private_dir()
 * - javacall_fileconnection_get_path_for_root()
 * - javacall_fileconnection_dir_content_size()
 * - javanotify_fileconnection_root_changed()
 */
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include "javacall_time.h"
#include "javacall_logging.h"
#include "javacall_dir.h"
#include "javacall_file.h"
#include "javautil_unicode.h"
#include "javacall_fileconnection.h"
#include "javacall_properties.h"


extern char* javacall_UNICODEsToUtf8(const javacall_utf16* fileName, int fileNameLen);

static const char* photos_dir     = "photo/";
static const char* videos_dir     = "video/";
static const char* graphics_dir   = "graphics/";
static const char* tones_dir      = "tones/";
static const char* music_dir      = "music/";
static const char* recordings_dir = "recordings/";
static const char* private_dir    = "private/";
static int photos_dir_len;
static int videos_dir_len;
static int graphics_dir_len;
static int tones_dir_len;
static int music_dir_len;
static int recordings_dir_len;
static int private_dir_len;

static const char* localized_photos_dir     = "photo/";
static const char* localized_videos_dir     = "video/";
static const char* localized_graphics_dir   = "graphics/";
static const char* localized_tones_dir      = "tones/";
static const char* localized_music_dir      = "music/";
static const char* localized_recordings_dir = "recordings/";
static const char* localized_private_dir    = "private/";
static int localized_photos_dir_len;
static int localized_videos_dir_len;
static int localized_graphics_dir_len;
static int localized_tones_dir_len;
static int localized_music_dir_len;
static int localized_recordings_dir_len;
static int localized_private_dir_len;

static int _fc_not_initialized = 0;

static void create_predefined_dirs() {
    char separator = '/';
    char* predefined_dirs[] = {JAVACALL_JAMS_NATIVE_ROOT_PATH_UNSECURE,
            JAVACALL_JAMS_NATIVE_ROOT_PATH_SECURE,
            JC_NATIVE_USER_ROOT_PATH};
    javacall_utf16 dir_name[JAVACALL_MAX_FILE_NAME_LENGTH];
    int i, j;
    int len;
    int minlen;
    javacall_result result;

    minlen = strlen(JC_NATIVE_ROOT_PATH);
    for (i = 0; i < sizeof(predefined_dirs)/sizeof(char*); i++) {
        len = strlen(predefined_dirs[i]);
        for (j = 0; j < len; j++) {
            dir_name[j] = predefined_dirs[i][j];
            if (predefined_dirs[i][j] == separator) {
                if (j > minlen) {
                    result = javacall_fileconnection_dir_exists(dir_name, j + 1);
                    if (result == JAVACALL_FAIL) {
                        result = javacall_fileconnection_create_dir(dir_name, j + 1);
                    }
                    if (result == JAVACALL_FAIL) {
                        javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_FC,
                            "Error: create dir failed. %s (%d)\n", predefined_dirs[i], j);
                        break;
                    }
                }
            }
        }
    }
}

/**
 * Makes all the required initializations for JSR 75 FileConnection
 * @return <tt>JAVACALL_OK</tt> if operation completed successfully
 *         <tt>JAVACALL_FAIL</tt> if an error occured or feature is not supported
 */
javacall_result javacall_fileconnection_init(void) {
#ifdef DEBUG_JAVACALL_FILECONNECTION
    javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_FC, "javacall_fileconnection_init\n");
#endif

    photos_dir_len = strlen(photos_dir);
    videos_dir_len = strlen(videos_dir);
    graphics_dir_len = strlen(graphics_dir);
    tones_dir_len = strlen(tones_dir);
    music_dir_len = strlen(music_dir);
    recordings_dir_len = strlen(recordings_dir);
    private_dir_len = strlen(private_dir);

    localized_photos_dir_len = strlen(localized_photos_dir);
    localized_videos_dir_len = strlen(localized_videos_dir);
    localized_graphics_dir_len = strlen(localized_graphics_dir);
    localized_tones_dir_len = strlen(localized_tones_dir);
    localized_music_dir_len = strlen(localized_music_dir);
    localized_recordings_dir_len = strlen(localized_recordings_dir);
    localized_private_dir_len = strlen(localized_private_dir);

    _fc_not_initialized = 1;

    create_predefined_dirs();
    return JAVACALL_OK;
}

inline static ensureInitialized() {
    if (!_fc_not_initialized) {
        javacall_fileconnection_init();
    }
}

/**
 * Cleans up resources used by fileconnection
 * @return JAVACALL_OK on success, JAVACALL_FAIL otherwise
 */
javacall_result javacall_fileconnection_finalize(void) {
    return JAVACALL_OK;
}

/**
 * Returns the HIDDEN attribute for the specified file or directory
 * If hidden files are not supported, the function should 
 * return JAVACALL_FALSE
 *
 * @param fileName      name in UNICODE of file
 * @param fileNameLen   length of file name
 * @param result        returned value: JAVACALL_TRUE if file is hidden
 *                      JAVACALL_FALSE file is not hidden or 
 *                      feature is not supported
 * @return <tt>JAVACALL_OK</tt> if operation completed successfully
 *         <tt>JAVACALL_FAIL</tt> if an error occured
 */
javacall_result javacall_fileconnection_is_hidden(const javacall_utf16* fileName,
                                                  int fileNameLen,
                                                  javacall_bool* /* OUT */ result) {
    return JAVACALL_FALSE;
}

/**
 * Returns the READABLE attribute for the specified file or directory
 *
 * @param pathName      name in UNICODE of file or directory
 * @param pathNameLen   length of path name
 * @param result        returned value: JAVACALL_TRUE if file/dir is readable
 *                      JAVACALL_FALSE file/dir is not readable
 * @return <tt>JAVACALL_OK</tt> if operation completed successfully
 *         <tt>JAVACALL_FAIL</tt> if an error occured
 */ 
javacall_result javacall_fileconnection_is_readable(const javacall_utf16* pathName,
                                                    int pathNameLen,
                                                    javacall_bool* /* OUT */ result) {

    char* szOsFilename=javacall_UNICODEsToUtf8(pathName, pathNameLen);
    int attrs;
    struct stat st;

    if(!szOsFilename) {
        javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_FC,  "Error: javacall_fileconnection_is_readable(), path name is too long\n");
        return JAVACALL_FAIL;
    }

    attrs = stat(szOsFilename, &st);
    if (attrs) {
        javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_FC,  "Error: javacall_fileconnection_is_readable(), path not found\n");
        return JAVACALL_FAIL;
    }

    *result = ((st.st_mode & (S_IRUSR|S_IRGRP|S_IROTH)) == 0) ? JAVACALL_FALSE : JAVACALL_TRUE;
    return JAVACALL_OK;
}

/**
 * Returns the WRITABLE attribute for the specified file or directory
 *
 * @param pathName      name in UNICODE of file or directory
 * @param pathNameLen   length of path name
 * @param result        returned value: JAVACALL_TRUE if file/dir is writable
 *                      JAVACALL_FALSE file/dir is not writable
 * @return <tt>JAVACALL_OK</tt> if operation completed successfully
 *         <tt>JAVACALL_FAIL</tt> if an error occured
 */ 
javacall_result javacall_fileconnection_is_writable(const javacall_utf16* pathName,
                                                    int pathNameLen,
                                                    javacall_bool* /* OUT */ result) {

    char* szOsFilename=javacall_UNICODEsToUtf8(pathName, pathNameLen);
    int attrs;
    struct stat st;

    if(!szOsFilename) {
        javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_FC,  "Error: javacall_fileconnection_is_writable(), path name is too long\n");
        return JAVACALL_FAIL;
    }

    attrs = stat(szOsFilename, &st);
    if (attrs) {
        javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_FC,  "Error: javacall_fileconnection_is_writable(), path not found\n");
        return JAVACALL_FAIL;
    }

    *result = ((st.st_mode & (S_IWUSR|S_IWGRP|S_IWOTH)) == 0) ? JAVACALL_FALSE : JAVACALL_TRUE;
    return JAVACALL_OK;
}

/**
 * Sets the HIDDEN attribute for the specified file or directory
 *
 * @param fileName      name in UNICODE of file
 * @param fileNameLen   length of file name
 * @param value         JAVACALL_TRUE to set file as hidden
 *                      JAVACALL_FALSE to set file as not hidden
 * @return <tt>JAVACALL_OK</tt> if operation completed successfully
 *         <tt>JAVACALL_FAIL</tt> if an error occured
 */
javacall_result
javacall_fileconnection_set_hidden(javacall_const_utf16_string fileName, javacall_bool value) {
    return JAVACALL_OK;
}

/**
 * Sets the READABLE attribute for the specified file or directory.
 *
 * @param pathName      name of file or directory.
 * @param value         <tt>JAVACALL_TRUE</tt> to set file as readable,
 *                      <tt>JAVACALL_FALSE</tt> to set file as not readable.
 * @return <tt>JAVACALL_OK</tt> if operation completed successfully,
 *         <tt>JAVACALL_FAIL</tt> if an error occured.
 */ 
javacall_result
javacall_fileconnection_set_readable(javacall_const_utf16_string pathName, javacall_bool value) {
	javacall_int32 pathNameLen;
	
	if (JAVACALL_OK != javautil_unicode_utf16_ulength(pathName, &pathNameLen)) {
		return JAVACALL_FAIL;
	}
	
    char* szOsFilename=javacall_UNICODEsToUtf8(pathName, pathNameLen);
	
    int attrs;
    struct stat st;

    if(!szOsFilename) {
        javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_FC,  "Error: javacall_fileconnection_set_readable(), path name is too long\n");
        return JAVACALL_FAIL;
    }

    if(access(szOsFilename, 0) != 0) {
        javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_FC,  "Error: javacall_fileconnection_set_readable(), file is not accessible\n");
        return JAVACALL_FAIL;
    }

    return JAVACALL_OK; // files are always readable, the call is ignored
}

/**
 * Sets the WRITABLE attribute for the specified file or directory.
 *
 * @param pathName      name of file or directory.
 * @param value         <tt>JAVACALL_TRUE</tt> to set file as writable,
 *                      <tt>JAVACALL_FALSE</tt> to set file as not writable.
 * @return <tt>JAVACALL_OK</tt> if operation completed successfully,
 *         <tt>JAVACALL_FAIL</tt> if an error occured.
 */ 
javacall_result 
javacall_fileconnection_set_writable(javacall_const_utf16_string pathName, javacall_bool value) {
	javacall_int32 pathNameLen;
		
	if (JAVACALL_OK != javautil_unicode_utf16_ulength(pathName, &pathNameLen)) {
		return JAVACALL_FAIL;
	}

    char* szOsFilename=javacall_UNICODEsToUtf8(pathName, pathNameLen);
    int attrs;
    struct stat st;

    if(!szOsFilename) {
        javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_FC,  "Error: javacall_fileconnection_set_writable(), path name is too long\n");
        return JAVACALL_FAIL;
    }

    if(access(szOsFilename, W_OK) != 0) {
        javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_FC,  "Error: javacall_fileconnection_set_writable(), file is not accessible\n");
        return JAVACALL_FAIL;
    }

    return JAVACALL_OK; // files are always writeable, the call is ignored
}


/**
 * Returns the time when the file or directory was last modified.
 *
 * @param fileName      name of file or directory.
 * @param result        A javacall_int64 value representing the time the file was 
 *                      last modified, measured in seconds since the epoch (00:00:00 GMT, 
 *                      January 1, 1970).
 * @return <tt>JAVACALL_OK</tt> on success,
 *         <tt>JAVACALL_FAIL</tt> otherwise.
 */
javacall_result 
javacall_fileconnection_get_last_modified(javacall_const_utf16_string fileName, 
                                          javacall_int64* /* OUT */ result) {
	javacall_int32 fileNameLen;
		
	if (JAVACALL_OK != javautil_unicode_utf16_ulength(fileName, &fileNameLen)) {
		return JAVACALL_FAIL;
	}

    char* szOsFilename=javacall_UNICODEsToUtf8(fileName, fileNameLen);
    int attrs;
    struct stat st;

    if(!szOsFilename) {
        javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_FC,  "Error: javacall_fileconnection_get_last_modified(), path name is too long\n");
        return JAVACALL_FAIL;
    }

    attrs = stat(szOsFilename, &st);
    if (attrs) {
        javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_FC,  "Error: javacall_fileconnection_get_last_modified(), path not found\n");
        return JAVACALL_FAIL;
    }

    *result = (javacall_int64)(st.st_mtime);
    return JAVACALL_OK;
}

/**
 * Returns the list of illegal characters in file names. The list must not
 * include '/', but must include native file separator, if it is different
 * from '/' character
 * @param illegalChars returned value: pointer to UNICODE string, allocated
 *                     by the VM, to be filled with the characters that are
 *                     not allowed inside file names.
 * @param illegalCharsLenMaxLen available size, in javacall_utf16 symbols,
 *                              of the buffer provided
 * @return <tt>JAVACALL_OK</tt> if operation completed successfully
 *         <tt>JAVACALL_FAIL</tt> otherwise
 */
javacall_result javacall_fileconnection_get_illegal_filename_chars(javacall_utf16* /* OUT */ illegalChars,
                                                                   int illegalCharsMaxLen) {
    int i;
    char str[] = "/:\"\\|";

    if(illegalCharsMaxLen < sizeof(str)) {
        javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_FC,  "Error: javacall_fileconnection_get_illegal_filename_chars(), insufficient buffer size\n");
        return JAVACALL_FAIL;
    }

    for(i = 0; i < sizeof(str); i++) { // all chars, including trailing zero
        illegalChars[i] = (unsigned short) str[i];
    }

    return JAVACALL_OK;
}


/**
 * Checks if the path exists in the file system storage and if 
 * it is a directory.
 * @param pathName name of file or directory in unicode format
 * @param pathNameLen length of pathName
 * @param result returned value: JAVACALL_TRUE if path is a directory, 
 *                               JAVACALL_FALSE otherwise
 * @return <tt>JAVACALL_OK</tt> if operation completed successfully
 *         <tt>JAVACALL_FAIL</tt> if an error occured
 */
javacall_result javacall_fileconnection_is_directory(const javacall_utf16* pathName,
                                                     int pathNameLen,
                                                     javacall_bool* /* OUT */ result) {
    char* szOsFilename=javacall_UNICODEsToUtf8(pathName, pathNameLen);
    int attrs;
    struct stat st;

#ifdef DEBUG_JAVACALL_FILECONNECTION
    javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_FC, "javacall_fileconnection_is_directory: %s\n", szOsFilename);
#endif

    if(!szOsFilename) {
        javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_FC,  "Error: javacall_fileconnection_is_directory(), path name is too long\n");
        return JAVACALL_FAIL;
    }

    attrs = stat(szOsFilename, &st);
    if (attrs) {
#ifdef DEBUG_JAVACALL_FILECONNECTION
        javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_FC, "javacall_fileconnection_is_directory(), path not found: %s\n", szOsFilename);
#endif
        return JAVACALL_FAIL;
    }

    *result = ((st.st_mode & S_IFDIR) == 0) ? JAVACALL_FALSE : JAVACALL_TRUE;
    return JAVACALL_OK;
}

/**
 * Create a directory   
 * @param dirName path name in UNICODE of directory
 * @param dirNameLen length of directory name
 * @return <tt>JAVACALL_OK</tt> success
 *         <tt>JAVACALL_FAIL</tt> fail
 */
javacall_result javacall_fileconnection_create_dir(const javacall_utf16* dirName,
                                                   int dirNameLen) {

    char* szOsFilename=javacall_UNICODEsToUtf8(dirName, dirNameLen);

    if(!szOsFilename) {
        javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_FC,  "Error: javacall_fileconnection_create_dir(), path name is too long\n");
        return JAVACALL_FAIL;
    }

    if(0 != mkdir(szOsFilename, 0777)) {
        javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_FC,  "Error: javacall_fileconnection_create_dir(), cannot create directory\n");
        return JAVACALL_FAIL;
    }
    return JAVACALL_OK;
}

/**
 * Deletes an empty directory from the persistent storage.
 *   If directory not empty this function must fail.
 * @param dirName path name in UNICODE of directory
 * @param dirNameLen length of directory name
 * @return <tt>JAVACALL_OK</tt> success
 *         <tt>JAVACALL_FAIL</tt> fail
 */
javacall_result javacall_fileconnection_delete_dir(const javacall_utf16* dirName,
                                                   int dirNameLen) {

    char* szOsFilename=javacall_UNICODEsToUtf8(dirName, dirNameLen);

    if(!szOsFilename) {
        javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_FC,  "Error: javacall_fileconnection_delete_dir(), path name is too long\n");
        return JAVACALL_FAIL;
    }

    if(0 != rmdir(szOsFilename)) {
        javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_FC,  "Error: javacall_fileconnection_delete_dir(), cannot delete directory\n");
        return JAVACALL_FAIL;
    }
    return JAVACALL_OK;
}

/**
 * Check if the directory exists in file system storage.
 * @param pathName name of directory in unicode format
 * @param pathNameLen length of directory name
 * @return <tt>JAVACALL_OK </tt> if it exists and it is a regular directory, 
 *         <tt>JAVACALL_FAIL</tt> if directory not exists or error
 */
javacall_result javacall_fileconnection_dir_exists(const javacall_utf16* pathName,
                                                   int pathNameLen) {

    javacall_bool res;
#ifdef DEBUG_JAVACALL_FILECONNECTION
    javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_FC,  "javacall_fileconnection_dir_exists\n");
#endif
    
    if(JAVACALL_OK != javacall_fileconnection_is_directory(pathName, pathNameLen, &res)) {
        //javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_FC, "javacall_fileconnection_dir_exists(): not exist directory\n");
        return JAVACALL_FAIL;
    }

    if(JAVACALL_FALSE == res) {
        return JAVACALL_FAIL;
    }
    return JAVACALL_OK;
}

/**
 * Renames the specified directory
 * @param oldDirName current name of file
 * @param oldDirNameLen current name length
 * @param newDirName new name of file
 * @param newDirNameLen length of new name
 * @return <tt>JAVACALL_OK</tt> on success, 
 *         <tt>JAVACALL_FAIL</tt> otherwise
 */
javacall_result javacall_fileconnection_rename_dir(const javacall_utf16* oldDirName,
                                                   int oldDirNameLen,
                                                   const javacall_utf16* newDirName,
                                                   int newDirNameLen) {

    if(JAVACALL_OK != javacall_file_rename(oldDirName, oldDirNameLen, newDirName, newDirNameLen)) {
        javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_FC,  "Error: javacall_fileconnection_rename_dir(), cannot rename directory\n");
        return JAVACALL_FAIL;
    }
    return JAVACALL_OK;
}

/**
 * Determines the free memory in bytes that is available on the 
 *      file system the file or directory resides on
 * @param pathName path name in UNICODE of any file within the file system
 * @param pathNameLen length of path
 * @param result returned value: on success, size of available storage space (bytes)
 * @return <tt>JAVACALL_OK</tt> if operation completed successfully,
 *         <tt>JAVACALL_FAIL</tt> otherwise.
 */
javacall_result javacall_fileconnection_get_free_size(const javacall_utf16* pathName,
                                                      int pathNameLen,
                                                      javacall_int64* /* OUT */ result) {

    javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_FC,  "Error: javacall_fileconnection_get_free_size(), stub out!\n");

    *result = (javacall_int64)10LL*1024LL*1024LL; //Dummy, always return 10M
    return JAVACALL_OK;
}

/**
 * Determines the total size in bytes of the file system the file 
 * or directory resides on
 * @param pathName file name in UNICODE of any file within the file system
 * @param pathNameLen length of path name
 * @param result returned value: on success, total size of storage space (bytes)
 * @return <tt>JAVACALL_OK</tt> if operation completed successfully,
 *         <tt>JAVACALL_FAIL</tt> otherwise.
 */
javacall_result javacall_fileconnection_get_total_size(const javacall_utf16* pathName,
                                                       int pathNameLen,
                                                       javacall_int64* /* OUT */ result) {
    javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_FC,  "Error: javacall_fileconnection_get_total_size(), stub out!\n");

    *result = (javacall_int64)1000LL*1024LL*1024LL; //Dummy, always return 1G
    return JAVACALL_OK;
}

/** 
 * Returns the mounted root file systems (UNICODE format). Each root must end
 * with '/' character
 * @param roots buffer to store the UNICODE string containing 
 *              currently mounted roots separated by '\n' character
 * @param rootsLen available buffer size (maximum number of javacall_utf16
 *                 symbols to be stored)
 * @return <tt>JAVACALL_OK</tt> on success,
 *         <tt>JAVACALL_FAIL</tt> otherwise
 */
javacall_result javacall_fileconnection_get_mounted_roots(javacall_utf16* /* OUT */ roots,
                                                          int rootsLen) {
    static char* root = "internal/\nroot/\nprivate/\nPhone/";
    int i;
    int len = strlen(root);
   
    if (rootsLen <= len) {
        javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_FC,  "Error: javacall_fileconnection_get_mounted_roots(), buffer is too small\n");
        return JAVACALL_FAIL;
    }

    for (i = 0; i <= len; i++) {
    	roots[i] = (javacall_utf16)root[i];
    }
    
    return JAVACALL_OK;
}

/** 
 * Returns the path to directory, that is used to store private directories
 * for all applications (accessed via "fileconn.dir.private" system property).
 * The returned path must use '/' as file separator and have this separator at
 * the end.
 *
 * @param dir buffer to store the string containing path to
 *            location of private directories for all applications.
 * @param dirLen available buffer size (maximum number of
 *               characters to be stored).
 * @param fromCache indicates whether the returned value should be taken from
 *                  internal cache (this parameter can be ignored if properties
 *                  caching is not supported by underlying implementation).
 * @return <tt>JAVACALL_OK</tt> on success,
 *         <tt>JAVACALL_FAIL</tt> otherwise.
 */
javacall_result
javacall_fileconnection_get_private_dir(javacall_utf16_string /* OUT */ dir,
                                        int dirLen, javacall_bool fromCache){
    int i;

    ensureInitialized();
    
    if (dirLen < private_dir_len) {
        javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_FC,  "Error: javacall_fileconnection_get_private_dir(), buffer is too small\n");
        return JAVACALL_FAIL;
    }

    for (i = 0; i <= private_dir_len; i++) {
    	dir[i] = (javacall_utf16)private_dir[i];
    }

    return JAVACALL_OK;
}

/**
 * Returns OS-specific path for the specified file system root.
 *
 * @param rootName root name.
 * @param pathName buffer to store the string containing 
 *                 the system-dependent path to access the specified 
 *                 root.
 * @param pathNameLen available buffer size (maximum number of
 *                    characters to be stored).
 * @return <tt>JAVACALL_OK</tt> on success,
 *         <tt>JAVACALL_FAIL</tt> otherwise.
 */ 
javacall_result 
javacall_fileconnection_get_path_for_root(javacall_const_utf16_string rootName,
                                          javacall_utf16_string /* OUT */ pathName,
                                          int pathNameLen) {
	javacall_int32 rootNameLen;
	
	if (JAVACALL_OK != javautil_unicode_utf16_ulength(rootName, &rootNameLen)) {
		return JAVACALL_FAIL;
	}

    static char* realroot_def[] = {JC_NATIVE_INTERNAL_ROOT_PATH,
        JC_NATIVE_USER_ROOT_PATH, JC_NATIVE_USER_ROOT_PATH,
        JC_NATIVE_USER_ROOT_PATH};
    static char* root[] = {"internal/", "root/", "private/", "Phone/"};
    static char* propname[] = {"internal_dir", "root_dir", "private_dir", "app_dir"};
    int i, r;
    int len;
    int ne;
    char* realroot[sizeof(root)/sizeof(char*)] = {0};
    
#ifdef DEBUG_JAVACALL_FILECONNECTION
    javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_FC, "javacall_fileconnection_get_path_for_root:%s\n", javacall_UNICODEsToUtf8(rootName, rootNameLen));
#endif

    for (r = 0; r < sizeof(root)/sizeof(char*); r++) {
    	 if (realroot[r] == 0) {
    	 	if (JAVACALL_OK != javacall_get_property(propname[r], JAVACALL_JSR75_PROPERTY, &realroot[r]) ||
    	 	     realroot[r] == NULL) {
    	 		realroot[r] = realroot_def[r];
    	 	}
    	 }
    	 
    	 len = strlen(root[r]);
        if (rootNameLen != len) {
        	continue;
        }
    
        for (i = 0, ne = 0; i <= len; i++) {
        	if ((javacall_utf16)root[r][i] != rootName[i]) {
        		ne = 1;
        		break;
        	}
        }

        if (ne) {
            continue;
        }
        
        len = strlen(realroot[r]);
       
        if (pathNameLen <= len) {
            javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_FC,  "javacall_fileconnection_get_path_for_root failed: buffer not enough\n");
            return JAVACALL_FAIL;
        }
    
        for (i = 0; i <= len; i++) {
        	pathName[i] = (javacall_utf16)realroot[r][i];
        }
        
        return JAVACALL_OK;
    }
    
#ifdef DEBUG_JAVACALL_FILECONNECTION
    javacall_logging_printf(JAVACALL_LOGGING_WARNING, JC_FC,  "javacall_fileconnection_get_path_for_root failed: no root matched\n");
#endif

    return JAVACALL_FAIL;
}


#define MAX_DIRECTORY_NESTING_LEVEL 50

/**
 * Get size in bytes of all files and possibly subdirectories contained 
 * in the specified dir.
 *
 * @param pathName          path name of directory.
 * @param includeSubdirs    if <tt>JAVACALL_TRUE</tt>, include subdirectories size too;
 *                          if <tt>JAVACALL_FALSE</tt>, do not include subdirectories.
 * @param result            returned value: size in bytes of all files contained in 
 *                          the specified directory and possibly its subdirectories.
 * @return <tt>JAVACALL_OK</tt> on success,
 *         <tt>JAVACALL_FAIL</tt> otherwise.
 */ 
javacall_result 
javacall_fileconnection_dir_content_size(javacall_const_utf16_string pathName,
                                         javacall_bool includeSubdirs,
                                         javacall_int64* /* OUT */ result) {
    javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_FC,  "Error: javacall_fileconnection_dir_content_size(), not implemented!\n");
    return JAVACALL_FAIL;
}

#ifdef __cplusplus
}
#endif

