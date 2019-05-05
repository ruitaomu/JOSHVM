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
#include <javacall_logging.h>
#include <javacall_events.h>
#include <javacall_memory.h>
#include <javacall_time.h>
#if ENABLE_JSR_120
#include <javacall_sms.h>
#endif
#include <javacall_socket.h>
#if ENABLE_JSR_75
#include <javacall_fileconnection.h>
#endif
#include <javautil_string.h>
#include <javacall_serial.h>
#include <javacall_file.h>
#include "string.h"

#define MSG_BUF_LEN 512
javacall_handle handle1, handle2;
long timer_count1 = 0;
long timer_count2 = 0;
unsigned char* pAlloc = (unsigned char*)0;
unsigned char* test_evt = "aaa\n";
javacall_int64 starttime = 0;

/* Copy from sni_event.h */
typedef enum SNIsignalType {
    NO_SIGNAL = 0,
    NETWORK_READ_SIGNAL,
    NETWORK_WRITE_SIGNAL,
    NETWORK_EXCEPTION_SIGNAL,
    COMM_OPEN_SIGNAL,
    COMM_READ_SIGNAL,
    COMM_WRITE_SIGNAL,
    COMM_CLOSE_SIGNAL,
    DEBUG_SIGNAL,
    VM_DEBUG_SIGNAL,
    HOST_NAME_LOOKUP_SIGNAL,
    SECURITY_CHECK_SIGNAL,
    WMA_SMS_READ_SIGNAL,
    WMA_SMS_WRITE_SIGNAL,
    PUSH_SIGNAL,
} SNIsignalType;

typedef struct _SNIReentryData {
    SNIsignalType waitingFor;   /**< type of signal */
    int descriptor;              /**< platform specific handle */
    int status;                  /**< error code produced by the operation
                                      that unblocked the thread */
    void* pContext;               /**< platform specific context info used
                                       by thread wait functions etc. */
} SNIReentryData;

int wma_enable = 0;
int network_enable = 0;
int event_enable = 1;
int comm_enable = 0;
int fc_enable = 0;
int network_ex_enable = 0;
int file_enable = 1;

javacall_result initialize() {
	return JAVACALL_OK;
}

void timer_cb(const javacall_handle handle) {
	if (handle == handle1)	timer_count1++;
	if (handle == handle2) timer_count2++;
}

void event_test_timer_cb(const javacall_handle handle) {
	javacall_event_send(test_evt, sizeof(test_evt));
}

javacall_result jctest_time() {
	javacall_print("========jctest_time========\n");
	starttime = javacall_time_get_milliseconds_since_1970();
	javacall_printf("start time: %d\n", (javacall_uint32)starttime);
	if (JAVACALL_OK == javacall_time_initialize_timer(10, JAVACALL_FALSE, timer_cb, &handle1)
		&&
		JAVACALL_OK == javacall_time_initialize_timer(10, JAVACALL_TRUE, timer_cb, &handle2)) {
		return JAVACALL_OK;
	}
	return JAVACALL_FAIL;
}

javacall_result jctest_memory() {
	javacall_printf("========jctest_memory(%d)========\n",MSG_BUF_LEN);
	pAlloc = (unsigned char*)javacall_malloc(MSG_BUF_LEN);
	if (pAlloc) {
		return JAVACALL_OK;
	} else {
		return JAVACALL_FAIL;
	}
}

javacall_result jctest_event() {
	int len = 0;
	javacall_handle handle;

	javacall_printf("========jctest_event========[%d,%d,%d]\n", 
						(javacall_uint32)(javacall_time_get_milliseconds_since_1970() - starttime), 
						timer_count1, timer_count2);

	javacall_event_receive(2000, pAlloc, MSG_BUF_LEN, &len);

	javacall_time_initialize_timer(5000, JAVACALL_FALSE, event_test_timer_cb, &handle);
	javacall_printf("Waiting for test event until it comes...[%d,%d,%d]\n", 
			(javacall_uint32)(javacall_time_get_milliseconds_since_1970() - starttime), timer_count1, timer_count2);
	javacall_event_receive(-1, pAlloc, MSG_BUF_LEN, &len);
	javacall_printf("Event received[%d,%d,%d]\n", 
			(javacall_uint32)(javacall_time_get_milliseconds_since_1970() - starttime), timer_count1, timer_count2);
	pAlloc[4]='\0';

	if (strcmp((char*)pAlloc, (char*)test_evt)) {
		return JAVACALL_FAIL;
	}
	if (JAVACALL_OK != javacall_time_finalize_timer(handle)) {
		return JAVACALL_FAIL;
	}
	return JAVACALL_OK;
}

#if ENABLE_JSR_120
javacall_result jctest_wma() {
	int len;
	javacall_result res;
	javacall_result ok;
	unsigned char* szWmaTest = "test";

	javacall_print("========jctest_wma========\n");
	ok = javacall_sms_is_service_available(0);
	if (JAVACALL_OK != ok) {
		return JAVACALL_FAIL;
	}

	javacall_print("send sms\n");
	res = javacall_sms_send(0, JAVACALL_SMS_MSG_TYPE_ASCII, "13330001111", szWmaTest, 4, 10000, 11111, 0);
	if (JAVACALL_WOULD_BLOCK == res) {
		javacall_event_receive(-1, pAlloc, MSG_BUF_LEN, &len);
	} else if (JAVACALL_OK != res) {
		return JAVACALL_FAIL;
	}
	
	javacall_sms_add_listening_port(12345);
	javacall_print("Waiting for sms\n");
	javacall_event_receive(-1, pAlloc, MSG_BUF_LEN, &len);
	javacall_print("sms received\n");
	javacall_sms_remove_listening_port(12345);
	if (strcmp((char*)pAlloc, (char*)szWmaTest)) {
		return JAVACALL_FAIL;
	} else {
		return JAVACALL_OK;
	}
}
#endif

javacall_result jctest_network() {
	javacall_handle h;
	void* pcontext;
	javacall_result res;
	int len;
	char host[4] = {220, 181, 57, 216};

	javacall_print("========jctest_network========\n");

	if (JAVACALL_OK != javacall_network_init_start()) {
		return JAVACALL_FAIL;
	}
	
	if (javacall_socket_open(JAVACALL_IP_VERSION_4, &h) != JAVACALL_OK) {
		return JAVACALL_FAIL;
	}

	javacall_print("javacall_socket_connect_start: start connecting...\n");
	res = javacall_socket_connect_start(h, JAVACALL_CONFIGURATION_NOT_SPECIFIED, JAVACALL_IP_VERSION_4,
									host,80,&pcontext);
	if (JAVACALL_WOULD_BLOCK == res) {
		javacall_print("Network connecting operation is blocking_1...\n");
		res = javacall_event_receive(-1, pAlloc, MSG_BUF_LEN, &len);
	} else if (JAVACALL_OK != res) {
		return JAVACALL_FAIL;
	}

	if (res != JAVACALL_OK) {
		return JAVACALL_FAIL;
	}

	javacall_print("Network connecting operation is blocking_2...\n");
	while ((((SNIReentryData*)pAlloc)->descriptor != (int)h) || (((SNIReentryData*)pAlloc)->waitingFor != NETWORK_WRITE_SIGNAL)) {
		res = javacall_event_receive(-1, pAlloc, MSG_BUF_LEN, &len);
		if (res != JAVACALL_OK) {
			return JAVACALL_FAIL;
		}
	}

	javacall_printf("javacall_socket connect event received: status=%d\n", ((SNIReentryData*)pAlloc)->status);
	res = javacall_socket_connect_finish(h, pcontext);
	if (res == JAVACALL_OK) {
		return JAVACALL_OK;
	} else {
		return JAVACALL_FAIL;
	}
}

javacall_result jctest_network_ex() {
	javacall_handle h, ah;
	void* pcontext;
	javacall_result res;
	int len;
	char host[4] = {39, 106, 21, 133};
	char *hostname = "api.toneless.org";
	char recv_buf[256];
	char hoststr[15];

	javacall_print("========jctest_network ex========\n");

	if (JAVACALL_OK != javacall_network_init_start()) {
		return JAVACALL_FAIL;
	}
	
	if (javacall_socket_open(JAVACALL_IP_VERSION_4, &h) != JAVACALL_OK) {
		return JAVACALL_FAIL;
	}


	// get host by name
	javacall_print("javacall_network_gethostbyname_start: start gethostbyname...\n");
	res = javacall_network_gethostbyname_start(hostname, JAVACALL_IP_VERSION_4, 0, hoststr, 16, &len, &ah, &pcontext);
	if (JAVACALL_WOULD_BLOCK == res) {
		javacall_print("Network gethostbyname operation is blocking_1...\n");
		res = javacall_event_receive(-1, pAlloc, MSG_BUF_LEN, &len);
		if (!res == JAVACALL_OK) {
			return JAVACALL_FAIL;
		}
	} else if (JAVACALL_OK != res) {
		return JAVACALL_FAIL;
	}

	
	res = javacall_network_gethostbyname_finish(JAVACALL_IP_VERSION_4, 0, hoststr, 16, &len, ah, pcontext);
	javacall_printf("javacall_network_gethostbyname_start: hoststr = %s\n", hoststr);

	if (res != JAVACALL_OK) {
		return JAVACALL_FAIL;
	}

	// connect
	javacall_print("javacall_socket_connect_start: start connecting...\n");
	res = javacall_socket_connect_start(h, JAVACALL_CONFIGURATION_NOT_SPECIFIED, JAVACALL_IP_VERSION_4,
									host,8000,&pcontext);
	if (JAVACALL_WOULD_BLOCK == res) {
		javacall_print("Network connecting operation is blocking_1...\n");
		res = javacall_event_receive(-1, pAlloc, MSG_BUF_LEN, &len);
	} else if (JAVACALL_OK != res) {
		return JAVACALL_FAIL;
	}

	if (res != JAVACALL_OK) {
		return JAVACALL_FAIL;
	}

	javacall_print("Network connecting operation is blocking_2...\n");
	javacall_printf("destcriptor = %d, h = %d, waitingfor = %d\n", ((SNIReentryData*)pAlloc)->descriptor, h, ((SNIReentryData*)pAlloc)->waitingFor);
	while ((((SNIReentryData*)pAlloc)->descriptor != (int)h) || (((SNIReentryData*)pAlloc)->waitingFor != NETWORK_WRITE_SIGNAL)) {
		res = javacall_event_receive(-1, pAlloc, MSG_BUF_LEN, &len);
		if (res != JAVACALL_OK) {
			return JAVACALL_FAIL;
		}
	}

	javacall_printf("javacall_socket connect event received: status=%d\n", ((SNIReentryData*)pAlloc)->status);
	res = javacall_socket_connect_finish(h, pcontext);
	if (!res == JAVACALL_OK) {
		return JAVACALL_FAIL;
	}

	// send message
	res = javacall_socket_write_start(h,
                                    "hello world",
                                    12,
                                    &len,
                                    &pcontext);
	if (JAVACALL_WOULD_BLOCK == res) {
		javacall_print("Network write operation is blocking_1...\n");
		javacall_event_receive(-1, pAlloc, MSG_BUF_LEN, &len);
	} else if (JAVACALL_OK != res) {
		return JAVACALL_FAIL;
	}

	while ((((SNIReentryData*)pAlloc)->descriptor != (int)h) || (((SNIReentryData*)pAlloc)->waitingFor != NETWORK_WRITE_SIGNAL)) {
		res = javacall_event_receive(-1, pAlloc, MSG_BUF_LEN, &len);
		if (res != JAVACALL_OK) {
			return JAVACALL_FAIL;
		}
	}

	javacall_socket_write_finish(h,
                                "hello world",
                                12,
                                &len,
                                pcontext);

	// recv message
	javacall_print("recv message..\n");
	res = javacall_socket_read_start(h,
                                   recv_buf,
                                   256,  
                                   &len,
                                   &pcontext);

	if (JAVACALL_WOULD_BLOCK == res) {
		javacall_print("Network read operation is blocking_1...\n");
		res = javacall_event_receive(-1, pAlloc, MSG_BUF_LEN, &len);
	} else if (JAVACALL_OK != res) {
		return JAVACALL_FAIL;
	}

	while ((((SNIReentryData*)pAlloc)->descriptor != (int)h) || (((SNIReentryData*)pAlloc)->waitingFor != NETWORK_READ_SIGNAL)) {
			res = javacall_event_receive(-1, pAlloc, MSG_BUF_LEN, &len);
			if (res != JAVACALL_OK) {
				return JAVACALL_FAIL;
			}
		}

	javacall_socket_read_finish(h,
                                   recv_buf,
                                   256,  
                                   &len,
                                   pcontext);
	
	javacall_printf("Network read msg = %s\n", recv_buf);
	
	// close
	res = javacall_socket_close_start(h, JAVACALL_TRUE, &pcontext);
	if (JAVACALL_WOULD_BLOCK == res) {
		javacall_print("Network close operation is blocking_1...\n");
		res = javacall_event_receive(-1, pAlloc, MSG_BUF_LEN, &len);
		javacall_socket_close_finish(h, pcontext);
	} else if (JAVACALL_OK != res) {
		return JAVACALL_FAIL;
	}
											
}


javacall_result jctest_comm() {
	/*char szStr[128];

	javacall_print("========jctest_comm========\n");
	
	if (JAVACALL_OK != javacall_serial_list_available_ports(szStr, 128)) {
		return JAVACALL_FAIL;
	}
	javacall_print("All available comm port:\n");
	javacall_print(szStr);
	javacall_print("\n");*/
	return JAVACALL_OK;
}

#define CHECK_FAIL(a,b) {if((a)!=JAVACALL_OK){javacall_print(b);return JAVACALL_FAIL;}}

javacall_result jctest_file() {
	javacall_handle handle;
	javacall_result res;
	long n;
	char* buffer;
	long size;
//	unsigned short* fn = L"/Hello.jar";
	unsigned short fn[256] = {'/','H','e','l','l','o','.','j','a','r'};
	javacall_print("========jctest_file========\n");
	res = javacall_file_open(fn, javautil_wcslen(fn), JAVACALL_FILE_O_RDONLY, &handle);
	CHECK_FAIL(res, "open file error\n");

	size = (int)javacall_file_sizeofopenfile(handle);
	if (size <= 0) {
		javacall_printf("javacall_file_sizeofopenfile returned unexpected result: %ld\n", size);
		return JAVACALL_FAIL;
	} else {
		javacall_printf("javacall_file_sizeofopenfile returned %ld\n", size);
	}

	n = javacall_file_seek(handle, 0, JAVACALL_FILE_SEEK_SET);
	if (n != 0) {
		javacall_printf("javacall_file_seek returned unexpected result: %ld\n", n);
		return JAVACALL_FAIL;
	}

	buffer = (char*)javacall_malloc((unsigned int)size);
	if (buffer == NULL) {
		javacall_print("Mem alloc error\n");
		return JAVACALL_FAIL;
	}
	n = javacall_file_read(handle, (unsigned char*)buffer, size);
	if (n < 0) {
		javacall_print("read file error\n");
	} else if (n == 0) {
		javacall_print("read file eof\n");
	} else {
		//buffer[n] = '\0';
		javacall_printf("(%ld) bytes read\n", n);
		//javacall_printf("buffer content:%s\n", buffer);
	}
	javacall_free(buffer);
	res = javacall_file_close(handle);
	CHECK_FAIL(res, "close file error");
	//javacall_print(buffer);
	
	return JAVACALL_OK;
}

#if ENABLE_JSR_75
javacall_result jctest_fc() {
	javacall_utf16* roots;
	int i = 0;
	javacall_utf16* pathname;
	int nroots = 1;
	javacall_result res;
	javacall_utf16* p;
	
	javacall_print("========jctest_fc========\n");
	javacall_fileconnection_init();
	roots = (javacall_utf16*)javacall_malloc(JAVACALL_MAX_ROOTS_LIST_LENGTH*sizeof(javacall_utf16));
	pathname = (javacall_utf16*)javacall_malloc(JAVACALL_MAX_ROOT_PATH_LENGTH*sizeof(javacall_utf16));

	if (!roots || !pathname) {
		javacall_print("out of memory\n");
		return JAVACALL_FAIL;
	}
	
	res = javacall_fileconnection_get_mounted_roots(roots, JAVACALL_MAX_ROOTS_LIST_LENGTH);

	if (res != JAVACALL_OK) {
		return JAVACALL_FAIL;
	}

	javacall_print("Mounted Roots:\n");
	p = roots;

	while (roots[i] != 0) {
		char ch = (char)(roots[i]&0xff);
		javacall_printf("%c", ch);
		if (ch == '\n') {
			roots[i] = (javacall_utf16)0;
			nroots++;
		}
		i++;
	}
	javacall_printf("\ntotal %d root entries\n", nroots);

	
	while (nroots--) {
		int len = javautil_wcslen(p);
		javacall_fileconnection_get_path_for_root(p,pathname, JAVACALL_MAX_FILE_NAME_LENGTH);
		i = 0;
		while (pathname[i] != 0) {
			javacall_printf("%c", (char)pathname[i]&0xff);
			i++;
		}
		p += (len+1);
		javacall_print("\n");
	}

	res = javacall_fileconnection_get_illegal_filename_chars(roots, JAVACALL_MAX_ROOTS_LIST_LENGTH);
	if (res != JAVACALL_OK) {
		return JAVACALL_FAIL;
	}

	javacall_print("Illegal filename chars:\n");
	i = 0;
	while (roots[i] != 0) {
		javacall_printf("%c", (char)(roots[i]&0xff));
		i++;
	}
	javacall_print("\n");
	return JAVACALL_OK;
}
#endif

void StartJavacallTest() {
	javacall_result ok;
	javacall_print("Javacall native tests start from here...\n");
	ok = initialize();
	javacall_printf("Javacall tests initialized and return: %d\n", ok==JAVACALL_OK?0:-1);
	ok = jctest_time();
	javacall_printf("jctest_time: %s\n", ok==JAVACALL_OK?"PASSED":"FAILED");
	ok = jctest_memory();
	javacall_printf("jctest_memory: %s\n", ok==JAVACALL_OK?"PASSED":"FAILED");
	if (event_enable) {
		ok = jctest_event();
		javacall_printf("jctest_event: %s\n", ok==JAVACALL_OK?"PASSED":"FAILED");
	}
#if ENABLE_JSR_120
	if (wma_enable) {
		ok = jctest_wma();
		javacall_printf("jctest_wma: %s\n", ok==JAVACALL_OK?"PASSED":"FAILED");
	}
#endif
	if (network_enable) {
		ok = jctest_network();
		javacall_printf("jctest_network: %s\n", ok==JAVACALL_OK?"PASSED":"FAILED");
	}
	if (network_ex_enable) {
		ok = jctest_network_ex();
		javacall_printf("jctest_network_ex: %s\n", ok==JAVACALL_OK?"PASSED":"FAILED");
	}
	if (comm_enable) {
		ok = jctest_comm();
		javacall_printf("jctest_comm: %s\n", ok==JAVACALL_OK?"PASSED":"FAILED");
	}
#if ENABLE_JSR_75
	if (fc_enable) {
		ok = jctest_fc();
		javacall_printf("jctest_fc: %s\n", ok==JAVACALL_OK?"PASSED":"FAILED");
	}
	
#endif
	if (file_enable) {
		ok = jctest_file();
		javacall_printf("jctest_file: %s\n", ok==JAVACALL_OK?"PASSED":"FAILED");
	}
	//jctest_dir();
	
	javacall_time_finalize_timer(handle1);
	javacall_time_finalize_timer(handle2);
	javacall_free(pAlloc);
	javacall_printf("jctest_timer_count(%d,%d)\n", timer_count1, timer_count2);
	javacall_print("End\n");
//	JavaTask();
}

