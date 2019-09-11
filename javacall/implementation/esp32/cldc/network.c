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

#define JC_NETWORK_MALLOC(size) malloc(size)

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
    javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_NETWORK, "esp_wifi_start ...\n");
    javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_NETWORK, "connecting to ap SSID:%s password:%s\n",
             wifi_config.sta.ssid, wifi_config.sta.password);
    ESP_ERROR_CHECK(esp_wifi_start() );
    javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_NETWORK, "wifi_init_sta finished.\n");
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

#define MAX_SOCKET_COUNT 10
#define VIRTUAL_FD_MIN 1000

typedef struct {
    int virtual_fd;
    int real_fd;
} javacall_socket_slot;

static javacall_socket_slot socket_slots[MAX_SOCKET_COUNT] = {0};
static int next_virtual_fd = VIRTUAL_FD_MIN;

static int get_free_socket_slot() {
    for (int i = 0; i < MAX_SOCKET_COUNT; i++) {
        if (socket_slots[i].virtual_fd < VIRTUAL_FD_MIN) {
            return i;
        }
    }
    return -1;
}

static int release_socket_slot(int slot_index) {
    socket_slots[slot_index].virtual_fd = 0;
}

static void set_socket_slot(int slot_index, int real_fd) {
    socket_slots[slot_index].virtual_fd = next_virtual_fd;
    socket_slots[slot_index].real_fd = real_fd;

    next_virtual_fd++;
    if (next_virtual_fd < VIRTUAL_FD_MIN) {
        next_virtual_fd = VIRTUAL_FD_MIN;
    }
}

static int get_virtual_fd(int slot_index) {
    return socket_slots[slot_index].virtual_fd;
}

static int get_real_fd(int slot_index) {
    return socket_slots[slot_index].real_fd;
}

static int get_socket_slot_by_virtual_fd(int virtual_fd) {
    for (int i = 0; i < MAX_SOCKET_COUNT; i++) {
        if (socket_slots[i].virtual_fd == virtual_fd) {
            return i;
        }
    }
    return -1;
}

static int socket_available(int fd) {
    int value = -1;
    if (0 == ioctlsocket(fd, FIONREAD, &value)) {
        return value;
    } else {
        return -1;
    }
}

typedef struct {
    char* hostname;
    int   h_addrtype;
    int   h_length;
    char  addr[MAX_HOST_ADDR_LENGTH];
    javacall_result result;
} gethostbyname_evt_t;

static javacall_result getHostByName_blocking(javacall_handle handle) {
    struct hostent *he;
    struct hostent host;
    int err = 0;
    char *buf;
    gethostbyname_evt_t* pEvt = (gethostbyname_evt_t*)handle;
    // int buflen = 32 + strlen(pEvt->hostname) + 1 + (MEM_ALIGNMENT - 1);
    int buflen = 44 + strlen(pEvt->hostname) + 1 + (MEM_ALIGNMENT - 1);

    buf = JC_NETWORK_MALLOC(buflen);
    if (buf == NULL) {
        javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_NETWORK, "javacall_network_gethostbyname_start failed, memory not enough\n");
        return JAVACALL_FAIL;
    }

    int ret = gethostbyname_r(pEvt->hostname, &host, buf, buflen, &he, &err);
    if (ret != 0){
        javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_NETWORK, "javacall_network_gethostbyname_start failed, error=%d\n", err);
        free(buf);
        return JAVACALL_FAIL;
    }

    int addrLen = sizeof(ip4_addr_t);
    if (addrLen > MAX_HOST_ADDR_LENGTH) {
        javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_NETWORK, "javacall_network_gethostbyname_start: too long address length: %d\n", addrLen);
        free(buf);
        return JAVACALL_INVALID_ARGUMENT;
    }

    pEvt->h_length = addrLen;
    memcpy(pEvt->addr, he->h_addr, addrLen);
    pEvt->h_addrtype = he->h_addrtype;
    free(buf);

    return JAVACALL_OK;
}

#define JC_NET_TASK_STACK_SIZE (2048)
#define JC_NET_TASK_PRIORITY (CONFIG_ESP32_PTHREAD_TASK_PRIO_DEFAULT)
#define JC_NET_TASK_QUEUE_SIZE 15
#define JC_NET_TASK_QUEUE_SIZE_DNS 5

typedef struct {
    javacall_handle handle;
    javacall_event_type event;
    int used;
} task_queue_item;

static TaskHandle_t dnsTask = NULL;
static StaticTask_t dnsTaskBuffer;
static StackType_t dnsTaskStack[JC_NET_TASK_STACK_SIZE];
static TaskHandle_t socketTask = NULL;
static StaticTask_t socketTaskBuffer;
static StackType_t socketTaskStack[JC_NET_TASK_STACK_SIZE];
static task_queue_item* taskQueue = NULL;

static void dns_handler(void *arg) {
    const int start = 0;
    const int end = JC_NET_TASK_QUEUE_SIZE_DNS;
    int index;
    int hasReq;

    xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);
    while (true) {
        hasReq = 0;
        for (index = start; index < end; index++) {
            if (taskQueue[index].used) {
                javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_NETWORK,
                    "handle gethost request (%d, %d)\n", index, taskQueue[index].handle);
                javacall_result result = getHostByName_blocking(taskQueue[index].handle);
                gethostbyname_evt_t* pEvt = (gethostbyname_evt_t*)(taskQueue[index].handle);
                pEvt->result = result;
                javanotify_socket_event(
                        JAVACALL_EVENT_NETWORK_GETHOSTBYNAME_COMPLETED,
                        taskQueue[index].handle,
                        JAVACALL_OK);
                taskQueue[index].used = 0;
                hasReq = 1;
            }
        }
        if (!hasReq) {
            xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);
            continue;
        }
    }
}

static void socket_handler(void *arg) {
    const int start = JC_NET_TASK_QUEUE_SIZE_DNS;
    const int end = JC_NET_TASK_QUEUE_SIZE;
    fd_set rfds, wfds, errfds;
    int slot_index;
    int fd;
    int index;
    int maxfd;
    int hasReq;
    struct timeval tv;
    int selectRet;
    javacall_result result = JAVACALL_FAIL;
    javacall_socket_event_type jc_event;

    xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);
    while (true) {
        FD_ZERO(&rfds);
        FD_ZERO(&wfds);
        FD_ZERO(&errfds);

        maxfd = -1;
        hasReq = 0;
        // set fdsets
        for (index = start; index < end; index++) {
            if (taskQueue[index].used) {
                slot_index = get_socket_slot_by_virtual_fd((int)taskQueue[index].handle);
                if (slot_index < 0) {
                    result = JAVACALL_FAIL;
                    if (taskQueue[index].event == EVENT_FD_CONNECT) {
                        jc_event = JAVACALL_EVENT_SOCKET_CONNECT_COMPLETED;
                    }  else if (taskQueue[index].event == EVENT_FD_READ) {
                        jc_event = JAVACALL_EVENT_SOCKET_RECEIVE;
                    } else if (taskQueue[index].event == EVENT_FD_WRITE) {
                        jc_event = JAVACALL_EVENT_SOCKET_SEND;
                    } else {
                        continue;
                    }
                    javanotify_socket_event(jc_event, taskQueue[index].handle, result);
                    taskQueue[index].used = 0;
                    continue;
                }
                fd = get_real_fd(slot_index);
                FD_SET(fd, &rfds);
                FD_SET(fd, &wfds);
                FD_SET(fd, &errfds);
                if (fd > maxfd) {
                    maxfd = fd;
                }
                hasReq = 1;
            }
        }

        if (!hasReq) {
            xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);
            continue;
        }

        tv.tv_sec = 1;
        tv.tv_usec = 0;
        selectRet = select(maxfd + 1, &rfds, &wfds, &errfds, &tv);
        if (selectRet == 0) {
            // time out
            continue;
        } else if (selectRet > 0) {
            // check
            for (index = start; index < end; index++) {
                if (taskQueue[index].used) {
                    slot_index = get_socket_slot_by_virtual_fd((int)taskQueue[index].handle);
                    if (slot_index < 0) {
                        continue;
                    }
                    fd = get_real_fd(slot_index);
                    // javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_NETWORK,
                    //         "check socket request (%d, %d)\n", index, taskQueue[index].handle);
                    if (taskQueue[index].event == EVENT_FD_CONNECT) {
                        jc_event = JAVACALL_EVENT_SOCKET_CONNECT_COMPLETED;
                        if (FD_ISSET(fd, &rfds) || FD_ISSET(fd, &wfds)) {
                            result = JAVACALL_OK;
                        } else if (FD_ISSET(fd, &errfds)) {
                            result = JAVACALL_FAIL;
                        } else {
                            continue;
                        }
                    } else if (taskQueue[index].event == EVENT_FD_READ) {
                        jc_event = JAVACALL_EVENT_SOCKET_RECEIVE;
                        if (FD_ISSET(fd, &rfds)) {
                            result = JAVACALL_OK;
                        } else if (FD_ISSET(fd, &errfds)) {
                            result = JAVACALL_FAIL;
                        } else {
                            continue;
                        }
                    } else if (taskQueue[index].event == EVENT_FD_WRITE) {
                        jc_event = JAVACALL_EVENT_SOCKET_SEND;
                        if (FD_ISSET(fd, &wfds)) {
                            result = JAVACALL_OK;
                        } else if (FD_ISSET(fd, &errfds)) {
                            result = JAVACALL_FAIL;
                        } else {
                            continue;
                        }
                    } else {
                        continue;
                    }

                    javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_NETWORK,
                            "check socket request (%d, %d) result is %d\n",
                            index, taskQueue[index].handle, result);
                    javanotify_socket_event(jc_event, taskQueue[index].handle, result);
                    taskQueue[index].used = 0;
                }
            }
        } else {
            // err
            javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_NETWORK, "select failed. (%d)\n", errno);
        }
    }
}

static int create_task_for_dns() {
    if (dnsTask != NULL) {
        return 0;
    }
    TaskHandle_t xHandle = NULL;
    xHandle = xTaskCreateStatic(dns_handler,
            "josh_dns",
            JC_NET_TASK_STACK_SIZE,
            NULL,
            JC_NET_TASK_PRIORITY,
            dnsTaskStack,
            &dnsTaskBuffer);
    if (xHandle == NULL) {
        return -1;
    }
    dnsTask = xHandle;
    javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_NETWORK, "josh gethost task created.\n");
    return 0;
}

static int create_task_for_socket() {
    if (socketTask != NULL) {
        return 0;
    }
    TaskHandle_t xHandle = NULL;
    xHandle = xTaskCreateStatic(socket_handler,
            "josh_soc",
            JC_NET_TASK_STACK_SIZE,
            NULL,
            JC_NET_TASK_PRIORITY,
            socketTaskStack,
            &socketTaskBuffer);
    if (xHandle == NULL) {
        return -1;
    }
    socketTask = xHandle;
    javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_NETWORK, "josh socket task created.\n");
    return 0;
}

static int create_task(javacall_event_type event) {
    if (event == EVENT_FD_GETHOSTBYNAME) {
        return create_task_for_dns();
    } else {
        return create_task_for_socket();
    }
}

static int create_task_queue() {
    if (taskQueue != NULL) {
        return 0;
    }
    task_queue_item* queue = (task_queue_item*)JC_NETWORK_MALLOC(JC_NET_TASK_QUEUE_SIZE * sizeof(task_queue_item));
    if (queue == NULL) {
        return -1;
    }
    memset(queue, 0, JC_NET_TASK_QUEUE_SIZE * sizeof(task_queue_item));
    taskQueue = queue;
    return 0;
}

static int add_request_to_queue(javacall_handle handle, javacall_event_type event) {
    int start, end;
    TaskHandle_t xHandle;
    if (event == EVENT_FD_GETHOSTBYNAME) {
        start = 0;
        end = JC_NET_TASK_QUEUE_SIZE_DNS;
        xHandle = dnsTask;
    } else {
        start = JC_NET_TASK_QUEUE_SIZE_DNS;
        end = JC_NET_TASK_QUEUE_SIZE;
        xHandle = socketTask;
    }
    for (int index = start; index < end; index++) {
        if (!taskQueue[index].used) {
            taskQueue[index].handle = handle;
            taskQueue[index].event = event;
            taskQueue[index].used = 1;
            javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_NETWORK,
                    "add handle %d, event %d to queue. index=%d.\n", handle, event, index);
            xTaskNotify(xHandle, 0, eNoAction);
            return 0;
        }
    }
    return -1;
}

static int set_event_observer(javacall_handle handle, javacall_event_type event) {
    int result = create_task(event);
    if (result < 0) {
        // create task failed
        javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_NETWORK, "task create failed.\n");
        return -1;
    }

    result = create_task_queue();
    if (result < 0) {
        javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_NETWORK, "task queue create failed.\n");
        return -1;
    }

    result = add_request_to_queue(handle, event);
    if (result < 0) {
        javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_NETWORK, "add network request to queue failed.\n");
        return -1;
    }

    return 0;
}

//=================== Javacall Interface IMPLEMENTATION ========================
javacall_result javacall_socket_open(javacall_ip_version ip_version,
                                     javacall_handle* pHandle) {
    int fd;
    int falsebuf = 0;
    int status;
    int flags;

    int slot_index = get_free_socket_slot();
    if (slot_index < 0) {
        javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_NETWORK,
                "javacall_socket_open() failed to allocate slot.\n");
        return JAVACALL_FAIL;
    }

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_NETWORK,
                "javacall_socket_open() failed to create socket, errno %d\n", errno);
        return JAVACALL_FAIL;
    }

    // make socket non-blocking
    flags = fcntl(fd, F_GETFL, 0);
    status = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    if (status == -1) {
        int ret = closesocket(fd);
        javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_NETWORK,
                "set nonblock failed. socket fd %d closed, returns %d\n", fd, ret);
        return JAVACALL_FAIL;
    }

    set_socket_slot(slot_index, fd);
    *pHandle = (javacall_handle)get_virtual_fd(slot_index);
    javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_NETWORK,
            "javacall_socket_open() returns handle=%d, fd=%d\n", *pHandle, fd);
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
    int fd;
    int status;
    struct sockaddr_in addr;
    int slot_index = get_socket_slot_by_virtual_fd((int)handle);
    if (slot_index < 0) {
        javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_NETWORK,
                "javacall_socket_connect_start: Invalid handle (%d)\n", (int)handle);
        return JAVACALL_FAIL;
    }

    fd = get_real_fd(slot_index);
    javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_NETWORK,
            "javacall_socket_connect_start: handle=%d, fd=%d\n", handle, fd);

    if (ipBytes == NULL) {
        //javacall_printf("javacall_socket_connect_start: JAVACALL_INVALID_ARGUMENT\n");
        return JAVACALL_INVALID_ARGUMENT;
    }

    if (ip_version != JAVACALL_IP_VERSION_4) {
        //javacall_printf("javacall_socket_connect_start: JAVACALL_IP_VERSION_4\n");
        return JAVACALL_INVALID_ARGUMENT;
    }

    //javacall_printf("javacall_socket_connect_start: ipAddr = %d.%d.%d.%d\n", ipBytes[0], ipBytes[1], ipBytes[2], ipBytes[3]);
    int addr_family, addr_len;
    memset(&addr, 0, sizeof(addr));
    addr_family = AF_INET;
    addr_len = 4;

    addr.sin_len = sizeof(addr);
    addr.sin_family      = addr_family;
    addr.sin_port        = PP_HTONS((unsigned short)port);
    memcpy((char*)&addr.sin_addr, (char*)ipBytes, addr_len);

    status = connect(fd, (struct sockaddr *)&addr, sizeof(addr));
    *pContext = NULL;
    if (status == 0) {
        javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_NETWORK,
                "javacall_socket_connect_start: JAVACALL_OK\n");
        return JAVACALL_OK;
    }

    if (errno == EINPROGRESS) {
        //javacall_printf("javacall_socket_connect_start: fd=%d\n", fd);
        if (set_event_observer(handle, EVENT_FD_CONNECT)) {
            return JAVACALL_FAIL;
        }

        javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_NETWORK,
                "javacall_socket_connect_start: JAVACALL_WOULD_BLOCK\n");
        return JAVACALL_WOULD_BLOCK;
    }

    javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_NETWORK,
            "javacall_socket_connect_start: JAVACALL_CONNECTION_NOT_FOUND\n");
    return JAVACALL_CONNECTION_NOT_FOUND;
}

/**
 * See pcsl_network.h for definition.
 */
int javacall_socket_connect_finish(void *handle,void *context) {
    int port;
    javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_NETWORK,
            "javacall_socket_connect_finish: trace\n");
    if (javacall_socket_getlocalport(handle, &port) == JAVACALL_OK) {
        javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_NETWORK,
                "javacall_socket_connect_finish: localport=%d\n", port);
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

    //javacall_printf("socket_read_common: trace, handle=%d\n", handle);

    int fd;
    int bytesRead;
    int slot_index = get_socket_slot_by_virtual_fd((int)handle);
    if (slot_index < 0) {
        javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_NETWORK,
                "socket_read_common: Invalid handle (%d)\n", (int)handle);
        return JAVACALL_FAIL;
    }
    fd = get_real_fd(slot_index);

    bytesRead = recv(fd, (char*)pData, len, 0);
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
                                           void **pContext) {
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
                                            void *context) {
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

    //javacall_printf("socket_write_common: trace, handle=%d\n", handle);

    int fd;
    int bytesSent;
    int slot_index = get_socket_slot_by_virtual_fd((int)handle);
    if (slot_index < 0) {
        javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_NETWORK,
                "socket_write_common: Invalid handle (%d)\n", (int)handle);
        return JAVACALL_FAIL;
    }
    fd = get_real_fd(slot_index);

    bytesSent = send(fd, pData, len, 0);
    javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_NETWORK,
            "socket_write_common: write %d of %d\n", bytesSent, len);

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
                                            void **pContext) {
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
javacall_result javacall_socket_available(void *handle, int *pBytesAvailable) {

    //javacall_printf("javacall_socket_available: trace\n");

    int fd;
    int len;
    int slot_index = get_socket_slot_by_virtual_fd((int)handle);
    if (slot_index < 0) {
        javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_NETWORK,
                "socket_available: Invalid handle (%d)\n", (int)handle);
        return JAVACALL_FAIL;
    }
    fd = get_real_fd(slot_index);

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
                                            void **pContext) {

    //javacall_printf("javacall_socket_close_start: trace\n");
    (void)pContext;

    int fd;
    int slot_index = get_socket_slot_by_virtual_fd((int)handle);
    if (slot_index < 0) {
        javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_NETWORK,
                "javacall_socket_close_start: Invalid handle (%d). Already closed?\n", (int)handle);
        return JAVACALL_OK;
    }
    fd = get_real_fd(slot_index);
    shutdown(fd, SHUT_RDWR);

    if (closesocket(fd) == 0) {
        release_socket_slot(slot_index);
        javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_NETWORK,
                "javacall_socket_close_start: success. %d\n", (int)handle);
        return JAVACALL_OK;
    }

        javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_NETWORK,
                "javacall_socket_close_start: failed. %d\n", (int)handle);
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
                                                     void **pContext) {
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

    gethostbyname_evt_t* p = JC_NETWORK_MALLOC(sizeof(gethostbyname_evt_t));
    if (p == NULL) {
        return JAVACALL_FAIL;
    }

    p->hostname = JC_NETWORK_MALLOC(strlen(hostname)+1);
    if (p->hostname == NULL) {
        free(p);
        return JAVACALL_FAIL;
    }
    strcpy(p->hostname, hostname);

    if (set_event_observer(p, EVENT_FD_GETHOSTBYNAME)) {
        free(p->hostname);
        free(p);
        return JAVACALL_FAIL;
    }

    *pHandle = p;
    // javacall_printf("javacall_network_gethostbyname_start: handle=%d\n", p);
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
                                                      void *context) {
    javacall_result result = JAVACALL_FAIL;

    // javacall_printf("javacall_network_gethostbyname_finish: handle = %d\n", handle);

    gethostbyname_evt_t* pEvt = (gethostbyname_evt_t*)handle;
    if (pEvt->result != JAVACALL_OK) {
        javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_NETWORK, "javacall_network_gethostbyname_finish: Failed to get host addr\n");
    } else if ((pEvt->h_addrtype == AF_INET) && (ip_version == JAVACALL_IP_VERSION_6)) {
        javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_NETWORK, "javacall_network_gethostbyname_finish: Expected IPv6 but got IPv4\n");
    } else if ((pEvt->h_addrtype == AF_INET6) && (ip_version == JAVACALL_IP_VERSION_4)) {
        javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_NETWORK, "javacall_network_gethostbyname_finish: Expected IPv4 but got IPv6\n");
    } else if (maxLen < pEvt->h_length) {
        javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_NETWORK, "javacall_network_gethostbyname_finish: Buffer not enough for host addr\n");
    } else {
        *pLen = pEvt->h_length;
        memcpy(pAddress, pEvt->addr, *pLen);
        result = JAVACALL_OK;
        javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_NETWORK, "javacall_network_gethostbyname_finish: JAVACALL_OK\n");
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
                                            int *pOptval) {
    return JAVACALL_FAIL;
}

/**
 * See pcsl_network.h for definition.
 */
javacall_result javacall_network_setsockopt(void *handle,
                                            javacall_socket_option flag,
                                            int optval) {
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
                                        /*OUT*/ char *pLocalIPAddress) {

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
javacall_result javacall_network_finalize_start(void) {
    return JAVACALL_FAIL;
}

/**
 * Finishes platform-specific finalize of the networking system.
 * The function is invoked be the JVM after receiving JAVACALL_NETWORK_DOWN
 * notification from the platform.
 * @retval JAVACALL_OK      success
 * @retval JAVACALL_FAIL    fail
 */
javacall_result javacall_network_finalize_finish(void) {
    return JAVACALL_FAIL;
}

/**
 * See pcsl_network.h for definition.
 */
int javacall_network_error(void *handle) {
   //javacall_printf("javacall_network_error: trace\n");
    return errno;
}

/**
 * See pcsl_network.h for definition.
 */
javacall_result javacall_network_get_local_host_name(char *pLocalHost) {
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

    int fd;
    int status;
    struct sockaddr_in sa;
    int saLen = sizeof (sa);
    int slot_index = get_socket_slot_by_virtual_fd((int)handle);
    if (slot_index < 0) {
        return JAVACALL_FAIL;
    }
    fd = get_real_fd(slot_index);

    sa.sin_family = AF_INET;
    status = getsockname(fd, (struct sockaddr*)&sa, &saLen);
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
