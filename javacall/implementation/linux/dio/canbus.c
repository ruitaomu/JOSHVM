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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <linux/can.h>
#include <linux/can/raw.h>

#include <javacall_logging.h>
#include <javacall_canbus.h>

extern int GetFD(javacall_handle handle);
extern void SetFD(javacall_handle handle, int fd);
extern javacall_handle AllocHandle();
extern void FreeHandle(javacall_handle handle);

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
															/*OUT*/void** pContext) {
	
	int s;
	int nbytes, flags;
	struct sockaddr_can addr;
	struct can_frame frame;
	struct ifreq ifr;

	*handle = AllocHandle();
	if (!*handle) {
		return JAVACALL_DIO_FAIL;
	}

	if((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
		javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_DAAPI, "Error while opening socket");
		return JAVACALL_DIO_FAIL;
	}
	
	SetFD(*handle, s);

	//make socket non-blocking
	flags = fcntl(s, F_GETFL, 0); 
	if (-1 == fcntl(s, F_SETFL, flags | O_NONBLOCK)) {
		close_socket(handle);
		return JAVACALL_DIO_FAIL;
	}

	strncpy(ifr.ifr_name, device_name, IFNAMSIZ);
	ioctl(s, SIOCGIFINDEX, &ifr);
	
	addr.can_family  = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;

	javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_DAAPI, "%s at index %d, socket handle %d\n", device_name, ifr.ifr_ifindex, s);

	if(bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		javacall_logging_printf(JAVACALL_LOGGING_WARNING, JC_DAAPI, "Error in socket bind");
		close_socket(handle);
		return JAVACALL_DIO_FAIL;
	}	

	return JAVACALL_DIO_OK;
}

/**
 * Open CAN BUS device, invoked when operation finishes
 *
 * @param handle the pointer to hold the returned handle
 * @param pContext hold the returned context
 * @return <tt>JAVACALL_DIO_OK</tt> Open succeed
 *		   <tt>JAVACALL_DIO_FAIL</tt> or failed
 *         <tt>JAVACALL_DIO_WOULDBLOCK if the operation would block
 */
javacall_dio_result javacall_dio_canbus_open_finish(javacall_handle handle, void* pContext) {
	return JAVACALL_DIO_FAIL;
}
