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

#include "javacall_i2c.h"

javacall_dio_result javacall_i2c_open(javacall_int32 busNum,
        javacall_int32 devAddr, javacall_int32 addrSize,
        javacall_int32 clockFrequency,
        const javacall_bool exclusive,
        /*OUT*/javacall_handle* pHandle) {
    return JAVACALL_DIO_FAIL;
}

javacall_dio_result javacall_i2c_transfer_sync(const javacall_handle handle,
                                                const javacall_i2c_message_type type,
                                                const javacall_bool write,
                                                char* pData, int len,
                                                javacall_int32 *const pBytes) {

    return JAVACALL_DIO_FAIL;
}

void javacall_i2c_close(javacall_handle handle) {
}
