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

#include <javacall_spi.h>

javacall_dio_result javacall_spi_open(javacall_int32 busNumber,
        javacall_int32 address, javacall_spi_cs_active csActive, javacall_int32 clockFrequency,
        javacall_int32 clockMode, javacall_int32 wordLength,
        javacall_byteorder bitOrdering,
        const javacall_bool exclusive,
        /*OUT*/javacall_handle* pHandle) {
	return JAVACALL_DIO_FAIL;

}

void javacall_spi_close(javacall_handle handle) {
	return;
}

javacall_dio_result javacall_spi_set_cs(javacall_handle handle, javacall_bool value) {
	return JAVACALL_DIO_OK;
}

int javacall_spi_send_and_receive_sync(javacall_handle handle,const char* pTxBuf, /*OUT*/char* pRxBuf, const int len) {
	return -1;
}

