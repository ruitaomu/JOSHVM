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

#include "javacall_datagram.h"

javacall_result javacall_datagram_open(int port, javacall_handle *pHandle) {
    return JAVACALL_FAIL;
}

int javacall_datagram_recvfrom_start(
    javacall_handle handle,
    unsigned char *pAddress,
    int *port,
    char *buffer,
    int length,
    int *pBytesRead,
    void **pContext) {

    return JAVACALL_FAIL;
}

int javacall_datagram_recvfrom_finish(
    javacall_handle handle,
    unsigned char *pAddress,
    int *port,
    char *buffer,
    int length,
    int *pBytesRead,
    void *context) {

    return JAVACALL_FAIL;
}

int javacall_datagram_sendto_start(
    javacall_handle handle,
    unsigned char *pAddress,
    int port,
    char *buffer,
    int length,
    int *pBytesWritten,
    void **pContext) {

    return JAVACALL_FAIL;
}

int javacall_datagram_sendto_finish(
      javacall_handle handle,
    unsigned char *pAddress,
    int port,
    char *buffer,
    int length,
    int *pBytesWritten,
    void *context) {

    return JAVACALL_FAIL;
}

int javacall_datagram_close(javacall_handle handle) {
    return JAVACALL_FAIL;
}
