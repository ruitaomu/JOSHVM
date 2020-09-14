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
#include <kni.h>
#include <sni.h>
#include <sni_event.h>
#include <kni_globals.h>
#include <pcsl_network.h>
#include <pcsl_serversocket.h>

#include <josh_logging.h>

#ifndef NULL
#define NULL 0
#endif

/**
 * @file
 *
 * The default implementation of the native functions that are needed
 * for supporting the "serversocket:" Generic Connection protocol.
 */

/**
 * Opens a server socket connection on the given port.  If successful,
 * stores a handle directly into the nativeHandle field.  If unsuccessful,
 * throws an exception.
 * <p>
 * Java declaration:
 * <pre>
 *     open0(I[B)V
 * </pre>
 *
 * @param port       TCP port to listen for connections on
 * @param suiteId    ID of current midlet suite, or null if there
 *                   is no current suite
 *
 * @exception IOException  if some other kind of I/O error occurs
 * or if reserved by another suite
 */
KNIEXPORT KNI_RETURNTYPE_VOID
Java_com_sun_cldc_io_j2me_serversocket_Socket_open0(void) {
    int port;
    void *pcslHandle = INVALID_HANDLE;
    int status = PCSL_NET_INVALID;
	SNIReentryData* info;
	void* context = NULL;
	int res;
	jfieldID id;

    KNI_StartHandles(2);
    KNI_DeclareHandle(thisObject);
	KNI_DeclareHandle(thisClass);

    KNI_GetThisPointer(thisObject);
    port = (int)KNI_GetParameterAsInt(1);

	KNI_GetObjectClass(thisObject, thisClass);
	id = KNI_GetFieldID(thisClass, "nativeHandle", "I");
	KNI_SetIntField(thisObject, id, (jint)INVALID_HANDLE);

	
	int hasInited = 0;
	
	info = (SNIReentryData*)SNI_GetReentryData(NULL);
	if (info == NULL) {
		if (!hasInited) {
			res = pcsl_network_init_start(NULL);
			if (res == PCSL_NET_WOULDBLOCK) {
				SNIEVT_wait(NETWORK_UP_SIGNAL, (int)0, NULL);
			} else if (res == PCSL_NET_SUCCESS) {
				hasInited = 1;
			} else {
				KNI_ThrowNew(KNIIOException, "Initialize network error!\n");
			}
		}
		if (hasInited) {
			SNI_BEGIN_RAW_POINTERS;
			status = pcsl_serversocket_open(port, &pcslHandle);
			SNI_END_RAW_POINTERS;

			if (status == PCSL_NET_SUCCESS) {
				KNI_SetIntField(thisObject, id, (jint)pcslHandle);
		        REPORT_INFO2(LC_PROTOCOL,
                     "serversocket::open port = %d handle = %d\n",
                     port, (jint)pcslHandle);
			} else if (status == PCSL_NET_IOERROR) {
				KNI_ThrowNew(KNIIOException, "IOError in serversocket::open");
			} else {
				KNI_ThrowNew(KNIIOException, "Unknown error during serversocket::open");
			}
		}
	} else {
		SNIsignalType type = info->waitingFor;
		if (type == NETWORK_UP_SIGNAL) {
			if (info->status == 0) {
				res = pcsl_network_init_finish();
			} else {
				res = PCSL_NET_IOERROR;
			}
			if (res == PCSL_NET_WOULDBLOCK) {
				SNIEVT_wait(NETWORK_UP_SIGNAL, (int)0, NULL);
			} else if (res == PCSL_NET_SUCCESS) {
				
				{
					SNI_BEGIN_RAW_POINTERS;
					status = pcsl_serversocket_open(port, &pcslHandle);
					SNI_END_RAW_POINTERS;

					if (status == PCSL_NET_SUCCESS) {
						KNI_SetIntField(thisObject, id, (jint)pcslHandle);
		        		REPORT_INFO2(LC_PROTOCOL,
                   		  "serversocket::open port = %d handle = %d\n",
                     		port, (jint)pcslHandle);
					} else if (status == PCSL_NET_IOERROR) {
						KNI_ThrowNew(KNIIOException, "IOError in serversocket::open");
					} else {
						KNI_ThrowNew(KNIIOException, "Unknown error during serversocket::open");
					}
				}
			} else {
				KNI_ThrowNew(KNIIOException, "Initialize network error!\n");
			}
		}
	} //End of SNI reentry process

    KNI_EndHandles();
    KNI_ReturnVoid();
}

/**
 * Closes the connection.
 * <p>
 * Java declaration:
 * <pre>
 *     close0(V)V
 * </pre>
 *
 * @exception  IOException  if an I/O error occurs when closing the
 *                          connection
 */
KNIEXPORT KNI_RETURNTYPE_VOID
Java_com_sun_cldc_io_j2me_serversocket_Socket_close0(void) {
    int serverSocketHandle;
    int status = PCSL_NET_INVALID;
    void* context = NULL;
    SNIReentryData* info;
    jfieldID id;

    KNI_StartHandles(2);
    KNI_DeclareHandle(thisObject);
	KNI_DeclareHandle(thisClass);
	
    KNI_GetThisPointer(thisObject);
	KNI_GetObjectClass(thisObject, thisClass);
	id = KNI_GetFieldID(thisClass, "nativeHandle", "I");

    info = (SNIReentryData*)SNI_GetReentryData(NULL);
    if (info == NULL) {
        /* initial invocation */
        serverSocketHandle = KNI_GetIntField(thisObject, id);

        /*
         * If serverSocketHandle is invalid, the socket has already been closed,
         * so per specification we do nothing and return.
         */
        if (serverSocketHandle == (int)INVALID_HANDLE) {
            REPORT_INFO(LC_PROTOCOL, "serversocket::close Invalid handle\n");
        } else {
            status = pcsl_socket_close_start((void*)serverSocketHandle,
                                                 &context);
            
            /* Server socket should be monitored for read events only */
            SNIEVT_signal(NETWORK_READ_SIGNAL, serverSocketHandle, 0);
            KNI_SetIntField(thisObject, id, (jint)INVALID_HANDLE);
        }
    } else {
        /* reinvocation */
        serverSocketHandle = info->descriptor;
        context = info->pContext;
        status = pcsl_socket_close_finish((void*)serverSocketHandle, context);
    }

    //REPORT_INFO1(LC_PROTOCOL, "serversocket::close handle=%d\n", serverSocketHandle);

    if (serverSocketHandle != (int)INVALID_HANDLE) {
        if (status == PCSL_NET_SUCCESS) {
			REPORT_INFO1(LC_PROTOCOL, "serversocket::close = 0x%x OK\n",
                         serverSocketHandle);
        } else if (status == PCSL_NET_WOULDBLOCK) {
            REPORT_INFO1(LC_PROTOCOL, "serversocket::close = 0x%x blocked\n",
                         serverSocketHandle);
            /* IMPL NOTE: is this the right signal? */
            SNIEVT_wait(NETWORK_EXCEPTION_SIGNAL,
                             serverSocketHandle, context);
        } else {
            KNI_ThrowNew(KNIIOException, "IOError in serversocket::close");
        }
    }

    KNI_EndHandles();
    KNI_ReturnVoid();
}

/**
 * Polls a server socket for an incoming TCP connection. If there is an
 * incoming connection, it is accepted and this method returns true. If
 * there is no incoming connection, this method returns false immediately.
 * This requires callers to call this method repeatedly (often in a
 * busy-wait loop) while awaiting an incoming connection.
 *
 * @return new established socket handle if a connection was made, otherwise exception thrown
 *
 * @exception IOException if an I/O error has occurred
 */
KNIEXPORT KNI_RETURNTYPE_INT
Java_com_sun_cldc_io_j2me_serversocket_Socket_accept0(void) {
    int serverSocketHandle;
    SNIReentryData* info;
    void* connectionHandle = INVALID_HANDLE;
    int status = PCSL_NET_INVALID;
    int processStatus = KNI_FALSE;
    void *context = NULL;
	jfieldID idServerSocketHandle;

    KNI_StartHandles(2);
    KNI_DeclareHandle(thisObject);
    KNI_DeclareHandle(thisClass);
	
    KNI_GetThisPointer(thisObject);
    KNI_GetObjectClass(thisObject, thisClass);
	
	idServerSocketHandle = KNI_GetFieldID(thisClass, "nativeHandle", "I");

	serverSocketHandle = KNI_GetIntField(thisObject, idServerSocketHandle);

    if (serverSocketHandle == (int)INVALID_HANDLE) {
        KNI_ThrowNew(KNIIOException, "Socket was closed");
    } else {
        info = (SNIReentryData*)SNI_GetReentryData(NULL);
        if (info == NULL) {   /* First invocation */
            REPORT_INFO1(LC_PROTOCOL, "serversocket::accept handle=%d\n",
                         serverSocketHandle);

            status = pcsl_serversocket_accept_start(
                (void*)serverSocketHandle, &connectionHandle, &context);

            processStatus = KNI_TRUE;

        } else {  /* Reinvocation after unblocking the thread */
            if (info->descriptor != serverSocketHandle) {
                //jvm_snprintf(gKNIBuffer, KNI_BUFFER_SIZE,
                //              "serversocket::accept Handles mismatched 0x%x != 0x%x\n",
                //              serverSocketHandle,
                //              info->descriptor);
                REPORT_CRIT(LC_PROTOCOL, "serversocket::accept Handles mismatched!");
                KNI_ThrowNew(KNIIllegalStateException, "serversocket::accept Handles mismatched!");
            } else {
                status = pcsl_serversocket_accept_finish(
                        (void*)serverSocketHandle, &connectionHandle, &context);

                processStatus = KNI_TRUE;
            }
        }

        if (processStatus) {
            REPORT_INFO1(LC_PROTOCOL,
                         "serversocket::accept connection handle=%d\n",
                         connectionHandle);
            if (status == PCSL_NET_SUCCESS) {
				
            } else if (status == PCSL_NET_WOULDBLOCK) {
                SNIEVT_wait(NETWORK_READ_SIGNAL,
                                 serverSocketHandle, context);
            } else if (status == PCSL_NET_IOERROR) {
                KNI_ThrowNew(KNIIOException, "IOError in serversocket::accept");
            } else {
                REPORT_INFO(LC_PROTOCOL, "Unknown error during serversocket::accept");
                KNI_ThrowNew(KNIIOException, "Unknown error during serversocket::accept");
            }
        }

//        if (connectionHandle != INVALID_HANDLE) {
//            /*
//             * We got a valid connection, either by checking it out of the
//             * push registry, or by accepting an incoming connection from the
//             * platform.
//             */
//            KNI_SetIntField(socketObject, idSocketHandle, (jint)connectionHandle);
//        }
    }

    KNI_EndHandles();
    KNI_ReturnInt(connectionHandle);
}


/**
 * Releases any native resources used by the server socket connection.
 * <p>
 * Java declaration:
 * <pre>
 *     finalize(V)V
 * </pre>
 */
KNIEXPORT KNI_RETURNTYPE_VOID
Java_com_sun_cldc_io_j2me_serversocket_Socket_finalize(void) {
    int serverSocketHandle;
    void* context = NULL;
    int status = PCSL_NET_INVALID;
	jfieldID id;

    KNI_StartHandles(2);
    KNI_DeclareHandle(thisObject);
	KNI_DeclareHandle(thisClass);
	
    KNI_GetThisPointer(thisObject);
	KNI_GetObjectClass(thisObject, thisClass);
	id = KNI_GetFieldID(thisClass, "nativeHandle", "I");

	
    serverSocketHandle = KNI_GetIntField(thisObject, id);

    REPORT_INFO1(LC_PROTOCOL, "serversocket::finalize handle=%d",
        serverSocketHandle);

    if (serverSocketHandle != (int)INVALID_HANDLE) {
        status = pcsl_socket_close_start(
            (void*)serverSocketHandle, &context);

        if (status == PCSL_NET_IOERROR) {
            REPORT_ERROR1(LC_PROTOCOL, "IOError in serversocket::finalize error=%d\n",
                          pcsl_network_error((void*)serverSocketHandle));
        } else if (status == PCSL_NET_WOULDBLOCK) {
            /* blocking during finalize is not supported */
            REPORT_CRIT1(LC_PROTOCOL, "serversocket::finalize = 0x%x blocked\n",
                         serverSocketHandle);
        }
    
		KNI_SetIntField(thisObject, id, (jint)INVALID_HANDLE);
    }

    KNI_EndHandles();
    KNI_ReturnVoid();
}

/**
 * Gets the local IP number.
 * <p>
 * Java declaration:
 * <pre>
 *     getLocalAddress0(V)Ljava/lang/String;
 * </pre>
 *
 * @return the IP address as a dotted-quad <tt>String</tt>
 */
KNIEXPORT KNI_RETURNTYPE_OBJECT
Java_com_sun_cldc_io_j2me_serversocket_Socket_getLocalAddress0(void) {
    int serverSocketHandle;
    char value[MAX_HOST_LENGTH];
    int status;
	jfieldID id;

    KNI_StartHandles(3);
    KNI_DeclareHandle(thisObject);
    KNI_DeclareHandle(result);
	KNI_DeclareHandle(thisClass);
	
    KNI_GetThisPointer(thisObject);
	KNI_GetObjectClass(thisObject, thisClass);
	id = KNI_GetFieldID(thisClass, "nativeHandle", "I");

    serverSocketHandle = KNI_GetIntField(thisObject, id);

    if (serverSocketHandle != (int)INVALID_HANDLE) {
        status = pcsl_network_getLocalIPAddressAsString(value);

        if (status == PCSL_NET_SUCCESS) {
            KNI_NewStringUTF(value, result);
        } else {
            KNI_ReleaseHandle(result);
        }
    } else {
        KNI_ThrowNew(KNIIOException, "socket closed");
    }

    KNI_EndHandlesAndReturnObject(result);
}

/**
 * Gets the local port to which this socket connection is bound.
 * <p>
 * Java declaration:
 * <pre>
 *     getLocalPort0(V)I
 * </pre>
 *
 * @param handle the native handle to the network connection.
 *
 * @return the local port number for this socket connection
 */
KNIEXPORT KNI_RETURNTYPE_INT
Java_com_sun_cldc_io_j2me_serversocket_Socket_getLocalPort0(void) {
    int port = -1;
    int serverSocketHandle;
	
	jfieldID id;
	
	KNI_StartHandles(2);
	KNI_DeclareHandle(thisObject);
	KNI_DeclareHandle(thisClass);
	
	KNI_GetThisPointer(thisObject);
	KNI_GetObjectClass(thisObject, thisClass);
	id = KNI_GetFieldID(thisClass, "nativeHandle", "I");

	serverSocketHandle = KNI_GetIntField(thisObject, id);
	
    if (serverSocketHandle != (int)INVALID_HANDLE) {
        int status;

        /*
         * IMPL NOTE: even though there is currently no PCSL serversocket
         * implementation, it HAPPENS to work to call PCSL's getlocalport
         * because on Linux and Win32, the old porting layer and PCSL both use
         * a socket descriptor as the handle.
         */
        status = pcsl_network_getlocalport((void *) serverSocketHandle, &port);
        if (status != PCSL_NET_SUCCESS) {
            KNI_ThrowNew(KNIIOException, "I/O error");
        }
    } else {
        KNI_ThrowNew(KNIIOException, "socket closed");
    }

    KNI_EndHandles();
    KNI_ReturnInt((jint)port);
}
