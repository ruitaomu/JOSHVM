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
#include <javacall_defs.h>
#include <javacall_dio.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file javacall_canbus.h
 * @ingroup DIO
 * @brief Javacall interfaces for CAN BUS
 */

/**
 * Open CAN BUS device, invoked when operation starts
 * @param device_name the name of the platform CAN Bus controller
 * 
 * @param mode open mode
 * @param handle the pointer to hold the returned handle
 * @param pContext hold the returned context
 * @return <tt>JAVACALL_DIO_OK</tt> Open succeed
 *		   <tt>JAVACALL_DIO_FAIL</tt> or failed
 *         <tt>JAVACALL_DIO_WOULDBLOCK if the operation would block
 */
javacall_dio_result javacall_dio_canbus_open_start(unsigned char* device_name, 
															int mode, 
															/*OUT*/javacall_handle* handle, 
															/*OUT*/void** pContext);

/**
 * Open CAN BUS device, invoked when operation finishes
 *
 * @param handle the pointer to hold the returned handle
 * @param pContext hold the returned context
 * @return <tt>JAVACALL_DIO_OK</tt> Open succeed
 *		   <tt>JAVACALL_DIO_FAIL</tt> or failed
 *         <tt>JAVACALL_DIO_WOULDBLOCK if the operation would block
 */
javacall_dio_result javacall_dio_canbus_open_finish(javacall_handle handle, void* pContext);

/** @} */

#ifdef __cplusplus
}
#endif


