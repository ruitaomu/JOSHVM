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
#include <kni.h>

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
    KNI_ReturnInt(-1);
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
    KNI_StartHandles(1);
    KNI_DeclareHandle(result);
	
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
    KNI_ReturnInt((jint)-1);
}
