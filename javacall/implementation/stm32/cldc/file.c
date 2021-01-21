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
#include "javacall_logging.h"

#include "main.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
//#define FATFS_MKFS_ALLOWED 0
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static FATFS SDFatFs;  /* File system object for SD card logical drive */
static char SDPath[6]; /* SD card logical drive path */

static uint8_t isInitialized = 0;
static uint8_t isMounted = 0;
static uint8_t isInserted = 0;

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

static void SD_Initialize(void)
{
  if (isInitialized == 0)
  {
      javacall_print("Calling BSP_SD_Init()\n");
    if (BSP_SD_Init() == MSD_OK)
    {
      javacall_print("SD initialized\n");
      isInitialized = 1;
    }

    BSP_SD_ITConfig();
  }
}

static javacall_result ensure_SD_mounted() {
    if (!isInitialized || !isInserted) {
        return JAVACALL_FAIL;
    }
    
    if (isMounted) {
        return JAVACALL_OK;
    }
    if(f_mount(&SDFatFs, (TCHAR const*)SDPath, 0) == FR_OK) {
        isMounted = 1;
        javacall_print("SD mount!\n");
        return JAVACALL_OK;
    } else {
        javacall_print("SD mount failed!\n");
        return JAVACALL_FAIL;
    }
}

void javacall_file_set_SDstatus(int inserted) {
    if (!inserted) {
        if (isMounted) {
            isMounted = 0;        
            f_mount(NULL, (TCHAR const*)"", 0);
            javacall_print("SD umount!\n");
        }
        isInserted = 0;
    } else {        
        isInserted = 1;
        ensure_SD_mounted();
    }    
}

/**
 * Initializes the File System
 * @return <tt>JAVACALL_OK</tt> on success, <tt>JAVACALL_FAIL</tt> or negative value on error
 */
javacall_result javacall_file_init(void) {
/* 1- Link the micro SD disk I/O driver */
  javacall_printf("javacall_file_init. _USE_LFN=%d, _LFN_UNICODE=%d, sizeof(TCHAR)=%d\n", _USE_LFN, _LFN_UNICODE, sizeof(TCHAR));
  if(FATFS_LinkDriver(&SD_Driver, SDPath) == 0)
  {
      javacall_printf("SDPath:%s\n", SDPath);
    /*##-2- Init the SD Card #################################################*/

    SD_Initialize();
    
    if(BSP_SD_IsDetected()) {
        isInserted = 1;
        ensure_SD_mounted();
    }

    /* Make sure that the SD detecion IT has a lower priority than the Systick */
    HAL_NVIC_SetPriority(SysTick_IRQn, 0x0E ,0);

    return JAVACALL_OK;
  }
  else
  {
    return JAVACALL_FAIL;
  }   
}
/**
 * Cleans up resources used by file system
 * @return <tt>JAVACALL_OK</tt> on success, <tt>JAVACALL_FAIL</tt> or negative value on error
 */
javacall_result javacall_file_finalize(void) {
    if (isInitialized) {
        f_mount(NULL, (TCHAR const*)"", 0);
        isInitialized = 0;
    }
    return JAVACALL_OK;
}

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
    BYTE mode = 0;
    int basicFlags = flags & 0x03;
    FIL* fileHandle;
    
    if (flags & JAVACALL_FILE_O_CREAT) {
        mode = FA_CREATE_ALWAYS;
    }
    
    if (basicFlags == JAVACALL_FILE_O_RDONLY) {
		mode |= FA_READ;
	} else if (basicFlags == JAVACALL_FILE_O_WRONLY) {
        mode |= FA_WRITE;
    } else if (basicFlags == JAVACALL_FILE_O_RDWR) {
        mode |= (FA_WRITE | FA_READ);
    }
    
    fileHandle = javacall_malloc(sizeof(FIL));
    if (NULL == fileHandle) {
        javacall_print("Javacall failed: out of memory when creating file\n");
        return JAVACALL_FAIL;
    }
    
    char* szFilename = javacall_UNICODEsToUtf8(unicodeFileName, fileNameLen);
    
    if(f_open(fileHandle, szFilename, mode) == FR_OK) {
        *handle = fileHandle;
        return JAVACALL_OK;
    } else {
        javacall_printf("Javacall failed: create file %s\n", szFilename);
        return JAVACALL_FAIL;
    }
}

/**
 * Closes the file with the specified handlei
 * @param handle handle of file to be closed
 * @return <tt>JAVACALL_OK</tt> on success,
 *         <tt>JAVACALL_FAIL</tt> or negative value otherwise
 */
javacall_result javacall_file_close(javacall_handle handle) {
	if (f_close((FIL*)handle) == FR_OK) {
        javacall_free(handle);
        return JAVACALL_OK;
    } else {
        return JAVACALL_FAIL;
    }
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
    UINT actualBytesRead = 0;
    if (f_read((FIL*)handle, buf, size, &actualBytesRead) == FR_OK) {
        return actualBytesRead;
    } else {
        if (f_eof((FIL*)handle)) {
            return 0;
        } else {
            return -1;
        }
    }
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
	UINT actualBytesWrite = 0;
    if (f_write((FIL*)handle, buf, size, &actualBytesWrite) == FR_OK) {
        return actualBytesWrite;
    } else {
        return -1;
    }
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

    if (FR_OK == f_unlink(pszOsFilename)) {
        return JAVACALL_OK;
    } else {
        return JAVACALL_FAIL;
    }
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

    //int status = ftruncate((int)handle, (off_t)size);
    //return (status == 0) ? JAVACALL_OK : JAVACALL_FAIL;
    return JAVACALL_FAIL;
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

    FSIZE_t fp = f_tell((FIL*)handle);
    
    switch (flag) {
        case JAVACALL_FILE_SEEK_SET: break;
        case JAVACALL_FILE_SEEK_CUR: offset += fp; break;
        case JAVACALL_FILE_SEEK_END: offset += f_size((FIL*)handle); break;
        default: return -1;
    }
        
    if (FR_OK == f_lseek((FIL*)handle, offset)) {
        return f_tell((FIL*)handle);
    } else {
        return -1;
    }
}


/**
 * Get file size
 * @param handle identifier of file
 *               This is the identifier returned by pcsl_file_open()
 * @return size of file in bytes if successful, -1 otherwise
 */
javacall_int64 javacall_file_sizeofopenfile(javacall_handle handle) {
	return f_size((FIL*)handle);
}

/**
 * Get file size
 * @param fileName name of file in unicode format
 * @param fileNameLen length of file name
 * @return size of file in bytes if successful, -1 otherwise
 */
javacall_int64 javacall_file_sizeof(const javacall_utf16 * fileName, int fileNameLen) {
    FILINFO info;
    
    if (FR_OK != f_stat (javacall_UNICODEsToUtf8(fileName, fileNameLen), &info)) {
        return -1;
    }
        
    return info.fsize;
}

/**
 * Check if the file exists in file system storage.
 * @param fileName name of file in unicode format
 * @param fileNameLen length of file name
 * @return <tt>JAVACALL_OK </tt> if it exists and is a regular file,
 *         <tt>JAVACALL_FAIL</tt> or negative value otherwise (eg: 0 returned if it is a directory)
 */
javacall_result javacall_file_exist(const javacall_utf16 * fileName, int fileNameLen) {

    FILINFO info;
    char* szFilename;
    
    szFilename = javacall_UNICODEsToUtf8(fileName, fileNameLen);
    
    if (FR_OK != f_stat (szFilename, &info)) {
        javacall_printf("File %s doesn't exist\n", szFilename);
        return JAVACALL_FAIL;
    }
    javacall_printf("File %s exists\n", szFilename);
    return JAVACALL_OK;
}


/**
 * Force the data to be written into the file system storage
 * @param handle identifier of file
 *               This is the identifier returned by javacall_file_open()
 * @return JAVACALL_OK  on success, <tt>JAVACALL_FAIL</tt> or negative value otherwise
 */
javacall_result javacall_file_flush(javacall_handle handle) {
    if (FR_OK == f_sync((FIL*)handle)) {
        return JAVACALL_OK;
    } else {
        return JAVACALL_FAIL;
    }    
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
    char* pszNewFilename = javacall_UNICODEsToUtf8(unicodeNewFilename, newNameLen);
    
    if (FR_OK != f_rename(pszOldFilename, pszNewFilename)) {
        return JAVACALL_FAIL;
    } else {
        return JAVACALL_OK;
    }
}

#ifdef __cplusplus
}
#endif
