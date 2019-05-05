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

/*=========================================================================
 * SYSTEM:    KVM
 * SUBSYSTEM: networking
 * FILE:      commProtocol.c
 * OVERVIEW:  Operations to support serial communication ports
 *            (native implementation of the 'comm:' protocol).
 *
 *            These functions are mapped onto platform-specific
 *            functions defined in Vm<Port>/src directories,
 *            where <Port> indicates the name of the target
 *            platform (e.g., Win, Unix, Pilot).
 *=======================================================================*/

/*=======================================================================
 * Include files
 *=======================================================================*/
	 
#include "incls/_precompiled.incl"
#include "incls/_commProtocol.cpp.incl"

#if ENABLE_PCSL
extern "C" {
#include <pcsl_network.h>
#include <pcsl_memory.h>
}
#endif

#include <string.h>
#include <serial_port_export.h>

/*=======================================================================
 * Protocol implementation functions
 *=======================================================================*/

/**
 * Configure a serial port optional parameters.
 *
 * @param port device port returned from open
 * @param baud baud rate to set the port at
 * @param flags options for the serial port:
 * bit 0: 0 - 1 stop bit, 1 - 2 stop bits 
 * bit 2-1: 00 - no parity, 01 - odd parity, 10 - even parity 
 * bit 4: 0 - no auto RTS, 1 - set auto RTS 
 * bit 5: 0 - no auto CTS, 1 - set auto CTS 
 * bit 7-6: 01 - 7 bits per symbol, 11 - 8 bits per symbol 
 */
KNIEXPORT KNI_RETURNTYPE_VOID
    Java_com_sun_cldc_io_j2me_comm_Protocol_native_1configurePort() {
#if ENABLE_PCSL
    unsigned int  flags      = (int)KNI_GetParameterAsInt(3);
    int  baud       = (int)KNI_GetParameterAsInt(2);
    int  port       = (int)KNI_GetParameterAsInt(1);

    int status = configurePort(port, baud, flags);
    if (status != PCSL_NET_SUCCESS) {
        jvm_sprintf((char*)gKNIBuffer,
                "Error configure port %d \n", port);
        KNI_ThrowNew(KNIIOException, (char*)gKNIBuffer);
    }
#endif
    KNI_ReturnVoid();
}

/**
 * Open a serial port by system dependent device name.
 *
 * @param name device name of the port
 * @param baud baud rate to set the port at
 * @param flags options for the serial port
 *
 * @return handle to a native serial port
 *
 * @exception  IOException  if an I/O error occurs.
 */
KNIEXPORT KNI_RETURNTYPE_INT
    Java_com_sun_cldc_io_j2me_comm_Protocol_native_1openByName() {
#if ENABLE_PCSL
	
		int    flags = (int)KNI_GetParameterAsInt(3);
		int    baud = (int)KNI_GetParameterAsInt(2);
		int    nameLen;
		char   szName[MAX_NAME_LEN];
		jchar* temp;
		int    hPort = (int)INVALID_HANDLE;
		int    i;
		int status = PCSL_NET_IOERROR;
		void* context = NULL;
		SNIReentryData* info;
	
		KNI_StartHandles(1);
		KNI_DeclareHandle(nameObject);
		KNI_GetParameterAsObject(1, nameObject);
	
		
		nameLen = KNI_GetStringLength(nameObject);
		if (nameLen > MAX_NAME_LEN) {
			jvm_sprintf((char*)gKNIBuffer, 
				"Serial device name has wrong length: %d\n", nameLen);
			KNI_ThrowNew(KNIIllegalArgumentException, (char*)gKNIBuffer);
		} else {
			temp = (jchar*)szName;
			KNI_GetStringRegion(nameObject, 0, nameLen, temp);
			/* device names are in ASCII */
			for (i = 0; i < nameLen; i++) {
				szName[i] = (char)temp[i];
			}
			szName[nameLen] = 0;
	
			info = (SNIReentryData*)SNI_GetReentryData(NULL);
			if (info == NULL) {
				status = openPortByNameStart(szName, baud, flags, &hPort, &context);
			} else {
				/* reinvocation */
				hPort = info->descriptor;
				context = info->pContext;
				status = openPortByNameFinish(szName, baud, flags, &hPort, context);	
			}
	
			switch (status) {
				case PCSL_NET_SUCCESS:			
					break;
				case PCSL_NET_INTERRUPTED:			
					jvm_sprintf((char*)gKNIBuffer, 
						"Opening port %s has been interrupted\n", szName);
					KNI_ThrowNew(KNIInterruptedIOException, (char*)gKNIBuffer);
					break;
				case PCSL_NET_WOULDBLOCK:			
					SNIEVT_wait(COMM_OPEN_SIGNAL, hPort, context);
					break;
				default:	   
					jvm_sprintf((char*)gKNIBuffer,
						"Opening port %s was failed\n", szName);
					KNI_ThrowNew(KNIIOException, (char*)gKNIBuffer);
			}
		}
		KNI_EndHandles();
		KNI_ReturnInt((jint)hPort);
#else
		KNI_ReturnInt((jint)-1);
#endif
}

/**
 * Close a serial port.
 *
 * @param hPort handle to a native serial port
 *
 * @exception  IOException  if an I/O error occurs
 */
KNIEXPORT KNI_RETURNTYPE_VOID
    Java_com_sun_cldc_io_j2me_comm_Protocol_native_1close() {
#if ENABLE_PCSL
    int hPort = (int)KNI_GetParameterAsInt(1);

    closePort(hPort);
#endif	
    KNI_ReturnVoid();
}

/**
 * Native finalizer.
 */
KNIEXPORT KNI_RETURNTYPE_VOID
Java_com_sun_cldc_io_j2me_comm_Protocol_finalize()
{
#if ENABLE_PCSL

    int hPort;

    KNI_StartHandles(2);
    KNI_DeclareHandle(instance);
    KNI_DeclareHandle(clazz);
    KNI_GetThisPointer(instance);

    KNI_GetObjectClass(instance, clazz);
    hPort = KNI_GetIntField(instance, KNI_GetFieldID(clazz, "handle", "I"));

    if (hPort != -1) {
        closePort(hPort);
    }

    KNI_EndHandles();
#endif
    KNI_ReturnVoid();
}

/**
 * Read from a serial port without blocking.
 *
 * @param hPort handle to a native serial port
 * @param b I/O buffer
 * @param off starting offset for data
 * @param len length of data
 *
 * @return number of bytes read
 *
 * @exception  IOException  if an I/O error occurs
 */
KNIEXPORT KNI_RETURNTYPE_INT
    Java_com_sun_cldc_io_j2me_comm_Protocol_native_1readBytes() {
#if ENABLE_PCSL

    int  length = (int)KNI_GetParameterAsInt(4);
    int  offset = (int)KNI_GetParameterAsInt(3);
    int  hPort  = (int)KNI_GetParameterAsInt(1);
	jboolean blocking = KNI_GetParameterAsBoolean(5);
    int  bytesRead = -1;
    int status = PCSL_NET_IOERROR;
    void* context = NULL;
    SNIReentryData* info;

    KNI_StartHandles(1);
    KNI_DeclareHandle(bufferObject);
    KNI_GetParameterAsObject(2, bufferObject);

    info = (SNIReentryData*)SNI_GetReentryData(NULL);
    if (info == NULL) {

        if (hPort < 0) {
            jvm_sprintf((char*)gKNIBuffer,
                "Read from port: handle %d is invalid\n", hPort);
            KNI_ThrowNew(KNIIllegalArgumentException, (char*)gKNIBuffer);
        } else {   
            SNI_BEGIN_RAW_POINTERS;
            status = readFromPortStart(hPort, 
                (char*)SNI_GetRawArrayPointer(bufferObject)+offset,
                length, &bytesRead, &context);
            SNI_END_RAW_POINTERS;
        }    
    } else if (info->status >= 0) {
        /* reinvocation */
        hPort = info->descriptor;
        context = info->pContext;
        SNI_BEGIN_RAW_POINTERS;
        status = readFromPortFinish(hPort,
            (char*)SNI_GetRawArrayPointer(bufferObject)+offset,
             length, &bytesRead, context);
        SNI_END_RAW_POINTERS;
    } else if (info->status == -2) {
    	status = PCSL_NET_INTERRUPTED;
    }

    switch (status) {
        case PCSL_NET_SUCCESS:			
            /*do nothing and return normally */
            break;
        case PCSL_NET_INTERRUPTED:			
            jvm_sprintf((char*)gKNIBuffer,
                "Reading from port %d has been interrupted\n", hPort);
            KNI_ThrowNew(KNIInterruptedIOException, (char*)gKNIBuffer);
            break;
        case PCSL_NET_WOULDBLOCK:		
			if (blocking) {
            	SNIEVT_wait(COMM_READ_SIGNAL, hPort, context);
			} else {
				bytesRead = 0;
			}
            break;
        default:	   
            jvm_sprintf((char*)gKNIBuffer,
                "Reading from port %d was failed\n", hPort);
            KNI_ThrowNew(KNIIOException, (char*)gKNIBuffer);
    }

    KNI_EndHandles();
    KNI_ReturnInt((jint)bytesRead);
#else
	KNI_ReturnInt((jint)-1);
#endif
}

/**
 * Write to a serial port without blocking.
 *
 * @param hPort handle to a native serial port
 * @param b I/O buffer
 * @param off starting offset for data
 * @param len length of data
 *
 * @return number of bytes that were written
 *
 * @exception  IOException  if an I/O error occurs.
 */
KNIEXPORT KNI_RETURNTYPE_INT
    Java_com_sun_cldc_io_j2me_comm_Protocol_native_1writeBytes() {
#if ENABLE_PCSL

    int  length = (int)KNI_GetParameterAsInt(4);
    int  offset = (int)KNI_GetParameterAsInt(3);
    int  hPort  = (int)KNI_GetParameterAsInt(1);
    int   bytesWritten = 0;
    int status = PCSL_NET_IOERROR;
    void* context = NULL;
    SNIReentryData* info;

    KNI_StartHandles(1);
    KNI_DeclareHandle(bufferObject);
    KNI_GetParameterAsObject(2, bufferObject);

    info = (SNIReentryData*)SNI_GetReentryData(NULL);
    if (info == NULL) {

        if (hPort < 0) {
            jvm_sprintf((char*)gKNIBuffer,
                "Write to port: handle %d is invalid\n", hPort);
            KNI_ThrowNew(KNIIllegalArgumentException, (char*)gKNIBuffer);
        } else {        		
            SNI_BEGIN_RAW_POINTERS;
            status = writeToPortStart(hPort,
                (char*)SNI_GetRawArrayPointer(bufferObject)+offset,
                length, &bytesWritten, &context);
            SNI_END_RAW_POINTERS;
        }
    } else if (info->status >= 0) {
        /* reinvocation */
        hPort = info->descriptor;
        context = info->pContext;
        SNI_BEGIN_RAW_POINTERS;
        status = writeToPortFinish(hPort,
            (char*)SNI_GetRawArrayPointer(bufferObject)+offset,
            length, &bytesWritten, context);
        SNI_END_RAW_POINTERS;
    }

    switch (status) {
        case PCSL_NET_SUCCESS:			
            /*do nothing and return normally */
            break;
        case PCSL_NET_INTERRUPTED:			
            jvm_sprintf((char*)gKNIBuffer,
                "Writing to port %d has been interrupted\n", hPort);
            KNI_ThrowNew(KNIInterruptedIOException, (char*)gKNIBuffer);
            break;
        case PCSL_NET_WOULDBLOCK:			
            SNIEVT_wait(COMM_WRITE_SIGNAL, hPort, context);
            break;
        default:	   
            jvm_sprintf((char*)gKNIBuffer,
                "Writing to  port %d was failed\n", hPort);
            KNI_ThrowNew(KNIIOException, (char*)gKNIBuffer);
    }
    	    
    KNI_EndHandles();
    KNI_ReturnInt((jint)bytesWritten);
#else
	KNI_ReturnInt((jint)0);
#endif
}

