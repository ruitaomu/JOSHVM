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
#include "incls/_OsSerialport_javacall.cpp.incl"

#include <javacall_events.h>
#include <javacall_serial.h>
#include <pcsl_network.h>
#include <serial_port_export.h>

/**
 * Open a serial port by logical device name.
 *
 * @param pszDeviceName logical name of the port (for example, COM1)
 * @param baudRate baud rate to set the port at
 * @param options options for the serial port
 * bit 0: 0 - 1 stop bit, 1 - 2 stop bits
 * bit 2-1: 00 - no parity, 01 - odd parity, 10 - even parity
 * bit 4: 0 - no auto RTS, 1 - set auto RTS
 * bit 5: 0 - no auto CTS, 1 - set auto CTS
 * bit 7-6: 01 - 7 bits per symbol, 11 - 8 bits per symbol
 * @param pHandle returns the connection handle; it is
 *        set only when this function returns PCSL_NET_SUCCESS
 * @param pContext filled by ptr to data for reinvocations
 *
 * @return PCSL_NET_SUCCESS for successful read operation;\n
 *       PCSL_NET_WOULDBLOCK if the operation would block,\n
 *       PCSL_NET_INTERRUPTED for an Interrupted IO Exception,\n
 *       PCSL_NET_IOERROR for all other errors
 */
int
openPortByNameStart(char* pszDeviceName, int baudRate,
					int options, int *pHandle, void **pContext)
{
    javacall_handle hPort = (javacall_handle)-1;
    javacall_result ret;
	int returnStatus;

	ret = javacall_serial_open_start(pszDeviceName, baudRate, options, &hPort, pContext);
    *pHandle = (int)hPort;

	switch (ret) {
		case JAVACALL_OK:
			returnStatus = PCSL_NET_SUCCESS;
			break;
		case JAVACALL_WOULD_BLOCK:
			returnStatus = PCSL_NET_WOULDBLOCK;
			break;
		default:
			returnStatus = PCSL_NET_IOERROR;
			break;
	}

	return returnStatus;
}

/**
 * Open a serial port by system dependent device name.
 *
 * @param pszDeviceName device name of the port
 * @param baudRate baud rate to set the port at
 * @param options options for the serial port
 * bit 0: 0 - 1 stop bit, 1 - 2 stop bits
 * bit 2-1: 00 - no parity, 01 - odd parity, 10 - even parity
 * bit 4: 0 - no auto RTS, 1 - set auto RTS
 * bit 5: 0 - no auto CTS, 1 - set auto CTS
 * bit 7-6: 01 - 7 bits per symbol, 11 - 8 bits per symbol
 * @param pHandle returns the connection handle; it is
 *        set only when this function returns PCSL_NET_SUCCESS
 * @param context ptr to data saved before sleeping
 *
 * @return PCSL_NET_SUCCESS for successful read operation;\n
 *       PCSL_NET_WOULDBLOCK if the operation would block,\n
 *       PCSL_NET_INTERRUPTED for an Interrupted IO Exception,\n
 *       PCSL_NET_IOERROR for all other errors
 */
int openPortByNameFinish(char* pszDeviceName, int baudRate,
    int options, int *pHandle, void *context) {

	javacall_handle hPort = (javacall_handle)-1;
    javacall_result ret;
	int returnStatus;

    ret = javacall_serial_open_finish(pszDeviceName, baudRate, options, &hPort, context);
    *pHandle = (int)hPort;

	switch (ret) {
		case JAVACALL_OK:
			returnStatus = PCSL_NET_SUCCESS;
			break;
		case JAVACALL_WOULD_BLOCK:
			returnStatus = PCSL_NET_WOULDBLOCK;
			break;
		default:
			returnStatus = PCSL_NET_IOERROR;
			break;
	}

	return returnStatus;
}

/**
 * Configure a serial port optional parameters.
 *
 * @param hPort port number
 * @param baudRate baudRate rate to set the port at
 * @param options options for the serial port:
 * bit 0: 0 - 1 stop bit, 1 - 2 stop bits
 * bit 2-1: 00 - no parity, 01 - odd parity, 10 - even parity
 * bit 4: 0 - no auto RTS, 1 - set auto RTS
 * bit 5: 0 - no auto CTS, 1 - set auto CTS
 * bit 7-6: 01 - 7 bits per symbol, 11 - 8 bits per symbol
 *
 * @return PCSL_NET_SUCCESS for successful configure operation;\n
 *       PCSL_NET_IOERROR for any error
 */
int configurePort(int hPort, int baudRate, unsigned int options) {
    int ret_v = PCSL_NET_IOERROR;
    if (JAVACALL_OK ==
	    javacall_serial_configure((javacall_handle)hPort, baudRate, options)) {
        ret_v = PCSL_NET_SUCCESS;
    }
	return ret_v;
}

/**
 * Close a serial port.
 *
 * @param hPort handle to a native serial port
 */

void closePort(int hPort)
{
    javacall_result ret;
    void* context = NULL;
    SNIReentryData* info = (SNIReentryData*)SNI_GetReentryData(NULL);

    if (info == NULL) { //first invocation
        javanotify_uart_event(BREAK_INTERRUPT, (javacall_handle)hPort, 0, JAVACALL_INTERRUPTED);
        ret = javacall_serial_close_start((javacall_handle)hPort, &context);
    } else { /* Reinvocation */
        hPort = info->descriptor;
        context = info->pContext;
        ret = javacall_serial_close_finish((javacall_handle)hPort, context);
    }

    if (JAVACALL_WOULD_BLOCK == ret) {
        SNIEVT_wait(COMM_CLOSE_SIGNAL, hPort, context);
    }
}

/**
 * Write to a serial port without blocking.
 *
 * @param hPort handle to a native serial port
 * @param pBuffer I/O buffer
 * @param nNumberOfBytesToWrite length of data
 * @param pBytesWritten returns the number of bytes written after
 *        successful write operation; only set if this function returns
 *        PCSL_NET_SUCCESS
 * @param pContext filled by ptr to data for reinvocations
 *
 * @return PCSL_NET_SUCCESS for successful write operation;\n
 *       PCSL_NET_WOULDBLOCK if the operation would block,\n
 *       PCSL_NET_INTERRUPTED for an Interrupted IO Exception\n
 *       PCSL_NET_IOERROR for all other errors
 */
int writeToPortStart(int hPort, char* pBuffer,
    int nNumberOfBytesToWrite, int* pBytesWritten, void **pContext) {
    int status;

    if (nNumberOfBytesToWrite == 0) {
        *pBytesWritten = 0;
        status = PCSL_NET_SUCCESS;
    } else {
        javacall_result ret;
        ret = javacall_serial_write_start((javacall_handle) hPort,
           (unsigned char*) pBuffer, nNumberOfBytesToWrite, pBytesWritten, pContext);

		switch (ret) {
			case JAVACALL_OK:
				status = PCSL_NET_SUCCESS;
				break;
			case JAVACALL_WOULD_BLOCK:
				status = PCSL_NET_WOULDBLOCK;
				break;
			default:
				status = PCSL_NET_IOERROR;
				break;
		}
	}
	return status;
}

/**
 * Write to a serial port without blocking.
 *
 * @param hPort handle to a native serial port
 * @param pBuffer I/O buffer
 * @param nNumberOfBytesToWrite length of data
 * @param pBytesWritten returns the number of bytes written after
 *        successful write operation; only set if this function returns
 *        PCSL_NET_SUCCESS
 * @param context ptr to data saved before sleeping
 *
 * @return PCSL_NET_SUCCESS for successful write operation;\n
 *       PCSL_NET_WOULDBLOCK if the operation would block,\n
 *       PCSL_NET_INTERRUPTED for an Interrupted IO Exception\n
 *       PCSL_NET_IOERROR for all other errors
 */
int writeToPortFinish(int hPort, char* pBuffer,
        int nNumberOfBytesToWrite, int* pBytesWritten, void *context) {
    int status;

    if (nNumberOfBytesToWrite == 0) {
        *pBytesWritten = 0;
        status = PCSL_NET_SUCCESS;
    } else {
        javacall_result ret;
        ret = javacall_serial_write_finish((javacall_handle) hPort,
           (unsigned char*) pBuffer, nNumberOfBytesToWrite, pBytesWritten, context);

		switch (ret) {
			case JAVACALL_OK:
				status = PCSL_NET_SUCCESS;
				break;
			case JAVACALL_WOULD_BLOCK:
				status = PCSL_NET_WOULDBLOCK;
				break;
			default:
				status = PCSL_NET_IOERROR;
				break;
		}
	}
	return status;
}

/**
 * Read from a serial port.
 *
 * @param hPort handle to a native serial port
 * @param pBuffer I/O buffer
 * @param nNumberOfBytesToRead length of data
 * @param pBytesRead returns the number of bytes actually read; it is
 *        set only when this function returns PCSL_NET_SUCCESS
 * @param pContext filled by ptr to data for reinvocations
 *
 * @return PCSL_NET_SUCCESS for successful read operation;\n
 *       PCSL_NET_WOULDBLOCK if the operation would block,\n
 *       PCSL_NET_INTERRUPTED for an Interrupted IO Exception,\n
 *       PCSL_NET_IOERROR for all other errors
 */
int readFromPortStart(int hPort, char* pBuffer,
                 int nNumberOfBytesToRead, int* pBytesRead, void **pContext) {
    int status;

    if (nNumberOfBytesToRead == 0) {
        *pBytesRead = 0;
        status = PCSL_NET_SUCCESS;
    } else {

	    javacall_result ret;

		ret = javacall_serial_read_start((javacall_handle) hPort,
               (unsigned char*) pBuffer,
               nNumberOfBytesToRead, pBytesRead, pContext);

		switch (ret) {
			case JAVACALL_OK:
				status = PCSL_NET_SUCCESS;
				break;
			case JAVACALL_WOULD_BLOCK:
				status = PCSL_NET_WOULDBLOCK;
				break;
			default: 
				status = PCSL_NET_IOERROR;
				break;
		}
	}
	return status;
}

/**
 * Read from a serial port.
 *
 * @param hPort handle to a native serial port
 * @param pBuffer I/O buffer
 * @param nNumberOfBytesToRead length of data
 * @param pBytesRead returns the number of bytes actually read; it is
 *        set only when this function returns PCSL_NET_SUCCESS
 * @param context ptr to data saved before sleeping
 *
 * @return PCSL_NET_SUCCESS for successful read operation;\n
 *       PCSL_NET_WOULDBLOCK if the operation would block,\n
 *       PCSL_NET_INTERRUPTED for an Interrupted IO Exception,\n
 *       PCSL_NET_IOERROR for all other errors
 */
int readFromPortFinish(int hPort, char* pBuffer,
    int nNumberOfBytesToRead, int* pBytesRead, void *context) {
    int status;

    if (nNumberOfBytesToRead == 0) {
        *pBytesRead = 0;
        status = PCSL_NET_SUCCESS;
    } else {

	    javacall_result ret;

		ret = javacall_serial_read_finish((javacall_handle) hPort,
               (unsigned char*) pBuffer,
               nNumberOfBytesToRead, pBytesRead, context);

		switch (ret) {
			case JAVACALL_OK:
				status = PCSL_NET_SUCCESS;
				break;
			case JAVACALL_WOULD_BLOCK:
				status = PCSL_NET_WOULDBLOCK;
				break;
			default: 
				status = PCSL_NET_IOERROR;
				break;
		}
	}
	return status;
}

void javanotify_uart_event(javacall_uart_event_type type,javacall_handle hPort,javacall_int32 bytesProcessed,
	                                javacall_result result) {
    SNIReentryData rd;

	rd.status = -1;
	rd.descriptor = (int)hPort;

	switch (type) {
		case INPUT_DATA_AVAILABLE:
			rd.waitingFor = COMM_READ_SIGNAL;
			rd.status = ((result == JAVACALL_OK)?bytesProcessed:-1);
			break;
		case OUTPUT_BUFFER_EMPTY:
			rd.waitingFor = COMM_WRITE_SIGNAL;
			rd.status = ((result == JAVACALL_OK)?0:-1);
			break;
		case BREAK_INTERRUPT:
			rd.waitingFor = COMM_READ_SIGNAL;
			rd.status = -2;
			break;
		case INPUT_BUFFER_OVERRUN:
			rd.status = -3;
    	case PARITY_ERROR:
			rd.status = -4;
		case FRAMING_ERROR:
			rd.status = -5;
			rd.waitingFor = COMM_READ_SIGNAL;
			break;

		default:
			/* IMPL_NOTE: decide what to do */
			return;
			/* do not send event to java */
		}
	javacall_event_send((unsigned char*)&rd, sizeof(SNIReentryData));
}

void javanotify_serial_event(
                             javacall_serial_callback_type type,
                             javacall_handle hPort,
                             javacall_result operation_result) {
    SNIReentryData rd;
	rd.descriptor = (int)hPort;
	rd.pContext = NULL;
	switch(operation_result) {
		case JAVACALL_OK:
			rd.status = 0;
			break;
		case JAVACALL_INTERRUPTED:
			rd.status = -2;
			break;
		default:
			rd.status = -1;
			break;
	}
	rd.status = (int)operation_result;

		switch (type) {
		case JAVACALL_EVENT_SERIAL_RECEIVE:
			rd.waitingFor = COMM_READ_SIGNAL;
			break;
		case JAVACALL_EVENT_SERIAL_WRITE:
			rd.waitingFor = COMM_WRITE_SIGNAL;
			break;
		case JAVACALL_EVENT_SERIAL_OPEN:
			rd.waitingFor = COMM_OPEN_SIGNAL;
			break;
		case JAVACALL_EVENT_SERIAL_CLOSE:
			rd.waitingFor = COMM_CLOSE_SIGNAL;
			break;

		default:
			/* IMPL_NOTE: decide what to do */
			return;
			/* do not send event to java */
		}
	javacall_event_send((unsigned char*)&rd, sizeof(SNIReentryData));
}
