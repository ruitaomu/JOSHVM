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
#include <kni_globals.h>
#include <sni.h>
#include <sni_event.h>

#if ENABLE_PCSL
#include "javacall_i2c.h"
#endif

KNIEXPORT
KNI_RETURNTYPE_INT
Java_com_joshvm_j2me_dio_i2cbus_I2CSlaveImpl_open0()
{
#if ENABLE_PCSL
    javacall_handle handle = (javacall_handle)-1;
    jfieldID field_id;
    jint deviceNumber = 0;
    jint address = 0;
    jint addressSize = 0;
    jint clockFrequency = 0;
    jboolean exclusive;
    javacall_dio_result result;

    KNI_StartHandles(3);
    KNI_DeclareHandle(cfgObj);
    KNI_DeclareHandle(cfgClassObj);
    KNI_GetParameterAsObject(1, cfgObj);
    KNI_GetObjectClass(cfgObj, cfgClassObj);
    exclusive = KNI_GetParameterAsBoolean(2);

    field_id = KNI_GetFieldID(cfgClassObj, "controllerNumber", "I");
    if (field_id) {
        deviceNumber = KNI_GetIntField(cfgObj, field_id);
    }
    field_id = KNI_GetFieldID(cfgClassObj, "address", "I");
    if (field_id) {
        address = KNI_GetIntField(cfgObj, field_id);
    }
    field_id = KNI_GetFieldID(cfgClassObj, "addressSize", "I");
    if (field_id) {
        addressSize = KNI_GetIntField(cfgObj, field_id);
    }
    field_id = KNI_GetFieldID(cfgClassObj, "clockFrequency", "I");
    if (field_id) {
        clockFrequency = KNI_GetIntField(cfgObj, field_id);
    }
    result = javacall_i2c_open(deviceNumber, address,
            addressSize, clockFrequency,
            (javacall_bool)exclusive,
            &handle);
    if (result == JAVACALL_DIO_INVALID_CONFIG) {
        KNI_ThrowNew(KNIInvalidDeviceConfigException, "Open I2C device error\n");
    } else if (result == JAVACALL_DIO_UNSUPPORTED_ACCESS_MODE) {
        KNI_ThrowNew(KNIUnsupportedAccessModeException, "Open I2C device error\n");
    } else if (result == JAVACALL_DIO_BUSY) {
        KNI_ThrowNew(KNIUnavailableDeviceException, "Open I2C device error\n");
    } else if (result == JAVACALL_DIO_NOT_FOUND) {
        KNI_ThrowNew(KNIDeviceNotFoundException, "Open I2C device error\n");
    } else if (result != JAVACALL_DIO_OK) {
        KNI_ThrowNew(KNIIOException, "Open I2C device error\n");
    }
    KNI_EndHandles();
    KNI_ReturnInt((int)handle);
#else
    KNI_ReturnInt(-1);
#endif
}

KNIEXPORT
KNI_RETURNTYPE_INT
Java_com_joshvm_j2me_dio_i2cbus_I2CSlaveImpl_transfer0()
{
#if ENABLE_PCSL
    javacall_int32 bytesTransferred = 0;
    int handle, offset, len, flag;
    jboolean write;

    javacall_dio_result result;

    KNI_StartHandles(1);
    KNI_DeclareHandle(buffer);

    handle = KNI_GetParameterAsInt(1);
    write = KNI_GetParameterAsBoolean(2);
    KNI_GetParameterAsObject(3, buffer);
    offset = KNI_GetParameterAsInt(4);
    len = KNI_GetParameterAsInt(5);
    flag = KNI_GetParameterAsInt(6);

    result = javacall_i2c_transfer_sync((javacall_handle)handle,
        (javacall_i2c_message_type)flag,
        (javacall_bool)write,
        (char *)SNI_GetRawArrayPointer(buffer) + offset,
        len, &bytesTransferred);
    if (result != JAVACALL_DIO_OK) {
        KNI_ThrowNew(KNIIOException, "i2c data transfer error\n");
    }

    KNI_EndHandles();
    KNI_ReturnInt((int)bytesTransferred);
#else
    KNI_ReturnInt(-1);
#endif
}

KNIEXPORT
KNI_RETURNTYPE_VOID
Java_com_joshvm_j2me_dio_i2cbus_I2CSlaveImpl_close0()
{
#if ENABLE_PCSL
    int handle;

    handle = KNI_GetParameterAsInt(1);
    javacall_i2c_close((javacall_handle)handle);
#endif
    KNI_ReturnVoid();
}
