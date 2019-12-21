/*
 *
 *
 * Copyright  1990-2009 Sun Microsystems, Inc. All Rights Reserved.
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
#include "incls/_datagramProtocol.cpp.incl"

#if ENABLE_PCSL
extern "C" {
#include <pcsl_network.h>
#include <pcsl_memory.h>
#include <pcsl_string.h>
#include <josh_logging.h>
}

/**
 * Create pcsl_string from the specified Java String object.
 * The caller is responsible for freeing the created pcsl_string when done.
 *
 * @param java_str pointer to the Java String instance
 * @param pcsl_str pointer to the pcsl_string instance
 * @return status of the operation
 */
static pcsl_string_status jstring_to_pcsl_string(jstring java_str,
                           pcsl_string * pcsl_str) {
    if (pcsl_str == NULL) {
        return PCSL_STRING_EINVAL;
    }

    if (KNI_IsNullHandle(java_str)) {
        *pcsl_str = PCSL_STRING_NULL;
        return PCSL_STRING_OK;
    } else {
        const jsize length  = KNI_GetStringLength(java_str);
        if (length < 0) {
            *pcsl_str = PCSL_STRING_NULL;
            return PCSL_STRING_ERR;
        } else if (length == 0) {
            *pcsl_str = PCSL_STRING_EMPTY;
            return PCSL_STRING_OK;
        } else {
            jchar* buffer = (jchar*)pcsl_mem_malloc(length * sizeof(jchar));
            if (buffer == NULL) {
                *pcsl_str = PCSL_STRING_NULL;
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
#endif

/**
 * @file
 *
 * The default implementation of the native functions that are needed
 * for supporting the "datagram:" Generic Connection protocol.
 */

/**
 * Opens a datagram connection on the given port.
 * <p>
 * Java declaration:
 * <pre>
 *     open0(I[B)V
 * </pre>
 *
 * @param port port to listen on, or 0 to have one selected
 * @param suiteId the ID of the current MIDlet suite
 */
KNIEXPORT KNI_RETURNTYPE_VOID
Java_com_sun_cldc_io_j2me_datagram_Protocol_open0(void) {
#if ENABLE_PCSL

    int port;
    jint nativeHandle;
    jfieldID fieldId;
    jboolean tryOpen = KNI_TRUE;

    KNI_StartHandles(2);
    KNI_DeclareHandle(thisObject);
    KNI_DeclareHandle(thisClass);

    KNI_GetThisPointer(thisObject);
    KNI_GetObjectClass(thisObject, thisClass);

    port = (int)KNI_GetParameterAsInt(1);
    fieldId = KNI_GetFieldID(thisClass, "nativeHandle", "I");
    nativeHandle = KNI_GetIntField(thisObject, fieldId);

    if (nativeHandle != (jint)INVALID_HANDLE) {
        KNI_ThrowNew(KNIIOException, "already open");
        tryOpen = KNI_FALSE;
    }

    if (tryOpen) {
        SNIReentryData* info;
        int status = PCSL_NET_IOERROR;
        void *socketHandle;
        void *context;

        info = (SNIReentryData*)SNI_GetReentryData(NULL);
        if (info == NULL) {
            /* initial invocation */
            status = pcsl_datagram_open_start(port, &socketHandle,
                &context);
        } else if (info->status >= 0) {
            /* reinvocation */
            socketHandle = (void *)info->descriptor;
            context = info->pContext;
            status = pcsl_datagram_open_finish(socketHandle, context);
        }

        if (status == PCSL_NET_SUCCESS) {
            KNI_SetIntField(thisObject, fieldId, (jint)socketHandle);
        } else if (status == PCSL_NET_WOULDBLOCK) {
            SNIEVT_wait(NETWORK_WRITE_SIGNAL, (int)socketHandle,
                context);
        } else {
            /* status == PCSL_NET_IOERROR */
            jvm_sprintf((char*)gKNIBuffer,
                    "error code %d", pcsl_network_error(socketHandle));
            REPORT_INFO1(LC_PROTOCOL, "datagram::open0 %s", gKNIBuffer);
            KNI_ThrowNew(KNIIOException, (char*)gKNIBuffer);
        }
    }

    KNI_EndHandles();
#endif
    KNI_ReturnVoid();
}

/**
 * Sends a datagram.
 * <p>
 * Java declaration:
 * <pre>
 *     send0(II[BII)I
 * </pre>
 *
 * @param ipNumber raw IPv4 address of the remote host
 * @param port UDP port of the remote host
 * @param buf the data buffer to send
 * @param off the offset into the data buffer
 * @param len the length of the data in the buffer
 * @return number of bytes sent
 */
KNIEXPORT KNI_RETURNTYPE_INT
Java_com_sun_cldc_io_j2me_datagram_Protocol_send0(void) {
#if ENABLE_PCSL

    int ipAddress;
    int port;
    int offset;
    int length;
    int bytesSent = 0;
    void *socketHandle;
    jfieldID fieldId;
    SNIReentryData* info;
    unsigned char ipBytes[MAX_ADDR_LENGTH];

    KNI_StartHandles(3);

    KNI_DeclareHandle(bufferObject);
    KNI_DeclareHandle(thisObject);
    KNI_DeclareHandle(thisClass);
    KNI_GetThisPointer(thisObject);
    KNI_GetObjectClass(thisObject, thisClass);

    ipAddress = (int)KNI_GetParameterAsInt(1);
    port      = (int)KNI_GetParameterAsInt(2);
    KNI_GetParameterAsObject(3, bufferObject);
    offset    = (int)KNI_GetParameterAsInt(4);
    length    = (int)KNI_GetParameterAsInt(5);
    fieldId = KNI_GetFieldID(thisClass, "nativeHandle", "I");
    socketHandle = (void *)KNI_GetIntField(thisObject, fieldId);

    REPORT_INFO5(LC_PROTOCOL,
        "datagram::send0 off=%d len=%d port=%d ip=0x%x handle=0x%x",
        offset, length, port, ipAddress, (int)socketHandle);

    /* Convert ipAddress(integer) to ipBytes */
    memcpy(ipBytes, &ipAddress, sizeof(ipBytes));

    info = (SNIReentryData*)SNI_GetReentryData(NULL);

    if (socketHandle != INVALID_HANDLE) {
        int status = PCSL_NET_IOERROR;
        void *context;

        if (info == NULL) {
            /* initial invocation */
            SNI_BEGIN_RAW_POINTERS;
            status = pcsl_datagram_write_start(
                socketHandle, ipBytes, port,
                (char*)SNI_GetRawArrayPointer(bufferObject)+offset,
                length, &bytesSent, &context);
            SNI_END_RAW_POINTERS;
        } else if (info->status >= 0) {
            /* reinvocation */
            context = info->pContext;
            SNI_BEGIN_RAW_POINTERS;
            status = pcsl_datagram_write_finish(
                socketHandle, ipBytes, port,
                (char*)SNI_GetRawArrayPointer(bufferObject)+offset,
                length, &bytesSent, context);
            SNI_END_RAW_POINTERS;
        }

        if (status == PCSL_NET_SUCCESS) {
        } else if (status == PCSL_NET_WOULDBLOCK) {
            SNIEVT_wait(NETWORK_WRITE_SIGNAL, (int)socketHandle, context);
        } else if (status == PCSL_NET_INTERRUPTED) {
            KNI_ThrowNew(KNIInterruptedIOException, NULL);
        } else {
            /* status == PCSL_NET_IOERROR */
            jvm_sprintf((char*)gKNIBuffer,
                "error code %d", pcsl_network_error(socketHandle));
            KNI_ThrowNew(KNIIOException, (char*)gKNIBuffer);
        }
    } else {
        if (info == NULL) {
            /* initial invocation */
            KNI_ThrowNew(KNIIOException, "socket closed");
        } else {
            /* reinvocation */
            KNI_ThrowNew(KNIInterruptedIOException, NULL);
        }
    }

    KNI_EndHandles();
    KNI_ReturnInt((jint)bytesSent);
#else
    KNI_ReturnInt((jint)-1);
#endif
}

/**
 * Packs an IP address, port number, and the number of bytes received into a
 * jlong value suitable for returning from the receive0 native method.
 */
static jlong
pack_recv_retval(int ipAddress, int port, int bytesReceived) {
    return
        (((jlong)ipAddress) << 32) +
        (unsigned)((port & 0xFFFF) << 16) +
        (bytesReceived & 0xFFFF);
}

/**
 * Receives a datagram.
 * <p>
 * Java declaration:
 * <pre>
 *     receive0([BII)J
 * </pre>
 *
 * @param buf the data buffer
 * @param off the offset into the data buffer
 * @param len the length of the data in the buffer
 * @return The upper 32 bits contain the raw IPv4 address of the
 *         host the datagram was received from. The next 16 bits
 *         contain the port. The last 16 bits contain the number
 *         of bytes received.
 */
KNIEXPORT KNI_RETURNTYPE_LONG
Java_com_sun_cldc_io_j2me_datagram_Protocol_receive0(void) {
#if ENABLE_PCSL

    int offset;
    int length;
    void *socketHandle;
    jlong lres = 0;
    jfieldID fieldId;
    SNIReentryData* info;

    KNI_StartHandles(3);
    KNI_DeclareHandle(bufferObject);
    KNI_DeclareHandle(thisObject);
    KNI_DeclareHandle(thisClass);
    KNI_GetThisPointer(thisObject);
    KNI_GetObjectClass(thisObject, thisClass);

    KNI_GetParameterAsObject(1, bufferObject);
    offset = (int)KNI_GetParameterAsInt(2);
    length = (int)KNI_GetParameterAsInt(3);
    fieldId = KNI_GetFieldID(thisClass, "nativeHandle", "I");
    socketHandle = (void *)KNI_GetIntField(thisObject, fieldId);

    info = (SNIReentryData*)SNI_GetReentryData(NULL);

    if (socketHandle != INVALID_HANDLE) {
        int ipAddress;
        int port;
        int bytesReceived;
        int status = PCSL_NET_IOERROR;
        unsigned char ipBytes[MAX_ADDR_LENGTH];
        void *context;

        if (info == NULL) {
            /* initial invocation */
            SNI_BEGIN_RAW_POINTERS;
            status = pcsl_datagram_read_start(
                       socketHandle, ipBytes, &port,
                       (char*)SNI_GetRawArrayPointer(bufferObject)+offset,
                       length, &bytesReceived, &context);
            SNI_END_RAW_POINTERS;
        } else if (info->status >= 0) {
            /* reinvocation */
            context = info->pContext;
            SNI_BEGIN_RAW_POINTERS;
            status = pcsl_datagram_read_finish(
                       socketHandle, ipBytes, &port,
                       (char*)SNI_GetRawArrayPointer(bufferObject)+offset,
                       length, &bytesReceived, context);
            SNI_END_RAW_POINTERS;
        }

        if (status == PCSL_NET_SUCCESS) {
            memcpy(&ipAddress, ipBytes, MAX_ADDR_LENGTH);
            lres = pack_recv_retval(ipAddress, port, bytesReceived);
        } else if (status == PCSL_NET_WOULDBLOCK) {
            SNIEVT_wait(NETWORK_READ_SIGNAL, (int)socketHandle, context);
        } else if (status == PCSL_NET_INTERRUPTED) {
            KNI_ThrowNew(KNIInterruptedIOException, NULL);
        } else {
            /* status == PCSL_NET_IOERROR */
            jvm_sprintf((char*)gKNIBuffer,
                "error code %d", pcsl_network_error(socketHandle));
            KNI_ThrowNew(KNIIOException, (char*)gKNIBuffer);
        }
    } else {
        if (info == NULL) {
            /* initial invocation */
            KNI_ThrowNew(KNIIOException, "socket closed");
        } else {
            /* reinvocation */
            KNI_ThrowNew(KNIInterruptedIOException, NULL);
        }
    }

    KNI_EndHandles();
    KNI_ReturnLong(lres);
#else
    KNI_ReturnInt((jint)-1);
#endif
}

/**
 * Closes the datagram connection.
 * <p>
 * Java declaration:
 * <pre>
 *     close0(V)V
 * </pre>
 */
KNIEXPORT KNI_RETURNTYPE_VOID
Java_com_sun_cldc_io_j2me_datagram_Protocol_close0(void) {
#if ENABLE_PCSL
    void *socketHandle;
    jfieldID fieldId;
    SNIReentryData *info;

    KNI_StartHandles(2);
    KNI_DeclareHandle(thisObject);
    KNI_DeclareHandle(thisClass);
    KNI_GetThisPointer(thisObject);
    KNI_GetObjectClass(thisObject, thisClass);

    fieldId = KNI_GetFieldID(thisClass, "nativeHandle", "I");
    socketHandle = (void *)KNI_GetIntField(thisObject, fieldId);

    info = (SNIReentryData*)SNI_GetReentryData(NULL);

    if (socketHandle != INVALID_HANDLE) {
        int status = PCSL_NET_IOERROR;
        void *context = NULL;

        if (info == NULL) {
            /* first invocation */
            status = pcsl_datagram_close_start(socketHandle, &context);
            KNI_SetIntField(thisObject, fieldId, (jint)INVALID_HANDLE);

            SNIEVT_signal(NETWORK_READ_SIGNAL, (int)socketHandle, 0);
            SNIEVT_signal(NETWORK_WRITE_SIGNAL, (int)socketHandle, 0);
        } else if (info->status >= 0) {
            /* reinvocation */
            socketHandle = (void *)(info->descriptor);
            context = info->pContext;
            status = pcsl_datagram_close_finish(socketHandle, context);
        }

        if (status == PCSL_NET_WOULDBLOCK) {
            /* IMPL NOTE: unclear whether this is the right signal */
            SNIEVT_wait(NETWORK_READ_SIGNAL, (int)socketHandle,
                context);
        } else {
            /* PCSL_NET_SUCCESS or PCSL_NET_IOERROR */
            if (status != PCSL_NET_SUCCESS) {
                jvm_sprintf((char*)gKNIBuffer,
                        "error code %d", pcsl_network_error(socketHandle));
                    REPORT_INFO1(LC_PROTOCOL, "datagram::close %s",
                        gKNIBuffer);
                KNI_ThrowNew(KNIIOException, (char*)gKNIBuffer);
            }
        }
    } else {
        if (info == NULL) {
            /* first invocation */
            /* already closed, do nothing */
        } else {
            /* reinvocation */
        }
    }

    KNI_EndHandles();
#endif
    KNI_ReturnVoid();
}

/**
 * Get a hostname for the given raw IPv4 address.
 * <p>
 * Java declaration:
 * <pre>
 *     static addrToString(I)Ljava/lang/String;
 * </pre>
 *
 * @param ipn raw IPv4 address
 * @return The hostname or <tt>ipn</tt> as a dotted-quad if
 *         no hostname was found
 */
KNIEXPORT KNI_RETURNTYPE_OBJECT
Java_com_sun_cldc_io_j2me_datagram_Protocol_addrToString(void) {
#if ENABLE_PCSL
    jint ipn;
    jchar* result;
    int resultLen;
    int status;

    ipn = KNI_GetParameterAsInt(1);

    status = pcsl_network_addrToString((unsigned char*)&ipn, &result,
                                       &resultLen);
#endif

    KNI_StartHandles(1);
    KNI_DeclareHandle(resultObj);

#if ENABLE_PCSL
    if (PCSL_NET_SUCCESS == status) {
        KNI_NewString(result, (jsize)resultLen, resultObj);
        pcsl_mem_free(result);
    } else {
        KNI_ThrowNew(KNIOutOfMemoryError, NULL);
    }
#endif

    KNI_EndHandlesAndReturnObject(resultObj);
}

/**
 * Gets a raw IPv4 address for the given hostname.
 * <p>
 * Java declaration:
 * <pre>
 *     static getIpNumber([B)I
 * </pre>
 *
 * @param szHost the hostname to lookup as a 'C' string
 * @return raw IPv4 address or <tt>-1</tt> if there was an error
 */
KNIEXPORT KNI_RETURNTYPE_INT
Java_com_sun_cldc_io_j2me_datagram_Protocol_getIpNumber(void) {
#if ENABLE_PCSL
    int len;
    int status = PCSL_NET_IOERROR;
    int ipn = -1;
    unsigned char ipBytes[MAX_ADDR_LENGTH];
    void* context = NULL;
    void* handle;
    SNIReentryData* info;

    info = (SNIReentryData*)SNI_GetReentryData(NULL);

    if (info == NULL) {  /* First invocation */
        pcsl_string host;
        KNI_StartHandles(1);
        KNI_DeclareHandle(hostObj);
        KNI_GetParameterAsObject(1, hostObj);
        if (PCSL_STRING_OK != jstring_to_pcsl_string(hostObj, &host)) {
            KNI_ThrowNew(KNIOutOfMemoryError, NULL);
        } else {
            const jbyte * const host_bytes = pcsl_string_get_utf8_data(&host);
            status = pcsl_network_gethostbyname_start(
                   (char*)host_bytes,
                    ipBytes, MAX_ADDR_LENGTH, &len, &handle, &context);
            pcsl_string_release_utf8_data(host_bytes, &host);
        }
        pcsl_string_free(&host);
        KNI_EndHandles();
    } else if (info->status >= 0) {  /* Reinvocation after unblocking the thread */
        handle = (void*)info->descriptor;
        context = info->pContext;
        status = pcsl_network_gethostbyname_finish(ipBytes, MAX_ADDR_LENGTH,
                                                  &len, handle, context);
    }

    if (status == PCSL_NET_SUCCESS) {
        /*
         * Convert the unsigned char ip bytes array into an integer
         * that represents a raw IP address.
         */
        //ipn = pcsl_network_getRawIpNumber(ipBytes);
        memcpy(&ipn, ipBytes, MAX_ADDR_LENGTH);
    } else if (status == PCSL_NET_WOULDBLOCK) {
        SNIEVT_wait(HOST_NAME_LOOKUP_SIGNAL, (int)handle, context);
    } else {
        /* status is either PCSL_NET_IOERROR or PCSL_NET_INVALID */
        /* or (in the case of out-of-memory) PCSL_NET_NOSTATUS */
        ipn = -1;
        REPORT_INFO1(LC_PROTOCOL,
            "datagram::getIpNumber returns PCSL error code %d", status);
        /*
         * IOException is thrown at the Java layer when return value
         * is -1
         */
        //KNI_ThrowNew(KNIIOException, "Host name could not be resolved");
    }

    KNI_ReturnInt((jint)ipn);
#else
    KNI_ReturnInt((jint)-1);
#endif
}

/**
 * Get the maximum length of a datagram.
 * <p>
 * Java declaration:
 * <pre>
 *     getMaximumLength0(V)I
 * </pre>
 *
 * @return maximum length of a datagram
 */
KNIEXPORT KNI_RETURNTYPE_INT
Java_com_sun_cldc_io_j2me_datagram_Protocol_getMaximumLength0(void) {
#if ENABLE_PCSL
    void *socketHandle;
    jfieldID fieldId;
    int len = -1;

    KNI_StartHandles(2);
    KNI_DeclareHandle(thisObject);
    KNI_DeclareHandle(thisClass);
    KNI_GetThisPointer(thisObject);
    KNI_GetObjectClass(thisObject, thisClass);

    fieldId = KNI_GetFieldID(thisClass, "nativeHandle", "I");
    socketHandle = (void *)KNI_GetIntField(thisObject, fieldId);
    if (socketHandle != INVALID_HANDLE) {
        int status;

        /*
         * IMPL NOTE:
         * Option=3 represents SO_RCVBUF
         * The SO_RCVBUF option is used by the the network implementation
         * as a hint to size the underlying network I/O buffers.
         */
        status = pcsl_network_getsockopt(socketHandle, 3, &len);
        if (status != PCSL_NET_SUCCESS) {
            KNI_ThrowNew(KNIIOException, NULL);
        }
    } else {
        KNI_ThrowNew(KNIIOException, "socket closed");
    }

    REPORT_INFO2(LC_PROTOCOL,
        "datagram::getMaximumLength0 handle=%d len=%d %d\n",
        (int)socketHandle, len);

    KNI_EndHandles();
    KNI_ReturnInt((jint)len);
#else
    KNI_ReturnInt((jint)-1);
#endif
}

/**
 * Gets the nominal length of a datagram.
 * <p>
 * Java declaration:
 * <pre>
 *     getNominalLength0(V)I
 * </pre>
 *
 * @return nominal length of a datagram
 */
KNIEXPORT KNI_RETURNTYPE_INT
Java_com_sun_cldc_io_j2me_datagram_Protocol_getNominalLength0(void) {
#if ENABLE_PCSL
    /*
     * IMPL NOTE:
     * This implementation is identical to getMaximumLength0().
     * Is this useful? Should this just call getMaxiumumLength0()?
     */

    void *socketHandle;
    jfieldID fieldId;
    int len = -1;

    KNI_StartHandles(2);
    KNI_DeclareHandle(thisObject);
    KNI_DeclareHandle(thisClass);
    KNI_GetThisPointer(thisObject);
    KNI_GetObjectClass(thisObject, thisClass);

    fieldId = KNI_GetFieldID(thisClass, "nativeHandle", "I");
    socketHandle = (void *)KNI_GetIntField(thisObject, fieldId);
    if (socketHandle != INVALID_HANDLE) {
        int status;

        /*
         * IMPL NOTE:
         * Option=3 represents SO_RCVBUF
         * The SO_RCVBUF option is used by the the network implementation
         * as a hint to size the underlying network I/O buffers.
         */
        status = pcsl_network_getsockopt(socketHandle, 3, &len);
        if (status != PCSL_NET_SUCCESS) {
            KNI_ThrowNew(KNIIOException, NULL);
        }
    } else {
        KNI_ThrowNew(KNIIOException, "socket closed");
    }

    REPORT_INFO2(LC_PROTOCOL,
        "datagram::getNominalLength0 handle=%d len=%d\n",
        (int)socketHandle, len);

    KNI_EndHandles();
    KNI_ReturnInt((jint)len);
#else
    KNI_ReturnInt((jint)-1);
#endif
}

/**
 * Releases any native resources used by the datagram connection.
 * <p>
 * Java declaration:
 * <pre>
 *     finalize(V)V
 * </pre>
 */
KNIEXPORT KNI_RETURNTYPE_VOID
Java_com_sun_cldc_io_j2me_datagram_Protocol_finalize(void) {
#if ENABLE_PCSL
    void *handle;
    int status;
    void *context = NULL;
    jfieldID fieldId;

    KNI_StartHandles(2);
    KNI_DeclareHandle(thisObject);
    KNI_DeclareHandle(thisClass);
    KNI_GetThisPointer(thisObject);
    KNI_GetObjectClass(thisObject, thisClass);

    fieldId = KNI_GetFieldID(thisClass, "nativeHandle", "I");
    handle = (void *)KNI_GetIntField(thisObject, fieldId);
    if (handle != INVALID_HANDLE) {
        status = pcsl_datagram_close_start(handle, &context);
        if (status == PCSL_NET_SUCCESS) {
        } else if (status == PCSL_NET_IOERROR) {
            REPORT_INFO2(LC_PROTOCOL,
                "datagram::finalize handle 0x%x I/O error code %d",
                (int)handle, pcsl_network_error(handle));
        } else if (status == PCSL_NET_WOULDBLOCK) {
            REPORT_ERROR1(LC_PROTOCOL,
                "datagram::finalize handle 0x%x WOULDBLOCK not supported",
                (int)handle);
        }
        KNI_SetIntField(thisObject, fieldId, (jint)INVALID_HANDLE);
    }

    KNI_EndHandles();
#endif
    KNI_ReturnVoid();
}

/**
 * Gets the local IP number.
 * <p>
 * Java declaration:
 * <pre>
 *     static getHost0(V)Ljava/lang/String;
 * </pre>
 *
 * @return the local IP address as a dotted-quad <tt>String</tt>
 */
KNIEXPORT KNI_RETURNTYPE_OBJECT
Java_com_sun_cldc_io_j2me_datagram_Protocol_getHost0(void) {
#if ENABLE_PCSL
    char value[MAX_HOST_LENGTH];
    int status;
#endif

    KNI_StartHandles(1);
    KNI_DeclareHandle(result);

#if ENABLE_PCSL
    status = pcsl_network_getLocalIPAddressAsString(value);

    if ((status == PCSL_NET_SUCCESS) && (value != NULL)) {
        KNI_NewStringUTF(value, result);
    } else {
        KNI_ReleaseHandle(result);
    }
#endif

    KNI_EndHandlesAndReturnObject(result);
}

/**
 * Gets the local port number of a datagram connection.
 * <p>
 * Java declaration:
 * <pre>
 *     getPort0(V)I
 * </pre>
 *
 * @return the local port number of the given datagram connection
 */
KNIEXPORT KNI_RETURNTYPE_INT
Java_com_sun_cldc_io_j2me_datagram_Protocol_getPort0(void) {
#if ENABLE_PCSL
    void *socketHandle;
    int port = -1;
    jfieldID fieldId;

    KNI_StartHandles(2);
    KNI_DeclareHandle(thisObject);
    KNI_DeclareHandle(thisClass);
    KNI_GetThisPointer(thisObject);
    KNI_GetObjectClass(thisObject, thisClass);

    fieldId = KNI_GetFieldID(thisClass, "nativeHandle", "I");
    socketHandle = (void *)KNI_GetIntField(thisObject, fieldId);
    if (socketHandle != INVALID_HANDLE) {
        int status;
        status = pcsl_network_getlocalport(socketHandle, &port);
        if (status != PCSL_NET_SUCCESS) {
            KNI_ThrowNew(KNIIOException, NULL);
        }
    } else {
        KNI_ThrowNew(KNIIOException, "socket closed");
    }

    KNI_EndHandles();
    KNI_ReturnInt((jint)port);
#else
    KNI_ReturnInt((jint)-1);
#endif
}
