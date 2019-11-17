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
#include <javacall_canbus.h>
#endif

KNIEXPORT
KNI_RETURNTYPE_INT
Java_com_sun_cldc_io_j2me_can_Protocol_open0()
{
#if ENABLE_PCSL
	javacall_handle handle = (javacall_handle)-1;
	javacall_dio_result status;
	void* context = NULL;
	SNIReentryData* info;

	
	KNI_StartHandles(1);
		
	KNI_DeclareHandle(strDevName_object);
	KNI_GetParameterAsObject(1, strDevName_object);
	// hostname is always NUL terminated. See can/Protocol.java for detail.
	unsigned char *strDevName = (unsigned char*) SNI_GetRawArrayPointer(strDevName_object);

	int mode = KNI_GetParameterAsInt(2);

	info = (SNIReentryData*)SNI_GetReentryData(NULL);
	if (info == NULL) {   /* First invocation */
		status = javacall_dio_canbus_open_start(strDevName, mode, &handle, &context);
		if (status == JAVACALL_DIO_OK) {
			
		} else if (status == JAVACALL_DIO_WOULD_BLOCK) {
			SNIEVT_wait(NETWORK_WRITE_SIGNAL, (int)handle,
				context);
		} else {
			KNI_ThrowNew(KNIIOException, NULL);
		}
		
	} else {  /* Reinvocation after unblocking the thread */
		handle = (void *) info->descriptor;
		context = (void *)info->pContext;
		status = (javacall_dio_result)info->status;
	
		if (status == JAVACALL_DIO_OK) {
			status = javacall_dio_canbus_open_finish(handle, context);
		}

		if (status == JAVACALL_DIO_OK) {
			
		} else if (status == JAVACALL_DIO_WOULD_BLOCK) {
			SNIEVT_wait(NETWORK_WRITE_SIGNAL, (int)handle, context);
		} else	{			 
			KNI_ThrowNew(KNIConnectionNotFoundException, "CAN bus open error");
		}
	}
	KNI_EndHandles();
	KNI_ReturnInt(handle);
#else
	KNI_ReturnInt(-1);
#endif

}

