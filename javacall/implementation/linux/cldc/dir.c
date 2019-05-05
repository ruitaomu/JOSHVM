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

#ifdef __cplusplus
extern "C" {
#endif
    
#include "javacall_dir.h"
#include "stdlib.h"
#include "string.h"
#include "dirent.h"

//#define DEBUG_JAVACALL_DIR 1

extern char* javacall_UNICODEsToUtf8(const javacall_utf16* fileName, int fileNameLen);

/**
 * returns a handle to a file list. This handle can be used in
 * subsequent calls to javacall_dir_get_next() to iterate through
 * the file list and get the names of files that match the given string.
 * 
 * @param path the name of a directory, but it can be a
 *             partial file name
 * @param pathLen length of directory name
 * @return pointer to an opaque filelist structure, that can be used in
 *         javacall_dir_get_next() and javacall_dir_close
 *         NULL returned on error, for example if root directory of the
 *         input 'string' cannot be found.
 */
javacall_handle javacall_dir_open(const javacall_utf16* path, int pathLen) {
#ifdef DEBUG_JAVACALL_DIR
    javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_FILE, "javacall_dir_open:\n");
#endif
    char* szPath = javacall_UNICODEsToUtf8(path, pathLen);
    if (!szPath) {
        return NULL;
    }

#ifdef DEBUG_JAVACALL_DIR    
   javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_FILE, szPath);
   javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_FILE, "\n");
#endif    

    DIR* handle = opendir(szPath);

#ifdef DEBUG_JAVACALL_DIR
    javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_FILE, "return %x\n", handle);
#endif
    return (javacall_handle)handle;
}
    
/**
 * closes the specified file list. This handle will no longer be
 * associated with this file list.
 * @param handle pointer to opaque filelist struct returned by
 *               javacall_dir_open 
 */
void javacall_dir_close(javacall_handle handle) {
    closedir((DIR*)handle);
}
    
/**
 * return the next filename in directory path (UNICODE format)
 * The order is defined by the underlying file system.
 * 
 * On success, the resulting file will be copied to user supplied buffer.
 * The filename returned will omit the file's path
 * 
 * @param handle pointer to filelist struct returned by javacall_dir_open
 * @param outFilenameLength will be filled with number of chars written 
 * 
 * 
 * @return pointer to UNICODE string for next file on success, 0 otherwise
 * returned param is a pointer to platform specific memory block
 * platform MUST BE responsible for allocating and freeing it.
 */
javacall_utf16* javacall_dir_get_next(javacall_handle handle,
        int* /*OUT*/ outFilenameLength, javacall_bool* /*OUT*/ isHidden,
        javacall_bool* /*OUT*/ isDirectory) {
    static javacall_utf16 name[JAVACALL_MAX_FILE_NAME_LENGTH+1];
    
    struct dirent* d = readdir((DIR*)handle);
    if (d != NULL) {
        int len = strlen(d->d_name);
#ifdef DEBUG_JAVACALL_DIR
        javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_FILE, 
        	"javacall_dir_get_next: %s, len=%d, dir=%d\n", d->d_name, len, (d->d_stat.st_attr & FIO_SO_IFDIR)?1:0);
#endif
    	if (len >= JAVACALL_MAX_FILE_NAME_LENGTH - 1) {
    	    return NULL;
    	} else {
    	    if(d->d_type == DT_DIR) {
    	         name[len] = (javacall_utf16)'/';
    	    	  *outFilenameLength = len+1;
    	    	  name[len+1] = 0;
				  *isDirectory = JAVACALL_TRUE;
    	    } else {
    	         *outFilenameLength = len;
                name[len] = 0;
				*isDirectory = JAVACALL_FALSE;
    	    }

			*isHidden = JAVACALL_FALSE; //Curretly every files are NOT HIDDEN
			
           while (len--) {
               name[len] = (javacall_utf16)d->d_name[len] & 0xff;
           }
    	}
    	return name;
    } else {
       return NULL;
    }
}
    
/**
 * Checks the size of free space in storage. 
 * @return size of free space
 */
javacall_int64 javacall_dir_get_free_space_for_java(void){
    return 0;
}
    
    
/**
 * Returns the root path of java's home directory.
 * 
 * @param rootPath returned value: pointer to unicode buffer, allocated 
 *        by the VM, to be filled with the root path.
 * @param rootPathLen IN  : lenght of max rootPath buffer
 *                    OUT : lenght of set rootPath
 * @return <tt>JAVACALL_OK</tt> if operation completed successfully
 *         <tt>JAVACALL_FAIL</tt> if an error occured
 */
javacall_result javacall_dir_get_root_path(javacall_utf16* /* OUT */ rootPath,
                                           int* /* IN | OUT */ rootPathLen) {
    static char* root = ".";
    int i;
    int len = strlen(root);
    if (*rootPathLen < len) {
    	return JAVACALL_FAIL;
    }
    
    for (i = 0; i < len; i++) {
    	rootPath[i] = (javacall_utf16)root[i];
    }
    *rootPathLen = i;
    
    return JAVACALL_OK;
}  

/**
 *  Returns file separator character used by the underlying file system
 * (usually this function will return '\\';)
 * @return 16-bit Unicode encoded file separator
 */
javacall_utf16 javacall_get_file_separator(void) {
    return '/';
}
    
    
/**
 * Check if the given path is located on secure storage
 * The function should return JAVACALL_TRUE only in the given path
 * is located on non-removable storage, and cannot be accessed by the 
 * user or overwritten by unsecure applications.
 * @return <tt>JAVACALL_TRUE</tt> if the given path is guaranteed to be on 
 *         secure storage
 *         <tt>JAVACALL_FALSE</tt> otherwise
 */
javacall_bool javacall_dir_is_secure_storage(javacall_utf16* classPath, int pathLen) {
    return JAVACALL_FALSE;
}

#ifdef __cplusplus
}
#endif


