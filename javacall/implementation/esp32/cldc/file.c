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

#include "javacall_file.h"
#include "javacall_memory.h"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_heap_caps.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "esp_vfs.h"

static const char *TAG = "JOSHVM SPIFFS";

#define PARTITION_LABEL_UNSECURE "unsecadb"
#define PARTITION_LABEL_SECURE "secadb"
#define PARTITION_LABEL_USER "user"


char* javacall_UNICODEsToUtf8(const javacall_utf16* fileName, int fileNameLen) {
    static char result[256];
    if (fileNameLen >= 256) {
        return NULL;	
    }

    result[fileNameLen] = 0;
    while(fileNameLen-- > 0) {
        result[fileNameLen] = fileName[fileNameLen];
    }
    return &result[0];
}

static javacall_result mount_partition(const char* mount_point, const char* partition_label)
{
	ESP_LOGI(TAG, "mount_partition: %s, %s", mount_point, partition_label);
    ESP_LOGI(TAG, "Initializing SPIFFS");
    
    esp_vfs_spiffs_conf_t conf = {
      .base_path = mount_point,
      .partition_label = partition_label,
      .max_files = 10,
      .format_if_mount_failed = true
    };
    
    // Use settings defined above to initialize and mount SPIFFS filesystem.
    // Note: esp_vfs_spiffs_register is an all-in-one convenience function.
    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return JAVACALL_FAIL;
    }
    
    size_t total = 0, used = 0;
    ret = esp_spiffs_info(partition_label, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
		return JAVACALL_FAIL;
    } else {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
		return JAVACALL_OK;
    }
}

static const char* normalize_path(const char* path) {
	static char result[ESP_VFS_PATH_MAX+1];
	memset(result, 0, sizeof(result));
	strncpy(result, path, ESP_VFS_PATH_MAX);
	if (result[strlen(result)-1]=='/') result[strlen(result)-1]='\0';
	return result;
}

/**
 * Initializes the File System
 * @return <tt>JAVACALL_OK</tt> on success, <tt>JAVACALL_FAIL</tt> or negative value on error
 */
javacall_result javacall_file_init(void) {
	if ((mount_partition(normalize_path(JAVACALL_JAMS_NATIVE_ROOT_PATH_UNSECURE), PARTITION_LABEL_UNSECURE) == JAVACALL_OK) &&
	    (mount_partition(normalize_path(JAVACALL_JAMS_NATIVE_ROOT_PATH_SECURE), PARTITION_LABEL_SECURE) == JAVACALL_OK) &&
	    (mount_partition(normalize_path(JC_NATIVE_USER_ROOT_PATH), PARTITION_LABEL_USER) == JAVACALL_OK))
		return JAVACALL_OK;
	else
		return JAVACALL_FAIL;
}
/**
 * Cleans up resources used by file system
 * @return <tt>JAVACALL_OK</tt> on success, <tt>JAVACALL_FAIL</tt> or negative value on error
 */ 
javacall_result javacall_file_finalize(void) {
	esp_vfs_spiffs_unregister(PARTITION_LABEL_UNSECURE);
	esp_vfs_spiffs_unregister(PARTITION_LABEL_SECURE);
	esp_vfs_spiffs_unregister(PARTITION_LABEL_USER);
    return JAVACALL_OK;
}

#define DEFAULT_CREATION_MODE_PERMISSION (0666)

/**
 * The open a file
 * @param unicodeFileName path name in UNICODE of file to be opened
 * @param fileNameLen length of file name
 * @param flags open control flags
 *        Applications must specify exactly one of the first three
 *        values (file access modes) below in the value of "flags"
 *        JAVACALL_FILE_O_RDONLY, JAVACALL_FILE_O_WRONLY, JAVACALL_FILE_O_RDWR
 *
 *        Any combination (bitwise-inclusive-OR) of the following may be used:
 *        JAVACALL_FILE_O_CREAT, JAVACALL_FILE_O_TRUNC, JAVACALL_FILE_O_APPEND,
 *
 * @param handle address of pointer to file identifier
 *        on successful completion, file identifier is returned in this 
 *        argument. This identifier is platform specific and is opaque
 *        to the caller.  
 * @return <tt>JAVACALL_OK</tt> on success, 
 *         <tt>JAVACALL_FAIL</tt> or negative value on error
 * 
 */
javacall_result javacall_file_open(const javacall_utf16 * unicodeFileName, int fileNameLen, int flags, /*OUT*/ javacall_handle * handle) {
	struct stat st;
	int exists;
	char* nativeFlags;
	int basicFlags = flags & 0x03;

    char* pszOsFilename = javacall_UNICODEsToUtf8(unicodeFileName, fileNameLen);

    if (pszOsFilename == NULL) {
       return JAVACALL_FAIL;
    }

	if (stat(pszOsFilename, &st) == 0) {
		exists = 1;
	} else {
		exists = 0;
	}

	if (basicFlags == JAVACALL_FILE_O_RDONLY) {
		nativeFlags = "r";
	}
    
    if ((basicFlags == JAVACALL_FILE_O_RDWR) || (basicFlags == JAVACALL_FILE_O_WRONLY)) {
		if (flags & JAVACALL_FILE_O_CREAT) {
			if (exists) {
				nativeFlags = "r+";
			} else {
				nativeFlags = "w+";
			}
		} else {
			nativeFlags = "r+";
		}
    }


    FILE* fd = fopen(pszOsFilename, nativeFlags);

    if (fd == NULL) {
        *handle = NULL;
        return JAVACALL_FAIL;
    }
    *handle = (javacall_handle)fd;
    return JAVACALL_OK;
}

/**
 * Closes the file with the specified handlei
 * @param handle handle of file to be closed
 * @return <tt>JAVACALL_OK</tt> on success, 
 *         <tt>JAVACALL_FAIL</tt> or negative value otherwise
 */
javacall_result javacall_file_close(javacall_handle handle) {

	int rc = fclose((FILE*)handle);
    return (rc == 0) ? JAVACALL_OK : JAVACALL_FAIL;
}


/**
 * Reads a specified number of bytes from a file, 
 * @param handle handle of file 
 * @param buf buffer to which data is read
 * @param size number of bytes to be read. Actual number of bytes
 *              read may be less, if an end-of-file is encountered
 * @return the number of bytes actually read
 */
long javacall_file_read(javacall_handle handle, unsigned char *buf, long size) {
	return fread(buf, 1, size, (FILE*)handle);
}

/**
 * Writes bytes to file
 * @param handle handle of file 
 * @param buf buffer to be written
 * @param size number of bytes to write
 * @return the number of bytes actually written. This is normally the same 
 *         as size, but might be less (for example, if the persistent storage being 
 *         written to fills up).
 */
long javacall_file_write(javacall_handle handle, const unsigned char *buf, long size) {
	return fwrite(buf, 1, size, (FILE*)handle);
}

/**
 * Deletes a file from the persistent storage.
 * @param unicodeFileName name of file to be deleted
 * @param fileNameLen length of file name
 * @return JAVACALL_OK on success, <tt>JAVACALL_FAIL</tt> or negative value otherwise
 */
javacall_result javacall_file_delete(const javacall_utf16 * unicodeFileName, int fileNameLen) {

    char* pszOsFilename = javacall_UNICODEsToUtf8(unicodeFileName, fileNameLen);
    if (pszOsFilename == NULL) {
    	return JAVACALL_FAIL;
    }

    int status = unlink(pszOsFilename);
    
    
    return (status == 0) ? JAVACALL_OK : JAVACALL_FAIL;
}

/**
 * The  truncate function is used to truncate the size of an open file in 
 * the filesystem storage.
 * @param handle identifier of file to be truncated
 *         This is the identifier returned by javacall_file_open()
 *         The handle may be optionally modified by the implementation
 *         of this function
 * @param size size to truncate to
 * @return <tt>JAVACALL_OK</tt> on success, 
 *         <tt>JAVACALL_FAIL</tt> or negative value on error
 */
javacall_result javacall_file_truncate(javacall_handle handle, javacall_int64 size) {

    return JAVACALL_NOT_IMPLEMENTED;
}

/**
 * Sets the file pointer associated with a file identifier 
 * @param handle identifier of file
 *               This is the identifier returned by javacall_file_open()
 * @param offset number of bytes to offset file position by
 * @param flag controls from where offset is applied, from 
 *                 the beginning, current position or the end
 *                 Can be one of JAVACALL_FILE_SEEK_CUR, JAVACALL_FILE_SEEK_SET 
 *                 or JAVACALL_FILE_SEEK_END
 * @return on success the actual resulting offset from beginning of file
 *         is returned, otherwise -1 is returned
 */
javacall_int64 javacall_file_seek(javacall_handle handle, javacall_int64 offset, javacall_file_seek_flags flag) {

    int whence = 
        (flag == JAVACALL_FILE_SEEK_SET) ? SEEK_SET :
        (flag == JAVACALL_FILE_SEEK_CUR) ? SEEK_CUR :
        (flag == JAVACALL_FILE_SEEK_END) ? SEEK_END : 0;
    
	off_t status = fseek((FILE*)handle, offset, whence);
    return (long)status;
}


/**
 * Get file size 
 * @param handle identifier of file
 *               This is the identifier returned by pcsl_file_open()
 * @return size of file in bytes if successful, -1 otherwise
 */
javacall_int64 javacall_file_sizeofopenfile(javacall_handle handle) {

    struct stat stat_buf;
    int status = fstat((int)fileno((FILE*)handle), &stat_buf);
    return (status == 0) ? stat_buf.st_size : -1;

}

/**
 * Get file size
 * @param fileName name of file in unicode format
 * @param fileNameLen length of file name
 * @return size of file in bytes if successful, -1 otherwise 
 */
javacall_int64 javacall_file_sizeof(const javacall_utf16 * fileName, int fileNameLen) {
    struct stat st;    
	
    char* pszOsFilename = javacall_UNICODEsToUtf8(fileName, fileNameLen);

    if (pszOsFilename == NULL) {
       return -1;
    }
	
	if (stat(pszOsFilename, &st) == 0) {
		return st.st_size;
	} else {
		return -1;
	}
}

/**
 * Check if the file exists in file system storage.
 * @param fileName name of file in unicode format
 * @param fileNameLen length of file name
 * @return <tt>JAVACALL_OK </tt> if it exists and is a regular file, 
 *         <tt>JAVACALL_FAIL</tt> or negative value otherwise (eg: 0 returned if it is a directory)
 */
javacall_result javacall_file_exist(const javacall_utf16 * fileName, int fileNameLen) {
    if (javacall_file_sizeof(fileName, fileNameLen) >= 0) {
		return JAVACALL_OK;
	} else {
		return JAVACALL_FAIL;
	}
}


/** 
 * Force the data to be written into the file system storage
 * @param handle identifier of file
 *               This is the identifier returned by javacall_file_open()
 * @return JAVACALL_OK  on success, <tt>JAVACALL_FAIL</tt> or negative value otherwise
 */
javacall_result javacall_file_flush(javacall_handle handle) {
	return JAVACALL_OK;
}


/**
 * Renames the filename.
 * @param unicodeOldFilename current name of file
 * @param oldNameLen current name length
 * @param unicodeNewFilename new name of file
 * @param newNameLen length of new name
 * @return <tt>JAVACALL_OK</tt>  on success, 
 *         <tt>JAVACALL_FAIL</tt> or negative value otherwise
 */
javacall_result javacall_file_rename(const javacall_utf16 * unicodeOldFilename, int oldNameLen, 
        const javacall_utf16 * unicodeNewFilename, int newNameLen) {

    char* pszOldFilename = javacall_UNICODEsToUtf8(unicodeOldFilename, oldNameLen);
    if (pszOldFilename == NULL) {
        return JAVACALL_FAIL;
    }

    char* pszNewFilename = javacall_UNICODEsToUtf8(unicodeNewFilename, newNameLen);
    if (pszNewFilename == NULL) {
        
        return JAVACALL_FAIL;
    }

    int status = rename(pszOldFilename, pszNewFilename);

    return (status == 0) ? JAVACALL_OK : JAVACALL_FAIL;

}


#ifdef __cplusplus
}
#endif
