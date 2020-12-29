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
javacall_time_milliseconds starttime = 0;

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
int comm_enable = 1;
int fc_enable = 0;
int network_ex_enable = 0;



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
	javacall_printf("start time: %d\n", starttime);
	if (JAVACALL_OK == javacall_time_initialize_timer(100, JAVACALL_TRUE, timer_cb, &handle1)
		//&&
		//JAVACALL_OK == javacall_time_initialize_timer(100, JAVACALL_TRUE, timer_cb, &handle2)
		) {
		return JAVACALL_OK;
	}
	return JAVACALL_FAIL;
}

javacall_result jctest_memory() {
	javacall_print("========jctest_memory========\n");
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

	starttime = javacall_time_get_milliseconds_since_1970();
	
	javacall_printf("========jctest_event========[%d,%d]\n", starttime, timer_count1);

	javacall_event_receive(2000, pAlloc, MSG_BUF_LEN, &len);

	starttime = javacall_time_get_milliseconds_since_1970();

	javacall_printf("========2000ms later========[%d,%d]\n", starttime, timer_count1);

	javacall_event_receive(5000, pAlloc, MSG_BUF_LEN, &len);

	
	starttime = javacall_time_get_milliseconds_since_1970();
		javacall_printf("========5000ms later========[%d,%d]\n", starttime, timer_count1);

	javacall_time_finalize_timer(handle1);

	javacall_event_receive(2000, pAlloc, MSG_BUF_LEN, &len);

	javacall_time_initialize_timer(5000, JAVACALL_FALSE, event_test_timer_cb, &handle);
	starttime = javacall_time_get_milliseconds_since_1970();
	javacall_printf("Waiting for test event until it comes...[%d,%d]\n", starttime, timer_count1);
	javacall_event_receive(-1, pAlloc, MSG_BUF_LEN, &len);
	starttime = javacall_time_get_milliseconds_since_1970();
	javacall_printf("Event received[%d,%d]\n", starttime, timer_count1);
	pAlloc[4]='\0';

	if (strcmp((char*)pAlloc, (char*)test_evt)) {
		return JAVACALL_FAIL;
	}
	javacall_time_finalize_timer(handle);
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
#if 0
javacall_result jctest_network() {
	javacall_handle h;
	void* pcontext;
	javacall_result res;
	int len;
	char host[4] = {54, 222, 225, 30};

	javacall_print("========jctest_network========\n");

	if (JAVACALL_WOULD_BLOCK = javacall_network_init_start()) {
		javacall_print("javacall_network_init_start: would block\n");
		res = javacall_event_receive(-1, pAlloc, MSG_BUF_LEN, &len);
		javacall_print("javacall_network_init_start: recv init finish notification\n");
		if (res != JAVACALL_OK) {
			return JAVACALL_FAIL;
		}
		javacall_network_init_finish();
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
#endif

#define CHECK_FAIL(a,b) {if((a)!=JAVACALL_OK){javacall_print(b);return JAVACALL_FAIL;}}

javacall_result jctest_comm() {
	char szStr[128];
	javacall_handle hPort;
	void* pContext;
	javacall_result res;
	int bytesRead = 0;
	unsigned char buffer[16];
	int size = 16;
	int len;

	javacall_print("========jctest_comm========\n");
	
	if (JAVACALL_OK != javacall_serial_list_available_ports(szStr, 128)) {
		return JAVACALL_FAIL;
	}
	javacall_print("All available comm port:\n");
	javacall_print(szStr);
	javacall_print("\n");

	CHECK_FAIL(javacall_serial_open_start("COM0", 115200, 0, &hPort, &pContext), "javacall_serial_open_start failed\n");
	res = javacall_serial_read_start(hPort, buffer, size, &bytesRead, pContext);
	if (res == JAVACALL_OK) {
		javacall_printf("javacall_serial_read_start reads in %d bytes\n", bytesRead);
		return JAVACALL_OK;
	} else if (res == JAVACALL_WOULD_BLOCK) {
		javacall_print("javacall_serial_read_start is blocking1\n");
		res = javacall_event_receive(-1, pAlloc, MSG_BUF_LEN, &len);
	} else {
		return JAVACALL_FAIL;
	}

	javacall_print("javacall_serial_read_start is blocking1\n");
	while ((((SNIReentryData*)pAlloc)->descriptor != (int)hPort) || (((SNIReentryData*)pAlloc)->waitingFor != COMM_READ_SIGNAL)) {
		res = javacall_event_receive(-1, pAlloc, MSG_BUF_LEN, &len);
		if (res != JAVACALL_OK) {
			return JAVACALL_FAIL;
		}
	}

	javacall_printf("serial event received: status=%d\n", ((SNIReentryData*)pAlloc)->status);
	CHECK_FAIL(javacall_serial_read_finish(hPort, buffer, size, &bytesRead, &pContext), "javacall_serial_read_finish fail\n");
	javacall_printf("javacall_serial_read_finish read % bytes\n", bytesRead);
	CHECK_FAIL(javacall_serial_close_start(hPort, &pContext), "javacall_serial_close_start fail\n");
	return JAVACALL_OK;
}


#if 0
javacall_result jctest_file() {
	javacall_handle handle;
	javacall_result res;
	long n;
	char* buffer;
	long size;
	unsigned short* fn = L"C:\\java\\appdb\\HelloWorld.jar";
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
#endif
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
		javacall_events_init();
		ok = jctest_event();
		javacall_printf("jctest_event: %s\n", ok==JAVACALL_OK?"PASSED":"FAILED");
	}
#if ENABLE_JSR_120
	if (wma_enable) {
		ok = jctest_wma();
		javacall_printf("jctest_wma: %s\n", ok==JAVACALL_OK?"PASSED":"FAILED");
	}
#endif
#if 0
	if (network_enable) {
		ok = jctest_network();
		javacall_printf("jctest_network: %s\n", ok==JAVACALL_OK?"PASSED":"FAILED");
	}
#endif
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
#if 0
	ok = jctest_file();
	javacall_printf("jctest_file: %s\n", ok==JAVACALL_OK?"PASSED":"FAILED");
	//jctest_dir();
#endif
	//javacall_time_finalize_timer(handle1);
	//javacall_time_finalize_timer(handle2);
	javacall_free(pAlloc);
	javacall_printf("%d,%d\n", timer_count1, timer_count2);
	javacall_print("End");

}

