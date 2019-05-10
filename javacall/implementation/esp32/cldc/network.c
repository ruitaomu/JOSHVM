/**
 * @file
 *
 * Implementation of pcsl_network.h for platforms that support the winsock 
 * API.
 *
 * For all functions, the "handle" is the winsock handle (an int)
 * cast to void *.  Since winsock reads and writes to sockets are synchronous,
 * the context for reading and writing is always set to NULL.
 */

#include "javacall_network.h"
#include "javacall_socket.h"
#include "javacall_datagram.h"
#include "javacall_events.h"
#include "javacall_logging.h"
#include "javacall_properties.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "sdkconfig.h"


//======================= WIFI IMPLEMENTATION ==========================
static enum {
	STA_UNINITIALIZED,
	STA_START,
	STA_CONNECTING,
	STA_CONNECTED,
	STA_DISCONNECTED
} wifi_status;

#define CONFIG_ESP_WIFI_SSID "DEFAULTSSID"
#define CONFIG_ESP_WIFI_PASSWORD "DEFAULTPASSWORD"

static int nvs_initialized = 0;
static const char *TAG = "JOSHVM WIFI";

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
		ESP_LOGI(TAG, "station start event handled. Connecting to wifi...");
		wifi_status = STA_CONNECTING;
        esp_wifi_connect();		
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        ESP_LOGI(TAG, "got ip:%s",
                 ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
 		wifi_status = STA_CONNECTED;
		javanotify_network_event(JAVACALL_NETWORK_UP, JAVACALL_TRUE);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
		javanotify_network_event(JAVACALL_NETWORK_DOWN, JAVACALL_TRUE);
		wifi_status = STA_CONNECTING;
		esp_wifi_connect();
        break;
    default:
        break;
    }
    return ESP_OK;
}

static void wifi_init_sta()
{
	char* ssid = NULL;
	char* password = NULL;
	
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL) );

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = CONFIG_ESP_WIFI_SSID,
            .password = CONFIG_ESP_WIFI_PASSWORD
        },
    };
	
	if (JAVACALL_OK == javacall_get_property("system.network.wifi.ssid", JAVACALL_INTERNAL_PROPERTY, &ssid) && ssid) {
		strncpy(wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
	}

	if (JAVACALL_OK == javacall_get_property("system.network.wifi.password", JAVACALL_INTERNAL_PROPERTY, &password) && password) {
		strncpy(wifi_config.sta.password, password, sizeof(wifi_config.sta.password));
	}
	
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
	ESP_LOGI(TAG, "esp_wifi_start ...");
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init_sta finished.");
    ESP_LOGI(TAG, "connect to ap SSID:%s password:%s",
             wifi_config.sta.ssid, wifi_config.sta.password);
	wifi_status = STA_START;
}

//=============== SOCKET IMPLEMENTATION ================
#define MAXGETHOSTSTRUCT 32

#define MAX_HOST_LENGTH 256
//#define ENABLE_NETWORK_TRACING

#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1
#endif

#ifndef SOCKET_ERROR
#define SOCKET_ERROR -1
#endif

#define MAX_HOST_ADDR_LENGTH 16

int GetFD(javacall_handle handle) {
	if (handle == NULL) {
		return -1;
	} else {
		int* p = (int*)handle;
		return *p;
	}
}

void SetFD(javacall_handle handle, int fd) {
	if (handle != NULL) {
		int* p = (int*)handle;
		*p = fd;
	}
}

javacall_handle AllocHandle() {
	return (javacall_handle)malloc(sizeof(int));
}

void FreeHandle(javacall_handle handle) {
	if (handle != NULL) {
		free(handle);
	}
}

#define IsInvalidFD(d) ((d) == -1)

typedef struct {
	javacall_handle handle;
	javacall_event_type event;
} socket_event_type;

typedef struct {
	char* hostname;
	int   h_addrtype;
    int   h_length;
	char  addr[MAX_HOST_ADDR_LENGTH];
} gethostbyname_evt_t;

int close_socket(javacall_handle handle){
	int fd = GetFD(handle);
	SetFD(handle, -1);
	FreeHandle(handle);
	if (!IsInvalidFD(fd)) {
		lwip_close(fd);
	} else {
		javacall_logging_printf(JAVACALL_LOGGING_WARNING, JC_NETWORK, "close_socket: Invalid handle\n");
	}
	return 0;
}

static int socket_available(int fd){
	int value = -1;
	if (0 == lwip_ioctl(fd, FIONREAD, &value)) {
		return value;
	} else {
		return -1;
	}
}

static javacall_result getHostByName_blocking(javacall_handle handle) {
	struct hostent *he;
	gethostbyname_evt_t* pEvt = (gethostbyname_evt_t*)handle;

	he = lwip_gethostbyname(pEvt->hostname);
	
	if (!he){
		javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_NETWORK, "javacall_network_gethostbyname_start: JAVACALL_FAIL\n"); 
		return JAVACALL_FAIL;
	}

	int addrLen = sizeof(ip4_addr_t);
    if (addrLen > MAX_HOST_ADDR_LENGTH) {
		javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_NETWORK, "javacall_network_gethostbyname_start: too long address length: %d\n", addrLen);
        return JAVACALL_INVALID_ARGUMENT;
    }

    pEvt->h_length = addrLen;
	memcpy(pEvt->addr, he->h_addr, addrLen);
	pEvt->h_addrtype = he->h_addrtype;
	
	return JAVACALL_OK;
}

void *socket_event_handler(void *arg){

	struct timeval tv;
	fd_set rfds, wfds;
	socket_event_type *a = (socket_event_type *)arg;
	javacall_handle handle = a->handle;
	javacall_event_type event = a->event;
	int fd = -1;
	int r=-1;

	if (event != EVENT_FD_GETHOSTBYNAME) {
		fd = GetFD(handle);
	}

	free(a);

	if (event==EVENT_FD_NETWORKUP){
		javanotify_network_event(JAVACALL_NETWORK_UP, JAVACALL_FALSE);
		return NULL;
	}
	
	if (event==EVENT_FD_GETHOSTBYNAME){
		javacall_result result = getHostByName_blocking(handle);
		javanotify_socket_event(
                           JAVACALL_EVENT_NETWORK_GETHOSTBYNAME_COMPLETED,
                           handle,
                           result);
		
		return NULL;
	}

	FD_ZERO(&rfds);    
	FD_ZERO(&wfds);
	FD_SET(fd, &rfds);    
    FD_SET(fd, &wfds); 

	tv.tv_sec = 10;
	tv.tv_usec =0;

	javacall_result result = JAVACALL_FAIL;
	javacall_socket_event_type jc_event;
	
	if (event == EVENT_FD_CONNECT){
		jc_event = JAVACALL_EVENT_SOCKET_CONNECT_COMPLETED;
		if (!IsInvalidFD(fd)) {
			r = select(fd + 1, &rfds, &wfds, 0, &tv); 
			if (r>0 && (FD_ISSET(fd, &rfds) || FD_ISSET(fd, &wfds))) {
				result = JAVACALL_OK;
			}
		}
	} else if (event == EVENT_FD_ACCEPT){
		jc_event = JAVACALL_EVENT_SERVER_SOCKET_ACCEPT_COMPLETED;
		if (!IsInvalidFD(fd)) {
			r = select(fd + 1, &rfds, &wfds, 0, NULL);
			if (r>0 && (FD_ISSET(fd, &rfds) || FD_ISSET(fd, &wfds))) {
				result = JAVACALL_OK;
			}
		}
	} else if (event == EVENT_FD_READ){
		jc_event = JAVACALL_EVENT_SOCKET_RECEIVE;
		if (!IsInvalidFD(fd)) {
			r = select(fd + 1, &rfds, 0, 0, NULL); 
			if (r>0 && FD_ISSET(fd, &rfds)) {
				result = JAVACALL_OK;
			}
		}
	} else if (event == EVENT_FD_WRITE){
		jc_event = JAVACALL_EVENT_SOCKET_SEND;
		if (!IsInvalidFD(fd)) {
			r = select(fd + 1, 0, &wfds, 0, &tv); 
			if (r>0 && FD_ISSET(fd, &wfds)) {
				javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_NETWORK, "socket_event_handler: EVENT_FD_WRITE\n");
				result = JAVACALL_OK;
			}
		}
	} else {
		return NULL;
	}

	javanotify_socket_event(jc_event, handle, result);
	return NULL;
}


int set_event_observer(javacall_handle handle, javacall_event_type event){
	pthread_t tid;

	socket_event_type *e = (socket_event_type*)malloc(sizeof(socket_event_type));
	e->handle = handle;
	e->event = event;
	
	if (pthread_create(&tid, NULL, socket_event_handler, e) != 0){
		javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_NETWORK, "pthread_create fail\n");
		free(e);
		return -1;
	} else {
		pthread_detach(tid);
		return 0;
	}
}

//=================== Javacall Interface IMPLEMENTATION ========================
javacall_result javacall_socket_open(javacall_ip_version ip_version,
                                     javacall_handle* pHandle) {
	int fd;
	int falsebuf = 0;
	int status;
	int flags;

	*pHandle = AllocHandle();
	if (!*pHandle) {
		return JAVACALL_FAIL;
	}

	fd = lwip_socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1) {
		return JAVACALL_FAIL;
	}	
	
	javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_NETWORK, "javacall_socket_open() returns handle=%p, fd=%d\n", *pHandle, fd);
	SetFD(*pHandle, fd);		

	//make socket non-blocking
	flags = lwip_fcntl(fd, F_GETFL, 0); 
	status = lwip_fcntl(fd, F_SETFL, flags | O_NONBLOCK);
	if(status == -1) {
		close_socket(*pHandle);
		return JAVACALL_FAIL;
	}

	return JAVACALL_OK;
}

/**
 * See pcsl_network.h for definition.
 */
javacall_result javacall_socket_connect_start(
	javacall_handle handle,
    javacall_configuration_id config_id,
    javacall_ip_version ip_version,
    unsigned char *ipBytes,
	int port, 
	void **pContext)
{
	int fd = GetFD(handle);
	int status;
	struct sockaddr_in addr;

	javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_NETWORK, "javacall_socket_connect_start: handle=%p, fd=%d\n", handle, fd);

	if (IsInvalidFD(fd)) {
		javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_NETWORK, "javacall_socket_connect_start: Invalid handle\n");
		return JAVACALL_FAIL;
	}
	
	if (ipBytes == NULL){
		//javacall_printf("javacall_socket_connect_start: JAVACALL_INVALID_ARGUMENT\n");
	    return JAVACALL_INVALID_ARGUMENT;
	}

	if (ip_version != JAVACALL_IP_VERSION_4){
		//javacall_printf("javacall_socket_connect_start: JAVACALL_IP_VERSION_4\n");
		return JAVACALL_INVALID_ARGUMENT;
	}

	//javacall_printf("javacall_socket_connect_start: ipAddr = %d.%d.%d.%d\n", ipBytes[0], ipBytes[1], ipBytes[2], ipBytes[3]);
	int addr_family, addr_len;
	memset(&addr, 0, sizeof(addr));
	if (ip_version == JAVACALL_IP_VERSION_4) {
		addr_family = AF_INET;
		addr_len = 4;
	} else if (ip_version == JAVACALL_IP_VERSION_6) {
		addr_family = AF_INET6;
		addr_len = 6;
	} else {
		//javacall_printf("javacall_socket_connect_start: JAVACALL_IP_VERSION_4\n");
		return JAVACALL_INVALID_ARGUMENT;
	}
	
	addr.sin_len = sizeof(addr);
	addr.sin_family      = addr_family;
	addr.sin_port        = PP_HTONS((unsigned short)port);

	memcpy((char*)&addr.sin_addr, (char*)ipBytes, addr_len);
	status = lwip_connect(fd, (struct sockaddr *)&addr, sizeof(addr));
	
	*pContext = NULL;

	if (status == 0) {
	 	javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_NETWORK, "javacall_socket_connect_start: JAVACALL_OK\n");
	    return JAVACALL_OK;
	}
	
	if (errno == EINPROGRESS) {
	    //javacall_printf("javacall_socket_connect_start: fd=%d\n", fd);
	    if (set_event_observer(handle, EVENT_FD_CONNECT)) {
			return JAVACALL_FAIL;
		}
		
	    javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_NETWORK,  "javacall_socket_connect_start: JAVACALL_WOULD_BLOCK\n");
	    return JAVACALL_WOULD_BLOCK;
	}

	javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_NETWORK,  "javacall_socket_connect_start: JAVACALL_CONNECTION_NOT_FOUND\n");
	return JAVACALL_CONNECTION_NOT_FOUND; 
}

/**
 * See pcsl_network.h for definition.
 */
int javacall_socket_connect_finish(void *handle,void *context){
	int port;
	javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_NETWORK,  "javacall_socket_connect_finish: trace\n");
	if (javacall_socket_getlocalport(handle, &port) == JAVACALL_OK) {
		javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_NETWORK, "javacall_socket_connect_finish: localport=%d\n", port);
	}
	return JAVACALL_OK;

}

/**
 * Common implementation between read_start() and read_finish().
 */
static int socket_read_common(void *handle,
                               unsigned char *pData,
                               int len,
                               int *pBytesRead){

	//javacall_printf("socket_read_common: trace, handle=%p\n", handle);

    int fd = GetFD(handle);
    int bytesRead;

	if (IsInvalidFD(fd)) {
		javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_NETWORK, "socket_read_common: Invalid handle\n");
		return JAVACALL_FAIL;
	}

    bytesRead = lwip_recv(fd, (char*)pData, len, 0);
	//javacall_printf("socket_read_common: fd=%d, %d, %s\n", *fd, lastError, strerror(lastError));

    if (bytesRead != -1) {
        *pBytesRead = bytesRead;
		//javacall_printf("socket_read_common: JAVACALL_OK\n");
        return JAVACALL_OK;
    }

    if (errno == EWOULDBLOCK) {
        if (set_event_observer(handle, EVENT_FD_READ)) {
			return JAVACALL_FAIL;
		}
        //javacall_printf("socket_read_common: JAVACALL_WOULD_BLOCK\n");
		return JAVACALL_WOULD_BLOCK;
    }

	if (errno == EINTR) {
		//javacall_printf("socket_read_common: JAVACALL_INTERRUPTED\n");
		return JAVACALL_INTERRUPTED;
	}

	//javacall_printf("socket_read_common: JAVACALL_FAIL\n");
    return JAVACALL_FAIL;
}

/**
 * See pcsl_network.h for definition.
 */
javacall_result javacall_socket_read_start(void *handle,
                                           unsigned char *pData,
                                           int len,  
                                           int *pBytesRead,
                                           void **pContext){
    //javacall_printf("javacall_socket_read_start: trace\n");

	(void)pContext;
    return socket_read_common(handle, pData, len, pBytesRead);

}


/**
 * See pcsl_network.h for definition.
 */
javacall_result javacall_socket_read_finish(void *handle,
                                            unsigned char *pData,
                                            int len,
                                            int *pBytesRead,
                                            void *context){

	//javacall_printf("javacall_socket_read_finish: trace\n");

	(void)context;
    return socket_read_common(handle, pData, len, pBytesRead);
}

/**
 * Common implementation between write_start() and write_finish().
 */
static int socket_write_common(void *handle,
                                char *pData,
                                int len,
                                int *pBytesWritten){

	//javacall_printf("socket_write_common: trace, handle=%p\n", handle);

    int fd = GetFD(handle);
    int bytesSent;

	if (IsInvalidFD(fd)) {
		javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_NETWORK, "socket_write_common: Invalid handle\n");
		return JAVACALL_FAIL;
	}

    bytesSent = lwip_send(fd, pData, len, 0);
    javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_NETWORK, "socket_write_common(): write %d of %d\n", bytesSent, len);

    if (bytesSent != -1) {
        *pBytesWritten = bytesSent;
		//javacall_printf("socket_write_common: JAVACALL_OK\n");
        return JAVACALL_OK;
    }

    if (errno == EWOULDBLOCK) {
        if (set_event_observer(handle, EVENT_FD_WRITE)) {
			return JAVACALL_FAIL;
		}
        //javacall_printf("socket_write_common: JAVACALL_WOULD_BLOCK\n");
        return JAVACALL_WOULD_BLOCK;
    }

    if (errno == EINTR){
		//javacall_printf("socket_write_common: JAVACALL_INTERRUPTED\n");
        return JAVACALL_INTERRUPTED;
    }

	//javacall_printf("socket_write_common: JAVACALL_FAIL, lastError = %d\n", lastError);
    return JAVACALL_FAIL;
}

/**
 * See pcsl_network.h for definition.
 */
javacall_result javacall_socket_write_start(void *handle,
                                            char *pData,
                                            int len,
                                            int *pBytesWritten,
                                            void **pContext){

	//javacall_printf("javacall_socket_write_start: trace\n");										

    *pContext = NULL;
    return socket_write_common(handle, pData, len, pBytesWritten);

}


/**
 * See pcsl_network.h for definition.
 */
javacall_result javacall_socket_write_finish(void *handle,
                                             char *pData,
                                             int len,
                                             int *pBytesWritten,
                                             void *context){

	//javacall_printf("javacall_socket_write_finish: trace\n");
	
	(void)context;
    return socket_write_common(handle, pData, len, pBytesWritten);

}


/**
 * See pcsl_network.h for definition.
 */
javacall_result javacall_socket_available(void *handle, int *pBytesAvailable){

	//javacall_printf("javacall_socket_available: trace\n");
	
    int fd = GetFD(handle);
    int len;

	if (IsInvalidFD(fd)) {
		javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_NETWORK, "socket_write_common: Invalid handle\n");
		return JAVACALL_FAIL;
	}
	
	len = socket_available(fd);
	
	if (len != -1) {
        *pBytesAvailable = (int)len;
		//javacall_printf("javacall_socket_available: JAVACALL_OK\n");
        return JAVACALL_OK;
    }
	
	//javacall_printf("javacall_socket_available: JAVACALL_FAIL\n");
    return JAVACALL_FAIL;

}

/**
 * See pcsl_network.h for definition.
 */
javacall_result javacall_socket_shutdown_output(void *handle) {

	return JAVACALL_FAIL;

}


/**
 * See pcsl_network.h for definition.
 *
 * Note that this function NEVER returns PCSL_NET_WOULDBLOCK. Therefore, the 
 * finish() function should never be called and does nothing.
 */
javacall_result javacall_socket_close_start(void *handle, javacall_bool abort,
                                            void **pContext){

	//javacall_printf("javacall_socket_close_start: trace\n");
    (void)pContext;
	
	int fd = GetFD(handle);

	if (IsInvalidFD(fd)) {
		javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_NETWORK, "javacall_socket_close_start: Invalid handle. Already closed?\n");
		return JAVACALL_OK;
	}

	lwip_shutdown(fd, SHUT_RDWR);

    if (close_socket(handle) == 0) {
		//javacall_printf("javacall_socket_close_start: JAVACALL_OK\n");
        return JAVACALL_OK;
    }        

	//javacall_printf("javacall_socket_close_start: JAVACALL_FAIL\n");
    return JAVACALL_FAIL;

}


/**
 * See pcsl_network.h for definition.
 */
javacall_result javacall_socket_close_finish(void *handle,
                                             void *context){
                                             
	//javacall_printf("javacall_socket_close_finish: trace\n");
	(void)handle;
	(void)context;
											 
	//javacall_printf("javacall_socket_close_finish: JAVACALL_OK\n");
	return JAVACALL_OK;
}

/**
 * See pcsl_network.h for definition.
 */
javacall_result javacall_network_gethostbyname_start(char *hostname,
                                                     javacall_ip_version ip_version,
                                                     javacall_configuration_id id,
                                                     unsigned char *pAddress, 
                                                     int maxLen,
                                                     int *pLen,
                                                     void **pHandle,
                                                     void **pContext){
	struct sockaddr_in sa;

	if ((ip_version == JAVACALL_IP_VERSION_ANY) || (ip_version == JAVACALL_IP_VERSION_4)) {
		if (inet_pton(AF_INET, hostname, &(sa.sin_addr)) == 1) {
			if (maxLen < 4) {
				return JAVACALL_INVALID_ARGUMENT;
			}
			memcpy(pAddress, (char*)&sa.sin_addr, 4);
			return JAVACALL_OK;
		}	
	}

	if ((ip_version == JAVACALL_IP_VERSION_ANY) || (ip_version == JAVACALL_IP_VERSION_6)) {
		if (inet_pton(AF_INET6, hostname, &(sa.sin_addr)) == 1) {
			if (maxLen < 16) {
				return JAVACALL_INVALID_ARGUMENT;
			}
			memcpy(pAddress, (char*)&sa.sin_addr, 16);
			return JAVACALL_OK;
		}
	}

	gethostbyname_evt_t* p = malloc(sizeof(gethostbyname_evt_t));
	
	p->hostname = malloc(strlen(hostname)+1);

	strcpy(p->hostname, hostname);

	*pHandle = p;

	javacall_printf("javacall_network_gethostbyname_start: handle=%d\n", p);

	if (set_event_observer(*pHandle, EVENT_FD_GETHOSTBYNAME)) {
		free(p->hostname);
		free(p);
		return JAVACALL_FAIL;
	}

	return JAVACALL_WOULD_BLOCK;	
}


/**
 * See pcsl_network.h for definition.
 *
 * Since the start function never returns PCSL_NET_WOULDBLOCK, this
 * function should never be called.
 */
javacall_result javacall_network_gethostbyname_finish(
                                                      javacall_ip_version ip_version,
                                                      javacall_configuration_id id,
                                                      unsigned char *pAddress,
                                                      int maxLen,
                                                      int *pLen,
                                                      void *handle,
                                                      void *context){
	javacall_result result = JAVACALL_FAIL;

	//javacall_printf("javacall_network_gethostbyname_finish: handle = %d\n", handle);

	gethostbyname_evt_t* pEvt = (gethostbyname_evt_t*)handle;

	if ((pEvt->h_addrtype == AF_INET) && (ip_version == JAVACALL_IP_VERSION_6)) {
		javacall_printf("javacall_network_gethostbyname_finish: Expected IPv6 but got IPv4\n");
	} else if ((pEvt->h_addrtype == AF_INET6) && (ip_version == JAVACALL_IP_VERSION_4)) {
		javacall_printf("javacall_network_gethostbyname_finish: Expected IPv4 but got IPv6\n");
	} else if (maxLen < pEvt->h_length) {
		javacall_printf("javacall_network_gethostbyname_finish: Buffer not enough for host addr\n");
	} else {
	    *pLen = pEvt->h_length;
	    memcpy(pAddress, pEvt->addr, *pLen);
		result = JAVACALL_OK;
		javacall_printf("javacall_network_gethostbyname_finish: JAVACALL_OK\n");
	}

	free(pEvt->hostname);
	free(pEvt);
	
	return result;

}


/**
 * See pcsl_network.h for definition.
 */
javacall_result javacall_network_getsockopt(void *handle,
                                            javacall_socket_option flag,
                                            int *pOptval){

    return JAVACALL_FAIL;
}


/**
 * See pcsl_network.h for definition.
 */
javacall_result javacall_network_setsockopt(void *handle,
                                            javacall_socket_option flag,
                                            int optval){

    return JAVACALL_FAIL;
}

/**
 * Gets the string representation of the local device's IP address.
 * This function returns dotted quad IP address as a string in the 
 * output parameter and not the host name.
 *
 * @param pLocalIPAddress base of char array to receive the local
 *        device's IP address
 *
 * @retval JAVACALL_OK      success
 * @retval JAVACALL_FAIL    if there is a network error
 */

javacall_result 
javacall_network_get_local_ip_address_as_string(javacall_ip_version ip_version,
                                                                    /*OUT*/ char *pLocalIPAddress){

	return JAVACALL_FAIL;
}
															
/**
 * See javacall_network.h for definition.
 */
javacall_result javacall_network_init_start(void) {
	if (!nvs_initialized) {
	    //Initialize NVS
	    esp_err_t ret = nvs_flash_init();
	    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
	      if (ESP_OK != nvs_flash_erase()) {
			return JAVACALL_FAIL;
		  }
	      ret = nvs_flash_init();
	    }
	    if (ESP_OK != ret) {
			return JAVACALL_FAIL;
		}
		nvs_initialized = 1;
	}
	
	switch (wifi_status) {
	case STA_CONNECTED:
		return JAVACALL_OK;
	case STA_UNINITIALIZED: 
		wifi_init_sta();
		break;
	case STA_START:
	case STA_CONNECTING:
		break;
	case STA_DISCONNECTED:
		wifi_status = STA_CONNECTING;
		esp_wifi_connect();
	}
	
    return JAVACALL_WOULD_BLOCK;
}

/**
 * See javacall_network.h for definition.
 */
javacall_result javacall_network_init_finish() {	
	switch (wifi_status) {
	case STA_CONNECTED:
		return JAVACALL_OK;
	case STA_START:
	case STA_CONNECTING:
		return JAVACALL_WOULD_BLOCK;
	case STA_DISCONNECTED:
	case STA_UNINITIALIZED:
		return JAVACALL_FAIL;
	}
}

/**
 * Performs platform-specific finalization of the networking system.
 * Will be called ONCE during VM shutdown.
 *
 * @retval JAVACALL_WOULD_BLOCK caller must call xxx_finalize_finish
 *         function to complete the operation
 * @retval JAVACALL_OK      success
 * @retval JAVACALL_FAIL    fail
 */
javacall_result javacall_network_finalize_start(void)
{
    
	return JAVACALL_FAIL;
}

/**
 * Finishes platform-specific finalize of the networking system.
 * The function is invoked be the JVM after receiving JAVACALL_NETWORK_DOWN
 * notification from the platform.
 * @retval JAVACALL_OK      success
 * @retval JAVACALL_FAIL    fail
 */
javacall_result javacall_network_finalize_finish(void)
{
    
	return JAVACALL_FAIL;
}

/**
 * See pcsl_network.h for definition.
 */
int javacall_network_error(void *handle){

   //javacall_printf("javacall_network_error: trace\n");
   
   (void)handle;
    return 0;
}


/**
 * See pcsl_network.h for definition.
 */
javacall_result javacall_network_get_local_host_name(char *pLocalHost){
    return JAVACALL_FAIL;
}

/**
 * See pcsl_network.h for definition.
 */
javacall_result /*OPTIONAL*/ javacall_server_socket_open_start(
    javacall_configuration_id config_id,
    javacall_ip_version ip_version,
	int port, 
	void **pHandle, 
	void **pContext) {    

    return JAVACALL_CONNECTION_NOT_FOUND; 
}

/**
 * See pcsl_network.h for definition.
 */
javacall_result javacall_server_socket_open_finish(void *handle, void *context){

	return JAVACALL_FAIL;
}

/**
 * See javacall_socket.h for definition.
 */
javacall_result /*OPTIONAL*/ javacall_server_socket_accept_start(
      javacall_handle handle, 
      javacall_handle *pNewhandle) {
   return JAVACALL_FAIL;
}

/**
 * See javacall_socket.h for definition.
 */
javacall_result /*OPTIONAL*/ javacall_server_socket_accept_finish(
	javacall_handle handle, 
	javacall_handle *pNewhandle) {

   return JAVACALL_FAIL;
}

/**
 * See javacall_network.h for definition.
 */
javacall_result javacall_server_socket_set_notifier(javacall_handle handle, javacall_bool set) {
	return JAVACALL_FAIL;
}

/**
 * Gets the http / https proxy address. This method is
 * called when the <tt>com.sun.midp.io.http.proxy</tt> or 
 <tt>com.sun.midp.io.https.proxy</tt> internal property
 * is retrieved.
 *
 * @param pHttpProxy base of char array to receive the hostname followed 
 *          by ':' and port. - ex) webcache.thecompany.com:8080.  
 *          Size of the pHttpProxy should be (MAX_HOST_LENGTH + 6).
 * @param pHttpsProxy base of char array to receive the hostname followed 
 *          by ':' and port. - ex) webcache.thecompany.com:8080.  
 *          Size of the pHttpsProxy should be (MAX_HOST_LENGTH + 6).
 *
 * @retval JAVACALL_OK      success
 * @retval JAVACALL_FAIL    if there is a network error
 */
javacall_result /*OPTIONAL*/ javacall_network_get_http_proxy(/*OUT*/ char *pHttpProxy) {
    
	return JAVACALL_FAIL;
}

/**
 * Gets the port number of the local socket endpoint.
 *
 * @param handle handle of an open connection
 * @param pPortNumber returns the local port number
 * 
 * @retval JAVACALL_OK      success
 * @retval JAVACALL_FAIL    if there was an error
 */
javacall_result /*OPTIONAL*/ javacall_socket_getlocalport(
        javacall_handle handle,
        int *pPortNumber) {
	//javacall_printf("javacall_socket_getlocalport: trace\n");

	int fd = GetFD(handle);
    int status;
    struct sockaddr_in sa;
    int saLen = sizeof (sa);

    sa.sin_family = AF_INET;
    status = lwip_getsockname(fd, (struct sockaddr*)&sa, &saLen);

    if (status == -1) {
		//javacall_printf("javacall_socket_getlocalport: JAVACALL_FAIL\n");
        return JAVACALL_FAIL;
    }

    *pPortNumber = PP_NTOHS(sa.sin_port);

	//javacall_printf("javacall_socket_getlocalport: JAVACALL_OK\n");
    return JAVACALL_OK;
}

/**
 * Gets the port number of the remote socket endpoint.
 *
 * @param handle handle of an open connection
 * @param pPortNumber returns the local port number
 * 
 * @retval JAVACALL_OK      success
 * @retval JAVACALL_FAIL    if there was an error
 */
javacall_result javacall_socket_getremoteport(
    void *handle,
    int *pPortNumber)
{
    return JAVACALL_FAIL;
}

/**
 * Gets the IP address of the local socket endpoint.
 *
 * @param handle handle of an open connection
 * @param pAddress base of byte array to receive the address
 *
 * @retval JAVACALL_OK      success
 * @retval JAVACALL_FAIL    if there was an error
 */
javacall_result /*OPTIONAL*/ javacall_socket_getlocaladdr(
    javacall_handle handle,
    char *pAddress)
{
    return JAVACALL_FAIL;
}

/**
 * Gets the IP address of the remote socket endpoint.
 *
 * @param handle handle of an open connection
 * @param pAddress base of byte array to receive the address
 *
 * @retval JAVACALL_OK      success
 * @retval JAVACALL_FAIL    if there was an error
 */
javacall_result /*OPTIONAL*/ javacall_socket_getremoteaddr(
    void *handle,
    char *pAddress)
{
    return JAVACALL_FAIL;
}

char* javacall_inet_ntoa(javacall_ip_version ip_version, void *address) {
    return inet_ntoa(*((struct in_addr*)address));
}
