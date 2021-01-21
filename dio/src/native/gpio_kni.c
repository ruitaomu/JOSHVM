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
#include <javacall_logging.h>
#include <javacall_gpio.h>
#endif


KNIEXPORT
KNI_RETURNTYPE_VOID
Java_com_joshvm_j2me_dio_gpio_GPIOPinDevice_close0()
{
#if ENABLE_PCSL
	int handle;
	handle = KNI_GetParameterAsInt(1);

	if (javacall_gpio_pin_close((javacall_handle)handle) != JAVACALL_DIO_OK) {
           KNI_ThrowNew(KNIIOException, "Error when close GPIO pin\n");
	}

	javanotify_gpio_pin_closed((javacall_handle)handle);
#endif

	KNI_ReturnVoid();
}

KNIEXPORT
KNI_RETURNTYPE_BOOLEAN
Java_com_joshvm_j2me_dio_gpio_GPIOPinDevice_getValue0()
{
	jboolean value = KNI_FALSE;

#if ENABLE_PCSL
		int handle;

		handle = KNI_GetParameterAsInt(1);

		if (javacall_gpio_pin_read((javacall_handle)handle,(javacall_bool*)&value) != JAVACALL_DIO_OK) {
            KNI_ThrowNew(KNIIOException, "Reading from GPIO pin has failed\n");
		}

#endif
		KNI_ReturnBoolean(value);

}

KNIEXPORT
KNI_RETURNTYPE_INT
Java_com_joshvm_j2me_dio_gpio_GPIOPinDevice_open0()
{
#if ENABLE_PCSL

	javacall_handle h;
	javacall_dio_result result;
	jint port, pin, dir, mode, trigger;
	port = KNI_GetParameterAsInt(1); 
	pin = KNI_GetParameterAsInt(2);
	dir = KNI_GetParameterAsInt(3);
	mode = KNI_GetParameterAsInt(4);
	trigger = KNI_GetParameterAsInt(5);
	result = javacall_gpio_pin_open(port, pin, (javacall_gpio_dir)dir,
											(javacall_gpio_mode)mode,
											(javacall_gpio_trigger_mode)trigger,
											JAVACALL_FALSE, JAVACALL_TRUE, &h);
	if (JAVACALL_DIO_OK == result) {
		KNI_ReturnInt(h);
	} else if (JAVACALL_DIO_NOT_FOUND == result) {
		KNI_ThrowNew(KNIDeviceNotFoundException, "GPIO pin not found\n");
	} else if (JAVACALL_DIO_BUSY == result) {
		KNI_ThrowNew(KNIDeviceNotFoundException, "GPIO pin is busy\n");
	} else {
		KNI_ThrowNew(KNIIOException, "Open GPIO pin has failed\n");
	}
#endif
	KNI_ReturnInt(-1);
}

KNIEXPORT
KNI_RETURNTYPE_VOID
Java_com_joshvm_j2me_dio_gpio_GPIOPinDevice_setDirection0()
{
#if ENABLE_PCSL
		int handle;
		int value;
		javacall_dio_result result;

		handle = KNI_GetParameterAsInt(1);
		value = KNI_GetParameterAsInt(2);

		result = javacall_gpio_pin_direction_set((javacall_handle)handle,
											(javacall_bool)value?JAVACALL_TRUE:JAVACALL_FALSE);

		if (JAVACALL_DIO_UNSUPPORTED_OPERATION == result) {
			KNI_ThrowNew(KNIIOException, "Set GPIO pin direction into unsupported mode\n");
		} else if (JAVACALL_DIO_OK != result) {
			KNI_ThrowNew(KNIIOException, "Set GPIO pin direction has failed\n");
		}

#endif
	KNI_ReturnVoid();
}

KNIEXPORT
KNI_RETURNTYPE_VOID
Java_com_joshvm_j2me_dio_gpio_GPIOPinDevice_setTrigger0()
{
#if ENABLE_PCSL
		int handle;
		int trigger;

		handle = KNI_GetParameterAsInt(1);
		trigger = KNI_GetParameterAsInt(2);

		if (JAVACALL_DIO_OK != javacall_gpio_pin_set_trigger((javacall_handle)handle, (javacall_gpio_trigger_mode)trigger)) {
			KNI_ThrowNew(KNIIOException, "Set GPIO pin trigger mode has failed\n");
		}

#endif
	KNI_ReturnVoid();
}

KNIEXPORT
KNI_RETURNTYPE_VOID
Java_com_joshvm_j2me_dio_gpio_GPIOPinDevice_startEventNotification0()
{
#if ENABLE_PCSL
	int handle;
	
	handle = KNI_GetParameterAsInt(1);


	
	if (javacall_gpio_pin_notification_start((javacall_handle)handle) != JAVACALL_DIO_OK) {
		KNI_ThrowNew(KNIIOException, "Start GPIO pin notification has failed\n");
	}
#endif

	KNI_ReturnVoid();
}

KNIEXPORT
KNI_RETURNTYPE_VOID
Java_com_joshvm_j2me_dio_gpio_GPIOPinDevice_stopEventNotification0()
{
#if ENABLE_PCSL
		int handle;
		javacall_dio_result res;

		handle = KNI_GetParameterAsInt(1);

		res = javacall_gpio_pin_notification_stop((javacall_handle)handle);

		if (res != JAVACALL_DIO_OK) {
			KNI_ThrowNew(KNIIOException, "Stop GPIO pin notification has failed\n");
		}

#endif

		KNI_ReturnVoid();

}

KNIEXPORT
KNI_RETURNTYPE_VOID
Java_com_joshvm_j2me_dio_gpio_GPIOPinDevice_setValue0()
{
#if ENABLE_PCSL
	int handle;
	jboolean value;

	handle = KNI_GetParameterAsInt(1);
	value = KNI_GetParameterAsBoolean(2);

	javacall_gpio_pin_write((javacall_handle)handle,(javacall_bool)value);

#endif
	KNI_ReturnVoid();
}

KNIEXPORT
KNI_RETURNTYPE_INT
Java_com_joshvm_j2me_dio_gpio_PinListenerThread_waitingForGPIOPinEvent()
{
	int handle = -1;
	int value = 0;
#if ENABLE_PCSL
	jfieldID newvalue_field_id;

	KNI_StartHandles(2);
	KNI_DeclareHandle(thisPointer);
	KNI_DeclareHandle(thisClass);

	KNI_GetThisPointer(thisPointer);
	KNI_GetObjectClass(thisPointer, thisClass);


	SNIReentryData* info = (SNIReentryData*)SNI_GetReentryData((int*)NULL);
	if (info != NULL) {
		value = info->status;
		if (info->pContext) {
			handle = -1;
		} else {
			handle = info->descriptor;
			newvalue_field_id = KNI_GetFieldID(thisClass, "newValue", "Z");
			if (newvalue_field_id) {
				if (value == -1) {
					javacall_bool port_val;
					if (JAVACALL_DIO_OK == javacall_gpio_pin_read((javacall_handle)handle, &port_val)) {
						KNI_SetBooleanField(thisPointer, newvalue_field_id, port_val?1:0);
					} else {
						SNIEVT_wait(GPIO_PIN_INPUT_SIGNAL, (int)handle, (void*)NULL);
					}
				} else {
					KNI_SetBooleanField(thisPointer, newvalue_field_id, value?1:0);
				}
			}
		}

	} else {
		SNIEVT_wait(GPIO_PIN_INPUT_SIGNAL, (int)handle, (void*)NULL);
	}
	KNI_EndHandles();
#endif

	KNI_ReturnInt(handle);
}
