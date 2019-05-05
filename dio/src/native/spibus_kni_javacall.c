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

#include <javacall_logging.h>
#include <javacall_spi.h>

#define SPI_DEBUG_OUTPUT 0

#if SPI_DEBUG_OUTPUT
#define DEBUG_PRINTF(x, ...) javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, LC_DAAPI, x, ...)
#else
#define DEBUG_PRINTF(x, ...)
#endif

KNIEXPORT
KNI_RETURNTYPE_INT
Java_com_joshvm_j2me_dio_spibus_SPISlaveImpl_openSPIDeviceByConfig0()
{
	jint deviceNumber, address, csActive, clockFrequency, clockMode, wordLen, bitOrdering;
	jboolean exclusive;
	javacall_handle handle;
	javacall_dio_result result;
	
	deviceNumber = KNI_GetParameterAsInt(1);
	address = KNI_GetParameterAsInt(2);
	csActive = KNI_GetParameterAsInt(3);
	clockFrequency = KNI_GetParameterAsInt(4);
	clockMode = KNI_GetParameterAsInt(5);
	wordLen = KNI_GetParameterAsInt(6);
	bitOrdering = KNI_GetParameterAsInt(7);
	exclusive = KNI_GetParameterAsBoolean(8);

	result = javacall_spi_open(deviceNumber, address,(javacall_spi_cs_active)csActive, 
								clockFrequency, clockMode, wordLen,
								(javacall_byteorder)bitOrdering, 
								(exclusive==KNI_FALSE)?JAVACALL_FALSE:JAVACALL_TRUE, 
								&handle);

	if (result == JAVACALL_DIO_INVALID_CONFIG) {
		KNI_ThrowNew(KNIInvalidDeviceConfigException, "Open SPI device error\n");
	} else if (result == JAVACALL_DIO_UNSUPPORTED_ACCESS_MODE) {
		KNI_ThrowNew(KNIUnsupportedAccessModeException, "Open SPI device error\n");
	} else if (result == JAVACALL_DIO_BUSY) {
		KNI_ThrowNew(KNIUnavailableDeviceException, "Open SPI device error\n");
	} else if (result == JAVACALL_DIO_NOT_FOUND) {
		KNI_ThrowNew(KNIDeviceNotFoundException, "Open SPI device error\n");
	} else if (result != JAVACALL_DIO_OK) {
		KNI_ThrowNew(KNIIOException, "Open SPI device error\n");
	}
	
	KNI_ReturnInt(handle);
}

KNIEXPORT
KNI_RETURNTYPE_VOID
Java_com_joshvm_j2me_dio_spibus_SPISlaveImpl_close0()
{
	int handle;
	
	handle = KNI_GetParameterAsInt(1);
	javacall_spi_close((javacall_handle)handle);

	KNI_ReturnVoid();
}

KNIEXPORT
KNI_RETURNTYPE_VOID
Java_com_joshvm_j2me_dio_spibus_SPISlaveImpl_begin0()
{
	int handle;
	int csActiveLevel;

	handle = KNI_GetParameterAsInt(1);
	csActiveLevel = KNI_GetParameterAsInt(2);

	if (csActiveLevel == DAAPI_SPI_CS_ACTIVE_LOW) {
		javacall_spi_set_cs((javacall_handle)handle, JAVACALL_FALSE);
	} else if (csActiveLevel == DAAPI_SPI_CS_ACTIVE_HIGH) {
		javacall_spi_set_cs((javacall_handle)handle, JAVACALL_TRUE);
	}
	
	KNI_ReturnVoid();
}

KNIEXPORT
KNI_RETURNTYPE_VOID
Java_com_joshvm_j2me_dio_spibus_SPISlaveImpl_end0()
{
	int handle;
	int csActiveLevel;

	handle = KNI_GetParameterAsInt(1);
	csActiveLevel = KNI_GetParameterAsInt(2);

	if (csActiveLevel == DAAPI_SPI_CS_ACTIVE_LOW) {
		javacall_spi_set_cs((javacall_handle)handle, JAVACALL_TRUE);
	} else if (csActiveLevel == DAAPI_SPI_CS_ACTIVE_HIGH) {
		javacall_spi_set_cs((javacall_handle)handle, JAVACALL_FALSE);
	}
	
	KNI_ReturnVoid();
}

KNIEXPORT
KNI_RETURNTYPE_INT
Java_com_joshvm_j2me_dio_spibus_SPISlaveImpl_writeAndRead0()
{
	int handle, src_offset, dst_offset, len;
	int bytesTransffered;

	handle = KNI_GetParameterAsInt(1);
	src_offset = KNI_GetParameterAsInt(3);
	dst_offset = KNI_GetParameterAsInt(5);
	len = KNI_GetParameterAsInt(6);

	DEBUG_PRINTF("[KNI]SPISlaveImpl_writeAndRead0: handle=%d, src_off=%d, dst_off=%d, len=%d\n", handle, src_offset, dst_offset, len);

	KNI_StartHandles(2);
  	{
		char* srcbuf;
		char* dstbuf;
		
	    KNI_DeclareHandle(src_buffer_object);
		KNI_DeclareHandle(dst_buffer_object);
	    KNI_GetParameterAsObject(2, src_buffer_object);
		KNI_GetParameterAsObject(4, dst_buffer_object);

		DEBUG_PRINTF("[KNI]SPISlaveImpl_writeAndRead0: src_buf len=%d, dst_buf len=%d\n", 
						KNI_GetArrayLength(src_buffer_object),
						KNI_GetArrayLength(dst_buffer_object));
		
	    srcbuf = (char *) SNI_GetRawArrayPointer(src_buffer_object) + src_offset;
		dstbuf = (char *) SNI_GetRawArrayPointer(dst_buffer_object) + dst_offset;

	    bytesTransffered = javacall_spi_send_and_receive_sync((javacall_handle)handle, srcbuf, dstbuf, len);
		if (bytesTransffered < 0) {
			KNI_ThrowNew(KNIIOException, "SPI device error\n");
		} else if (bytesTransffered == 0) {
			KNI_ThrowNew(KNIIOException, "SPI bus is busy\n");
		}
  	}
	
  	KNI_EndHandles();
	KNI_ReturnInt(bytesTransffered);
}

