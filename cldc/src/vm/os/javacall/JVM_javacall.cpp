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

/*
 *
 *
 * Copyright  1990-2009 Sun Microsystems, Inc. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version
 * 2 only, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License version 2 for more details (a copy is
 * included at /legal/license.txt).
 *
 * You should have received a copy of the GNU General Public License
 * version 2 along with this work; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 * Please contact Sun Microsystems, Inc., 4150 Network Circle, Santa
 * Clara, CA 95054 or visit www.sun.com if you need additional
 * information or have any questions.
 */

/*
 * JVM_javacall.cpp: Generic-specific VM startup and
 *                  shutdown routines.
 *
 * This file provides Generic-specific virtual machine
 * startup and shutdown routines.  Refer to file
 * "/src/vm/share/runtime/JVM.hpp" and the Porting
 * Guide for details.
 */

#include "incls/_precompiled.incl"
#include "incls/_JVM_javacall.cpp.incl"

#if defined(UNDER_CE)
int _system_address = 0;
#endif

#if ENABLE_PCSL
extern "C" {
#include <pcsl_memory.h>
#include <pcsl_print.h>
#include <pcsl_network.h>
#include <pcsl_file.h>
}
#endif

static int executeVM( void ) {
  const int result = JVM::start();
  Arguments::finalize();
  return result;
}

#include "../utilities/JVM_Malloc.hpp"
#include "../../share/natives/sni_event.h"
extern "C" {
#include <javacall_events.h>
#include <javacall_logging.h>
#include <javacall_properties.h>
#include <javacall_file.h>
#ifdef ENABLE_DIRECTUI
#include <javacall_directui.h>
#endif
#ifdef ENABLE_JSR_120
#include <javacall_sms.h>
#endif
#ifdef ENABLE_DIO
#include <javacall_gpio.h>
#endif
#ifdef ENABLE_CELLULAR
#include <javacall_cellular_network.h>
#endif
#include <josh_logging.h>
}

//#define RUN_JAVACALL_NATIVE_TEST 1

#if ENABLE_JAVACALL_NATIVE_TEST
extern "C" {
	void StartJavacallTest();
}
#else
#define RUN_JAVACALL_NATIVE_TEST 0
#endif

#if ENABLE_PCSL
	#define internal_startup_mainclass "com.sun.cldchi.test.HelloWorld"
#else
	#define internal_startup_mainclass  ""
#endif

void JVMSPI_PrintRaw(const char* s) {
  /* Print the string to the standard output device */
#if ENABLE_PCSL
  pcsl_print(s);
#else
  javacall_print(s);
#endif
}

void JVMSPI_Exit(int code) {
  /* Terminate the current process */
  return ;
}

/*
 * This function is called by the VM periodically. It has to check if
 * any of the blocked threads are ready for execution, and call
 * SNI_UnblockThread() on those threads that are ready.
 *
 * Values for the <timeout> paramater:
 *  >0 = Block until an event happens, or until <timeout> milliseconds
 *       has elapsed.
 *   0 = Check the events sources but do not block. Return to the
 *       caller immediately regardless of the status of the event sources.
 *  -1 = Do not timeout. Block until an event happens.
 */
void JVMSPI_CheckEvents(JVMSPI_BlockedThreadInfo * blocked_threads,
                        int blocked_threads_count, jlong timeout_ms) {
  SNIReentryData rd;
  javacall_result res;
  int outEventLen;

  res = javacall_event_receive ((long)timeout_ms, (unsigned char*)&rd, sizeof(SNIReentryData), &outEventLen);

  if (!JAVACALL_SUCCEEDED(res)) {
    return;
  }

  SNIEVT_signal_list(blocked_threads, blocked_threads_count, rd.waitingFor, rd.descriptor, rd.status);
}


extern "C" int JVM_Start(const JvmPathChar *classpath, char *main_class, int argc,
                         char **argv) {
  JVM::set_arguments(classpath, main_class, argc, argv);
  return executeVM();
}

extern "C" int JVM_Start2(const JvmPathChar *classpath, char *main_class, int argc,
                          jchar **u_argv) {
  JVM::set_arguments2(classpath, main_class, argc, NULL, u_argv, true);
  return executeVM();
}

static int JVM_Task(int argc, char **argv) {

  //int   size = 0x00200000;
  int code;
#if ENABLE_PCSL
  pcsl_mem_initialize(NULL, -1);
#endif

  JVM_Initialize();
  javacall_events_init();

#if ENABLE_PCSL
	  pcsl_file_init();
#else
	  javacall_file_init();
#endif

  javacall_initialize_configurations();

  JVMSPI_setJAMSPath();

#ifdef ENABLE_JSR_120
  javacall_sms_init();
#endif
#ifdef ENABLE_CELLULAR
  javacall_cellular_init();
#endif
#ifdef ENABLE_DIO
  javacall_gpio_init();
#endif
#ifdef ENABLE_DIRECTUI
	javacall_directui_init();
#endif

  argc --;
  argv ++;

  while (true) {
    int n = JVM_ParseOneArg(argc, argv);
    if (n < 0) {
      //javacall_printf("Unknown argument: %s\n", argv[0]);
      JVMSPI_DisplayUsage(NULL);
      code = -1;
      goto end;
    } else if (n == 0) {
      break;
    }
    argc -= n;
    argv += n;
  }

#if 0

  if (JVM_GetConfig(JVM_CONFIG_SLAVE_MODE) == KNI_FALSE) {
    // Run the VM in regular mode -- JVM_Start won't return until
    // the VM completes execution.
    code = JVM_Start(NULL, NULL, argc, argv);
  } else {
    // Run the VM in slave mode -- we keep calling JVM_TimeSlice(),
    // which executes bytecodes for a small amount and returns. This
    // mode is necessary for platforms that need to keep the main
    // control loop outside of of the VM.

    JVM_Start(NULL, NULL, argc, argv);

    for (;;) {
      jlong timeout = JVM_TimeSlice();
      if (timeout <= -2) {
        break;
      } else {
        int blocked_threads_count;
        JVMSPI_BlockedThreadInfo * blocked_threads;

        blocked_threads = SNI_GetBlockedThreads(&blocked_threads_count);
        JVMSPI_CheckEvents(blocked_threads, blocked_threads_count, timeout);
      }
    }

    code = JVM_CleanUp();
  }
#else
	code = JVM_Start(NULL, NULL, argc, argv);
#endif

#ifdef ENABLE_DIRECTUI
	javacall_directui_finalize();
#endif
#ifdef ENABLE_JSR_120
	  javacall_sms_deinit();
#endif
#ifdef ENABLE_CELLULAR
	  javacall_cellular_deinit();
#endif
#ifdef ENABLE_DIO
	javacall_gpio_deinit();
#endif

#if ENABLE_PCSL
	  pcsl_file_finalize();
#else
	  javacall_file_finalize();
#endif

	javacall_events_finalize();
	javacall_finalize_configurations();
	

end:
#if ENABLE_PCSL
  pcsl_mem_finalize();
#endif

  return code;
}

#ifndef JAVACALL_JAVATASK_STARTUP_PARAM
#define JAVACALL_JAVATASK_STARTUP_PARAM "=HeapMin81920", "=HeapCapacity81920", "-int", \
 	"com.sun.cldchi.test.HelloWorld"
#endif

extern "C" void JavaTask(void) {
#if ENABLE_JAVACALL_NATIVE_TEST && RUN_JAVACALL_NATIVE_TEST
	StartJavacallTest();
#else
static char* args[] = {
  "JavaTask",
  JAVACALL_JAVATASK_STARTUP_PARAM
};

	JVM_Task(sizeof(args)/sizeof(char*), args);
#endif
}

/**
 * STUBs of javanotify functions. Temp
 **/
#include "javacall_socket.h"

void javanotify_socket_event(
							  javacall_socket_event_type type,
							  javacall_handle socket_handle,
							  javacall_result operation_result) {
	SNIReentryData rd;

	if (operation_result == JAVACALL_OK) {
		rd.status = PCSL_NET_SUCCESS;
	} else {
		rd.status = PCSL_NET_IOERROR;
	}

	rd.descriptor = (int)socket_handle;

	switch (type) {
		case JAVACALL_EVENT_SOCKET_CONNECT_COMPLETED:
		case JAVACALL_EVENT_SOCKET_SEND:
			rd.waitingFor= NETWORK_WRITE_SIGNAL;
			break;
		case JAVACALL_EVENT_SOCKET_RECEIVE:
			rd.waitingFor = NETWORK_READ_SIGNAL;
			break;
		case JAVACALL_EVENT_NETWORK_GETHOSTBYNAME_COMPLETED:
			rd.waitingFor = HOST_NAME_LOOKUP_SIGNAL;
			break;
		case JAVACALL_EVENT_SOCKET_CLOSE_COMPLETED:
			/**
			 * Wakeup possiblely pending Java thread, regardless read/write/lookup signal
			 **/
			rd.status = PCSL_NET_INTERRUPTED;
			rd.waitingFor = NETWORK_READ_SIGNAL;
			javacall_event_send((unsigned char*)&rd, sizeof(SNIReentryData));
			rd.waitingFor = NETWORK_WRITE_SIGNAL;
			javacall_event_send((unsigned char*)&rd, sizeof(SNIReentryData));
			rd.waitingFor = HOST_NAME_LOOKUP_SIGNAL;
			javacall_event_send((unsigned char*)&rd, sizeof(SNIReentryData));
			return;
		default:
			/* IMPL_NOTE: decide what to do */
			return;
			/* do not send event to java */
			/* IMPL_NOTE: NETWORK_EXCEPTION_SIGNAL is not assigned by any indication */
			REPORT_WARN(LC_CORE, "Uncaptured Network Exception Signal\n");
		}

	javacall_event_send((unsigned char*)&rd, sizeof(SNIReentryData));
}

void javanotify_server_socket_event(
                             javacall_socket_event_type type,
                             javacall_handle socket_handle,
                             javacall_handle new_socket_handle,
                             javacall_result operation_result) {

}

#include "javacall_network.h"

extern "C" {
	void notify_network_down();
}

void javanotify_network_event(javacall_network_event event, javacall_bool success) {
	SNIReentryData rd;

	rd.status = ((success != JAVACALL_FALSE)?0:-1);
	rd.descriptor = (int)0;
	rd.pContext = (void*)0;

	switch (event) {
		case JAVACALL_NETWORK_UP: rd.waitingFor = NETWORK_UP_SIGNAL;
		break;
		case JAVACALL_NETWORK_DOWN:
			rd.waitingFor = NETWORK_DOWN_SIGNAL;
			notify_network_down();
		break;
		case JAVACALL_NETWORK_DOWN_REQUEST:
		break;
		default: return;
	}

	javacall_event_send((unsigned char*)&rd, sizeof(SNIReentryData));
}

#include "javacall_datagram.h"

void javanotify_datagram_event(
                             javacall_datagram_event_type type,
                             javacall_handle handle,
                             javacall_result operation_result) {

}

/*
#include "javacall_cbs.h"

void javanotify_incoming_cbs(
									 javacall_cbs_encoding	msgType,
									 unsigned short 		msgID,
									 unsigned char* 		msgBuffer,
									 int					msgBufferLen) {}

*/

extern "C" void javanotify_gpio_port_value_changed(int h, int val ){

}

extern "C" void javanotify_gpio_pin_value_changed(const javacall_handle h, const int val){
	SNIReentryData rd;

	rd.status = val;
	rd.descriptor = (int)h;
	rd.pContext = 0;
	rd.waitingFor = GPIO_PIN_INPUT_SIGNAL;

	javacall_event_send((unsigned char*)&rd, sizeof(SNIReentryData));
}

extern "C" void javanotify_gpio_pin_closed(const javacall_handle h) {
	SNIReentryData rd;

	rd.status = 0;
	rd.descriptor = (int)h;
	rd.pContext = (void*)-1;
	rd.waitingFor = GPIO_PIN_INPUT_SIGNAL;

	javacall_event_send((unsigned char*)&rd, sizeof(SNIReentryData));
}

#if ENABLE_DIRECTUI
#include "javacall_directui.h"

extern "C" void javanotify_key_event(javacall_keypress_code key,
        javacall_keypress_type type) {
    SNIReentryData rd;

    rd.status = JAVACALL_OK;
    rd.descriptor = (int)key;
    rd.pContext = (void*)type;
    rd.waitingFor = KEY_SIGNAL;

    javacall_event_send((unsigned char*)&rd, sizeof(SNIReentryData));
}
#endif


#if ENABLE_SECURITY_NATIVE_RSA_SIGNATURE
#include "javacall_security.h"

extern "C" void javanotify_security_event(void* handle) {
    SNIReentryData rd;

    rd.status = 0;
    rd.descriptor = (int)handle;
    rd.pContext = (void*)0;
    rd.waitingFor = BN_CALC_COMP_SIGNAL;
    javacall_event_send((unsigned char*)&rd, sizeof(SNIReentryData));
}
#endif


