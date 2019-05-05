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

/**
* @file
*
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <winbase.h>
#include "javacall_serial.h"
#include "javacall_logging.h"
#include "javacall_memory.h"


#define BAUD_RATE CBR_115200 //CBR_115200;//CBR_9600;

#define REPORT(msg) javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_SERIAL, msg)

HANDLE fd;

#define PLATFORM_SERIAL_BUFFER_SIZE 1024
typedef struct _platform_comm_handle {
	HANDLE fd;
	HANDLE hThreads[2];
	OVERLAPPED reader;
	OVERLAPPED writer;
	BOOL   closed;
	int bytesWrite;
	int bytesInBuffer;
	unsigned char buffer[PLATFORM_SERIAL_BUFFER_SIZE];
} platform_comm_handle;

/**
 * Return an string the contains a list of available ports delimited by a comma
 * (COM1,COM2)
 * If there is no available port then buffer will be empty string and return JAVACALL OK.
 *
 * @param buffer lists of available ports. This value must be null terminated.
 * @param maxBufferLen the maximum length of buffer
 * @retval JAVACALL_OK success
 * @retval JAVACALL_FAIL fail or the return length is more than maxBufferLen characters.
 */
javacall_result
javacall_serial_list_available_ports(char* buffer, int maxBufLen)
{
    char *comPort[] = {"COM1","COM2","COM3","COM4","COM5","COM6","COM7","COM8","COM9"};
    int i, len, totalCount = 0;
    HANDLE hPort;
    char *port;

    buffer[0] = '\0';
    for (i = 0; i < 9; i++) {
        port = comPort[i];
        hPort =  CreateFile(port, 
                GENERIC_READ | GENERIC_WRITE, 
                0,
                0,OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,0);

        if (hPort != INVALID_HANDLE_VALUE) {
            len = strlen(port);
            if (totalCount == 0) {
                if ((int) len >= maxBufLen) {
                    return JAVACALL_FAIL;     
                } else {
                    strcpy(buffer, port);	  
                }
            } else {
                if ((int) totalCount + len + 1 >= maxBufLen) {
                    return JAVACALL_FAIL;     
                } else {
                    strcat(buffer, ",");
                    strcat(buffer, port);
                }
            }
            totalCount += len;
            CloseHandle(hPort);
        } /* end  if (hPort != INVALID_HANDLE_VALUE)*/    
    } /* end for */

    return JAVACALL_OK;
}

static void
wincomm_serial_configurePort(HANDLE hPort, int baudRate, unsigned int options) 
{
    DCB deviceControlBlock;
    COMMTIMEOUTS commTimeouts;

    memset(&deviceControlBlock, 0, sizeof (deviceControlBlock));
    deviceControlBlock.DCBlength = sizeof (deviceControlBlock);
    deviceControlBlock.BaudRate = baudRate;

    /* binary mode, no EOF check */
    deviceControlBlock.fBinary = 1;

    /* 0-4=no,odd,even,mark,space */
    if (options & JAVACALL_SERIAL_ODD_PARITY) {
        deviceControlBlock.fParity = 1;
        deviceControlBlock.Parity = 1;
    } else if (options & JAVACALL_SERIAL_EVEN_PARITY) {
        deviceControlBlock.fParity = 1;
        deviceControlBlock.Parity = 2;
    }

    /* CTS output flow control */
    if (options & JAVACALL_SERIAL_AUTO_CTS) {
        deviceControlBlock.fOutxCtsFlow = 1;
    }

    deviceControlBlock.fDtrControl = DTR_CONTROL_ENABLE;

    /* RTS flow control */
    if (options & JAVACALL_SERIAL_AUTO_RTS) {
        deviceControlBlock.fRtsControl = RTS_CONTROL_HANDSHAKE;
    } else {
        deviceControlBlock.fRtsControl = RTS_CONTROL_ENABLE;
    }

    /* BITS_PER_CHAR_8 is 2 bits and includes BITS_PER_CHAR_7 */
    if ((options & JAVACALL_SERIAL_BITS_PER_CHAR_8)
            == JAVACALL_SERIAL_BITS_PER_CHAR_8) {
        deviceControlBlock.ByteSize = 8;
    } else {
        deviceControlBlock.ByteSize = 7;
    }

    /* StopBits 0,1,2 = 1, 1.5, 2 */
    if (options & JAVACALL_SERIAL_STOP_BITS_2) {
        deviceControlBlock.StopBits = 2;
    }

    if (!SetCommState((HANDLE) hPort, &deviceControlBlock)) {
	REPORT("configurePort << ERROR SetCommState\n");        
        return;
    }

    if (!GetCommTimeouts((HANDLE) hPort, &commTimeouts)) {
	REPORT("configurePort << ERROR GetCommTime timeouts\n");
        return;
    }

    /* set non blocking (async methods) */
    commTimeouts.ReadIntervalTimeout = MAXDWORD;
    commTimeouts.ReadTotalTimeoutMultiplier = 0;
    commTimeouts.ReadTotalTimeoutConstant = 0;

    commTimeouts.WriteTotalTimeoutMultiplier = 0;
    commTimeouts.WriteTotalTimeoutConstant = 0;

    if (!SetCommTimeouts((HANDLE) hPort, &commTimeouts)) {
     	 REPORT("configurePort << ERROR cannot SetCommTimeouts timeouts\n");   	
        return;
    }
}

static DWORD WINAPI SerialReaderThread( LPVOID lpParam ) {
   platform_comm_handle* h = (platform_comm_handle*)lpParam;

   while (!h->closed) {
   	   DWORD dwRead;
       DWORD dwRes = WaitForSingleObject(h->reader.hEvent, INFINITE);
	   if (!h->closed) {
		   switch(dwRes)
		   {
		      // Read completed.
		      case WAIT_OBJECT_0:
		          if (!GetOverlappedResult(h->fd, &h->reader, &dwRead, FALSE)) {
		             // Error in communications; report it.
		             javanotify_serial_event(JAVACALL_EVENT_SERIAL_RECEIVE,
		                                     h, JAVACALL_FAIL);
		          } else {
		             // Read completed successfully.
		             h->bytesInBuffer = (int)dwRead;
		             javanotify_serial_event(JAVACALL_EVENT_SERIAL_RECEIVE,
		                                     h, JAVACALL_OK);
		          }
		   	}
	   	}
   	}

   
	CloseHandle(h->reader.hEvent);

	javanotify_serial_event(JAVACALL_EVENT_SERIAL_RECEIVE,
												 h, JAVACALL_INTERRUPTED);

	return 0;
}

static DWORD WINAPI SerialWriterThread( LPVOID lpParam ) {
   platform_comm_handle* h = (platform_comm_handle*)lpParam;

   while (!h->closed) {
   	   DWORD dwWritten;
       DWORD dwRes = WaitForSingleObject(h->writer.hEvent, INFINITE);
	   if (!h->closed) {
		   switch(dwRes)
		   {
		      // Read completed.
		      case WAIT_OBJECT_0:
		          if (!GetOverlappedResult(h->fd, &h->writer, &dwWritten, FALSE)) {
		             // Error in communications; report it.
		             javanotify_serial_event(JAVACALL_EVENT_SERIAL_WRITE,
		                                     h, JAVACALL_FAIL);
		          } else {
		             // Write completed successfully.
                     h->bytesWrite = (int)dwWritten;
		             javanotify_serial_event(JAVACALL_EVENT_SERIAL_WRITE,
		                                     h, JAVACALL_OK);
		          }
		   	}
	   	}
   	}

   
	CloseHandle(h->writer.hEvent);

	javanotify_serial_event(JAVACALL_EVENT_SERIAL_WRITE,
												 h, JAVACALL_INTERRUPTED);

	return 0;
}

/**
 * Opens serial link according to the given parameters
 *
 * @param devName the name of the port / device to be opened ("COMM1")
 * @param baudRate the baud rate for the open connection
 * @param options the serial link option (JAVACALL_SERIAL_XXX)
 * @return <tt>hPort</tt> on success, 
 *         <tt>-1</tt> on error
 */
static javacall_handle
wincomm_serial_open(const char *devName, int baudRate, unsigned int options)
{
    DCB dcbSerialParams = {0};
    COMMTIMEOUTS timeouts={0};
    char lastError[1024];
    HANDLE fd;	
    LPCTSTR lpFileName;
	platform_comm_handle* comm_handle;
	DWORD dwThreadID = (DWORD)0;

    lpFileName = devName;
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,GetLastError(),MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),lastError,1024,NULL);

    fd = CreateFile(lpFileName, 
        GENERIC_READ | GENERIC_WRITE, 
        0,
        0,OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,0);

    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,GetLastError(),MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),lastError,1024,NULL);

    if (fd == INVALID_HANDLE_VALUE) {
        REPORT("javacall_serial_open: couldn't open port!\n");
        return (javacall_handle) -1;
    }

	HANDLE r_evt = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (r_evt == NULL) {
		REPORT("javacall_serial_open: couldn't create event!\n");
		CloseHandle(fd);
        return (javacall_handle) -1;
	}

	HANDLE w_evt = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (w_evt == NULL) {
		REPORT("javacall_serial_open: couldn't create event!\n");
		CloseHandle(fd);
		CloseHandle(r_evt);
        return (javacall_handle) -1;
	}
	
    wincomm_serial_configurePort(fd, baudRate, options);

	comm_handle = javacall_malloc(sizeof(platform_comm_handle));
	memset(comm_handle, 0, sizeof(platform_comm_handle));
	comm_handle->fd = fd;
	comm_handle->reader.hEvent = r_evt;
	comm_handle->writer.hEvent = w_evt;
	
	comm_handle->hThreads[0] = CreateThread( 
            NULL,                   
            0,                        
            SerialReaderThread,       
            comm_handle,           
            0,                       
            &dwThreadID);    

	comm_handle->hThreads[1] = CreateThread( 
            NULL,                   
            0,                        
            SerialWriterThread,       
            comm_handle,           
            0,                       
            &dwThreadID);    
		
    return (javacall_handle)comm_handle; 
}

/**
 * Update the baudRate of an open serial port
 *
 * @param hPort the port to configure
 * @param baudRate the new baud rate for the open connection
 * @return <tt>JAVACALL_OK</tt> on success, 
 *         <tt>JAVACALL_FAIL</tt> on error
 */
javacall_result
javacall_serial_set_baudRate(javacall_handle hPort, int baudRate)
{
   DCB dcbSerialParams = {0};
   hPort = ((platform_comm_handle*)hPort)->fd;

   dcbSerialParams.BaudRate = baudRate; //2400,4800,9600,115200...
   if (!SetCommState(hPort, &dcbSerialParams)) {
        REPORT("javacall_serial_set_baudRate: error setting serial params!\n");
        return JAVACALL_FAIL;
   }
  
   return JAVACALL_OK;
}

/**
 * Retrive the current baudRate of the open serial port
 *
 * @param hPort the port to configure
 * @param baudRate pointer to where to return the baudRate
 * @return <tt>JAVACALL_OK</tt> on success, 
 *         <tt>JAVACALL_FAIL</tt> on error
 */
javacall_result /*OPTIONAL*/
javacall_serial_get_baudRate(javacall_handle hPort, int *baudRate)
{
    DCB dcbSerialParams = {0};
	hPort = ((platform_comm_handle*)hPort)->fd;
    if (!GetCommState(hPort, &dcbSerialParams)) {
        REPORT("javacall_serial_init: error setting serial params!\n");
        return JAVACALL_FAIL;
    }  

    *baudRate = dcbSerialParams.BaudRate;
  
    return JAVACALL_OK;
}

/**
 * Configure serial port
 *
 * @param hPort the port to configure
 * @param baudRate the new baud rate for the open connection
 * @param options options for the serial port:
 * bit 0: 0 - 1 stop bit, 1 - 2 stop bits 
 * bit 2-1: 00 - no parity, 01 - odd parity, 10 - even parity 
 * bit 4: 0 - no auto RTS, 1 - set auto RTS 
 * bit 5: 0 - no auto CTS, 1 - set auto CTS 
 * bit 7-6: 01 - 7 bits per symbol, 11 - 8 bits per symbol 
 * @return <tt>JAVACALL_OK</tt> on success, 
 *         <tt>JAVACALL_FAIL</tt> on error
 */
javacall_result /*OPTIONAL*/ javacall_serial_configure(javacall_handle pHandle, int baudRate, int options) {
    wincomm_serial_configurePort(((platform_comm_handle*)pHandle)->fd, baudRate, options);
	return JAVACALL_OK;
}

/**
 * Reads a specified number of bytes from serial link, 
 
 * @param hPort the port to read the data from
 * @param buffer to which data is read
 * @param size number of bytes to be read. Actual number of bytes
 *              read may be less, if less data is available
 * @param byteRead actual number the were read from the port.
 * @retval JAVACALL_OK          success
 * @retval JAVACALL_FAIL        if there was an error   
 * @retval JAVACALL_WOULD_BLOCK  if the caller must call the finish function again to complete the operation
 */
static javacall_result
wincomm_serial_read(javacall_handle hPort, unsigned char* buffer, int size, int *byteRead)
{
	platform_comm_handle* pHandle = (platform_comm_handle*)hPort;
	int num = 0;

	// int nByteAvailable = pHandle->bytesInBuffer;
	// if (nByteAvailable > 0) {
	// 	if (size > nByteAvailable) {
	// 		size = nByteAvailable;
	// 	}
	// 	memcpy(buffer, &(pHandle->buffer[0]), size);
	// 	*byteRead = size;
	// 	nByteAvailable -= size;
	// 	pHandle->bytesInBuffer = nByteAvailable;
		
	// 	return JAVACALL_OK;
	// }

	if (size > PLATFORM_SERIAL_BUFFER_SIZE) {
		size = PLATFORM_SERIAL_BUFFER_SIZE;
	}
	
    // pHandle->bytesInBuffer = 0;
	
    if (!ReadFile((HANDLE)pHandle->fd, &(pHandle->buffer[0]), size, &num, &(pHandle->reader))) {
		if (GetLastError() != ERROR_IO_PENDING) {
         // Error in communications; report it.
         REPORT("javacall_serial_read: error reading\n");
         return JAVACALL_FAIL;
      	} else {         
         return JAVACALL_WOULD_BLOCK;
      	}
    } else {
		if (num > 0) {
			memcpy(buffer, &(pHandle->buffer[0]), num);
        }
        *byteRead = num;
        return JAVACALL_OK;
    }
} 


/**
 * Writes a specified number of bytes to serial link, 
 * @param hPort the port to write the data to
 * @param buffer buffer to write data from
 * @param size number of bytes to be write.
 * @param bytesWrite the point to the number of bytes actually written
 */
static javacall_result
wincomm_serial_write(javacall_handle hPort, unsigned char* buffer, int size, int* bytesWrite)
{
    DWORD dwBytesWrite = 0;
	platform_comm_handle* pHandle = (platform_comm_handle*)hPort;
    if (!WriteFile((HANDLE)pHandle->fd, buffer, size, &dwBytesWrite, &(pHandle->writer))) {
		if (GetLastError() != ERROR_IO_PENDING) {
         // Error in communications; report it.
         REPORT("javacall_serial_write: error writing\n");
         return JAVACALL_FAIL;
      	} else {        
         return JAVACALL_WOULD_BLOCK;
      	}        
    }
	*bytesWrite = (int)dwBytesWrite;
    return JAVACALL_OK;
} 

/**
 * Closes serial link 

 * @param hPort the port to close
 * after this call, java is guaranteed not to call javacall_serial_read() or 
 * javacall_serial_write() before issuing another javacall_serial_open( ) call.
 *
 * @return <tt>JAVACALL_OK</tt> on success, 
 *         <tt>JAVACALL_FAIL</tt> or negative value on error
 */
static javacall_result
wincomm_serial_close(javacall_handle hPort)
{
    int result;	
	javacall_result ret;
	platform_comm_handle* pHandle = (platform_comm_handle*)hPort;
	HANDLE comm = ((platform_comm_handle*)hPort)->fd;
	
	REPORT("javacall_serial_finalize \n");		

	pHandle->closed = TRUE;
	SetEvent(pHandle->reader.hEvent);
	SetEvent(pHandle->writer.hEvent);

	WaitForMultipleObjects(2, pHandle->hThreads, TRUE, 500);
	
    result = CloseHandle(comm);
    if (result == 0) { 
        REPORT("ERROR javacall_serial_finalize failed!!!! \n");
        ret = JAVACALL_FAIL;
    }

	javacall_free(pHandle);
	
    REPORT("javacall_serial_finalize <<\n");
     	
    return ret;    
}

/**
 * Initiates opening serial link according to the given parameters
 *
 * @param devName the name of the port / device to be opened ("COM1")
 * @param baudRate the baud rate for the open connection. in case
 *        bauseRate=JAVACALL_UNSPECIFIED_BAUD_RATE
 *        the baudRate that was specified by the platform should be used
 * @param options the serial link option (JAVACALL_SERIAL_XXX)
 * @param pHandle the handle of the port to be opend
 * @param pContext filled by ptr to data for reinvocations
 * @retval JAVACALL_OK          success
 * @retval JAVACALL_FAIL        if there was an error   
 * @retval JAVACALL_WOULD_BLOCK  if the caller must call the finish function again to complete the operation
 */
javacall_result  /*OPTIONAL*/
javacall_serial_open_start(const char *devName, int baudRate, unsigned int options ,javacall_handle *hPort, void **pContext)
{
	*pContext = NULL;
    *hPort = wincomm_serial_open(devName, baudRate, options);
    if ((javacall_handle)-1 == *hPort) {
        return JAVACALL_FAIL;
    } else {
        return JAVACALL_OK;
    }
}

/**
 * Finishes opening serial link according to the given parameters
 *
 * @param devName the name of the port / device to be opened ("COM1")
 * @param baudRate the baud rate for the open connection. in case
 *        bauseRate=JAVACALL_UNSPECIFIED_BAUD_RATE
 *        the baudRate that was specified by the platform should be used
 * @param options the serial link option (JAVACALL_SERIAL_XXX)
 * @param pHandle the handle of the port to be opend
 * @param context ptr to the data of reinvokation context
 * @retval JAVACALL_OK on success, 
 * @retval JAVACALL_FAIL on error
 * @retval JAVACALL_WOULD_BLOCK  if the caller must call the finish function again to complete the operation
 */
javacall_result  /*OPTIONAL*/ javacall_serial_open_finish(const char *devName, int baudRate, unsigned int options, 
	  /*OUT*/javacall_handle *pHandle, void *context)

{
    return JAVACALL_FAIL;    
}

/**
 * Initiates closing serial link 
 *
 * @param hPort the port to close
 * @param pContext filled by ptr to data for reinvocations
 * after this call, java is guaranteed not to call javacall_serial_read() or 
 * javacall_serial_write() before issuing another javacall_serial_open( ) call.
 *
 * @return <tt>JAVACALL_OK</tt> on success, 
 *         <tt>JAVACALL_FAIL</tt>
 * @retval JAVACALL_WOULD_BLOCK  if the caller must call the finish function again to complete the operation
 */
javacall_result /*OPTIONAL*/
javacall_serial_close_start(javacall_handle hPort, void **pContext)
{
	*pContext= NULL;
    if (wincomm_serial_close(hPort) == JAVACALL_OK) 
        return JAVACALL_OK;
    else
	return JAVACALL_FAIL;	   		 
}

/**
 * Finishes closing serial link.
 *
 * @param hPort the port to close
 * @param context ptr to data saved before sleeping
 * @retval <tt>JAVACALL_OK</tt> on success, 
 * @retval <tt>JAVACALL_FAIL</tt>
 * @retval JAVACALL_WOULD_BLOCK  if the caller must call the finish function again to complete the operation
 */
javacall_result /*OPTIONAL*/
javacall_serial_close_finish(javacall_handle hPort, void *context)
{
    return JAVACALL_OK;
}

/**
 * Initiates reading a specified number of bytes from serial link, 
 
 * @param hPort the port to read the data from
 * @param buffer to which data is read
 * @param size number of bytes to be read. Actual number of bytes
 *              read may be less, if less data is available
 * @param byteRead actual number the were read from the port.
 * @param pContext filled by ptr to data for reinvocations
 * @retval JAVACALL_OK          success
 * @retval JAVACALL_FAIL        if there was an error   
 * @retval JAVACALL_WOULD_BLOCK  if the caller must call the finish function again to complete the operation
 */
javacall_result /*OPTIONAL*/
javacall_serial_read_start(javacall_handle hPort, unsigned char* buffer, int size ,int *bytesRead, void **pContext)
{
	*pContext = NULL;
    return wincomm_serial_read(hPort, buffer, size, bytesRead);
}

/**
 * Finishes reading a specified number of bytes from serial link, 
 *
 * @param hPort the port to read the data from
 * @param buffer to which data is read
 * @param size number of bytes to be read. Actual number of bytes
 *              read may be less, if less data is available
 * @param byteRead actual number the were read from the port.
 * @param context ptr to data saved before sleeping
 * @retval JAVACALL_OK          success
 * @retval JAVACALL_FAIL        if there was an error   
 * @retval JAVACALL_WOULD_BLOCK  if the caller must call the finish function again to complete the operation
 */
javacall_result /*OPTIONAL*/
javacall_serial_read_finish(javacall_handle hPort, unsigned char* buffer, int size, int *bytesRead, void *context)
{
    return wincomm_serial_read(hPort, buffer, size, bytesRead);;
}

/**
 * Initiates writing a specified number of bytes to serial link, 
 *
 * @param hPort the port to write the data to
 * @param buffer buffer to write data from
 * @param size number of bytes to be write.
 * @param bytesWritten the number of bytes actually written.
 * @param pContext filled by ptr to data for reinvocations
 * @retval JAVACALL_OK          success
 * @retval JAVACALL_FAIL        if there was an error   
 * @retval JAVACALL_WOULD_BLOCK  if the caller must call the finish function again to complete the operation
 */
javacall_result /*OPTIONAL*/
javacall_serial_write_start(javacall_handle hPort, unsigned char* buffer, int size, int *bytesWritten, void **pContext)
{ 
	*pContext = NULL;
    return wincomm_serial_write(hPort, buffer, size, bytesWritten);    
}

/**
 * Finishes writing a specified number of bytes to serial link, 
 *
 * @param hPort the port to write the data to
 * @param buffer buffer to write data from
 * @param size number of bytes to be write.
 * @param bytesWritten the number of bytes actually written.
 * @param context ptr to data saved before sleeping
 * @retval JAVACALL_OK          success
 * @retval JAVACALL_FAIL        if there was an error   
 */
javacall_result /*OPTIONAL*/
javacall_serial_write_finish(javacall_handle hPort, unsigned char* buffer, int size, int *bytesWritten, void *context)
{
    platform_comm_handle* pHandle = (platform_comm_handle*)hPort;
    *bytesWritten = pHandle->bytesWrite;
    pHandle->bytesWrite = 0;
    return JAVACALL_OK;
}
