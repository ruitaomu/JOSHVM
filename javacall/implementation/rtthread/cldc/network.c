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

#include <rtthread.h>
#include <lwip/sockets.h>
#include <lwip/netdb.h>

#if RT_THREAD_PRIORITY_MAX == 8
#define THREAD_PRIORITY        6
#elif RT_THREAD_PRIORITY_MAX == 32
#define THREAD_PRIORITY        25
#elif RT_THREAD_PRIORITY_MAX == 256
#define THREAD_PRIORITY        200
#endif
#define THREAD_STACK_SIZE    1024
#define THREAD_TIMESLICE    5

#define TC_STAT_END        0x00
#define TC_STAT_RUNNING    0x01
#define TC_STAT_FAILED    0x10
#define TC_STAT_PASSED    0x00

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

typedef struct {
	javacall_handle fd;
	javacall_event_type event;
} socket_event_type;

typedef struct {
	char* hostname;
	int   h_addrtype;
    int   h_length;
	char  addr[MAX_HOST_ADDR_LENGTH];
} gethostbyname_evt_t;

/* For use by pcsl_network_error. */
static int netinit = 0;

int close_socket(javacall_handle handle){
	return lwip_close((int)handle);
}

// for test only
#if 0
#define PCSL_NET_SUCCESS 0
#define PCSL_NET_WOULDBLOCK -1
#define PCSL_NET_IOERROR -2
#define PCSL_NET_INTERRUPTED -3
#define PCSL_NET_CONNECTION_NOTFOUND -4
#define PCSL_NET_INVALID -5
#define PCSL_NET_NOSTATUS -6
#define INVALID_HANDLE ((void*)-1)
#define MAX_ADDR_LENGTH 4
#define MAX_HOST_LENGTH 256

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
			javacall_print("Uncaptured Network Exception Signal\n");
		}
	
	javacall_event_send((unsigned char*)&rd, sizeof(SNIReentryData));
}

// end of test
#endif

javacall_result javacall_socket_open(javacall_ip_version ip_version,
                                     javacall_handle* pHandle) {

	javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_NETWORK,  "javacall_socket_open: trace\n");
	
	int fd = -1;
	int falsebuf = 0;
	int status;
	int flags;
	unsigned long arg = 1;

	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1) {
		return JAVACALL_FAIL;
	}

	/*status = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
	                    (char*)&falsebuf, sizeof (falsebuf));
	if (status == -1) {
		close_socket((javacall_handle)fd);
		return JAVACALL_FAIL;
	}*/

	*pHandle = (javacall_handle)fd;
	
	javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_NETWORK,  "javacall_socket_open: fd=%d\n", fd);
	return JAVACALL_OK;
}

/**
 * See pcsl_network.h for definition.
 */
 
typedef struct {
	int fd;
	struct sockaddr_in addr;
	int ret;
} socket_connect_context_t;	
 
static void javacall_socket_connect_start_thread_entry(void* parameter){
	socket_connect_context_t *c = (socket_connect_context_t*)parameter;
	javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_NETWORK, "javacall_socket_connect_start_thread_entry: fd=%d\n", c->fd);	
	c->ret = connect(c->fd, (struct sockaddr *)&(c->addr), sizeof(c->addr));
	javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_NETWORK, "javacall_socket_connect_start_thread_entry: ret=%d, fd=%d\n", c->ret, c->fd);

	javanotify_socket_event(JAVACALL_EVENT_SOCKET_CONNECT_COMPLETED,
				(javacall_handle)c->fd, JAVACALL_OK);
}

javacall_result javacall_socket_connect_start(
	javacall_handle handle,
    javacall_configuration_id config_id,
    javacall_ip_version ip_version,
    unsigned char *ipBytes, 
	int port, 
	void **pContext)
{
	javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_NETWORK, "javacall_socket_connect_start: trace\n");

	int fd = (int)handle;
	int status;
	struct sockaddr_in addr;
	
	if (ipBytes == NULL){
		javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_NETWORK, "javacall_socket_connect_start: JAVACALL_INVALID_ARGUMENT\n");
	    return JAVACALL_INVALID_ARGUMENT;
	}

	if (ip_version != JAVACALL_IP_VERSION_4){
		javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_NETWORK, "javacall_socket_connect_start: JAVACALL_IP_VERSION_4\n");
		return JAVACALL_INVALID_ARGUMENT;
	}

	javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_NETWORK, "javacall_socket_connect_start: ipAddr = %d.%d.%d.%d\n", ipBytes[0], ipBytes[1], ipBytes[2], ipBytes[3]);
	int addr_family, addr_len;
	if (ip_version == JAVACALL_IP_VERSION_4) {
		addr_family = AF_INET;
		addr_len = 4;
	} else {
		javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_NETWORK, "javacall_socket_connect_start: only support JAVACALL_IP_VERSION_4\n");
		return JAVACALL_INVALID_ARGUMENT;
	}

	rt_memset(&addr, 0, sizeof(addr));
	addr.sin_family      = addr_family;
	addr.sin_port        = htons(port);
	rt_memcpy((char*)&addr.sin_addr, (char*)ipBytes, addr_len);

	socket_connect_context_t *c = (socket_connect_context_t*)malloc(sizeof(socket_connect_context_t));
	if (c == NULL) {
		return JAVACALL_FAIL;
	}
	c->fd = fd;
	c->addr = addr;
	*pContext = c;
	rt_thread_t tid = rt_thread_create("socket_connect",
        javacall_socket_connect_start_thread_entry,     
        c,            
        THREAD_STACK_SIZE, THREAD_PRIORITY, THREAD_TIMESLICE);
    if (tid != RT_NULL) {
        rt_thread_startup(tid);
		//javacall_printf("javacall_socket_connect_start: JAVACALL_CONNECTION_NOT_FOUND\n");
		return JAVACALL_WOULD_BLOCK; 
    } else {
    	free(c);
		return JAVACALL_FAIL;
	}
}

/**
 * See pcsl_network.h for definition.
 */
javacall_result javacall_socket_connect_finish(javacall_handle handle,void *context){
	javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_NETWORK,  "javacall_socket_connect_finish: trace\n");

	socket_connect_context_t *c = (socket_connect_context_t*)context;
	int ret = c->ret;
	free(c);
	return ret<0?JAVACALL_FAIL:JAVACALL_OK;
}

/**
 * implementation between read_start() and read_finish().
 */

typedef struct {
	int fd;
	unsigned char *pData;
	int len;
	int ret;
	int err;
} socket_read_context_t;	

static void javacall_socket_read_start_thread_entry(void* parameter){
	socket_read_context_t *c = (socket_read_context_t*)parameter;
	javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_NETWORK,  "Blocking for socket reading...\n");
	c->ret = recv(c->fd, c->pData, c->len, 0);
	c->err = errno;
	javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_NETWORK,  "Socket reading returned\n");
	javanotify_socket_event(JAVACALL_EVENT_SOCKET_RECEIVE, 
				(javacall_handle)c->fd, JAVACALL_OK);
}


/**
 * See pcsl_network.h for definition.
 */
javacall_result javacall_socket_read_start(javacall_handle handle,
                                           unsigned char *pData,
                                           int len,  
                                           int *pBytesRead,
                                           void **pContext){
                                           
    char name[9];
	static int thcount = 0;
	snprintf(name, 9, "soc_r%03d", thcount++);
	int fd = (int)handle;
    int bytesRead;
	int readlen;

	if ((readlen = recv(handle, pData, len, MSG_DONTWAIT)) > 0) {
		javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_NETWORK, "javacall_socket_read_start: %d bytes has been read\n", readlen);
		*pBytesRead = readlen;
		return JAVACALL_OK;
	} else {
		if ((errno != EWOULDBLOCK) && (errno != EAGAIN)) {
			javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_NETWORK, "javacall_socket_read_start: fail errno=%d\n", errno);
			return JAVACALL_FAIL;
		}
	}

	socket_read_context_t *c = (socket_read_context_t*)malloc(sizeof(socket_read_context_t));
	if (c == NULL) {
		return JAVACALL_FAIL;
	}
	c->fd = fd;
	c->pData = (unsigned char *)malloc(len+1);
	if (c->pData == NULL) {
		free(c);
		return JAVACALL_FAIL;
	}
	c->len = len;
	*pContext = c;
	rt_thread_t tid = rt_thread_create(name,
        javacall_socket_read_start_thread_entry,     
        c,            
        THREAD_STACK_SIZE, THREAD_PRIORITY, THREAD_TIMESLICE);
    if (tid != RT_NULL) {
	    rt_thread_startup(tid);
		javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_NETWORK, "javacall_socket_read_start would block, thread created: %s\n", name);
		return JAVACALL_WOULD_BLOCK;
    } else {
    	free(c->pData);
		free(c);
		return JAVACALL_FAIL;
	}
}


/**
 * See pcsl_network.h for definition.
 */
javacall_result javacall_socket_read_finish(javacall_handle handle,
                                            unsigned char *pData,
                                            int len,
                                            int *pBytesRead,
                                            void *context){

	socket_read_context_t *c = (socket_read_context_t*)context;
	int ret = c->ret;

	*pBytesRead = 0;
	
	if (ret<0){
		int err = c->err;
		free(c->pData);
		free(c);
		if (err == EINTR) {
			return JAVACALL_INTERRUPTED;
		} else {
			return JAVACALL_FAIL;
		}
	}
		
	if (ret>0){
		rt_memcpy(pData, c->pData, ret);
		*pBytesRead = ret;
	}
	
	free(c->pData);
	free(c);
	
    return ret<0?JAVACALL_FAIL:JAVACALL_OK;
}

/**
 * Common implementation between write_start() and write_finish().
 */
 
typedef struct {
	int fd;
	unsigned char *pData;
	int len;
	int ret;
	int err;
} socket_write_context_t;	

static void javacall_socket_write_start_thread_entry(void* parameter){
	socket_write_context_t *c = (socket_write_context_t*)parameter;
	c->ret = send(c->fd, c->pData, c->len, 0);
	c->err = errno;
	javanotify_socket_event(JAVACALL_EVENT_SOCKET_SEND, 
				(javacall_handle)c->fd, JAVACALL_OK);
}

/**
 * See pcsl_network.h for definition.
 */
javacall_result javacall_socket_write_start(javacall_handle handle,
                                            char *pData,
                                            int len,
                                            int *pBytesWritten,
                                            void **pContext){

    int fd = (int)handle;
    int bytesSent;
	int writtenlen;

	if ((writtenlen = send(handle, pData, len, MSG_DONTWAIT)) > 0) {
		javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_NETWORK, "javacall_socket_write_start: %d bytes has been written\n", writtenlen);
		*pBytesWritten = writtenlen;
		return JAVACALL_OK;
	} else {
		if ((errno != EWOULDBLOCK) && (errno != EAGAIN)) {
			javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_NETWORK,   "javacall_socket_write_start: fail errno=%d\n", errno);
			return JAVACALL_FAIL;
		}
	}

    socket_write_context_t *c = (socket_write_context_t*)malloc(sizeof(socket_write_context_t));
	if (c == NULL) {
		return JAVACALL_FAIL;
	}
	c->fd = fd;
	c->pData = (char *)malloc(len+1);
	if (c->pData == NULL) {
		free(c);
		return JAVACALL_FAIL;
	}
	c->len = len;
	rt_memcpy(c->pData, pData, len);
	*pContext = c;
	rt_thread_t tid = rt_thread_create("socket_write",
        javacall_socket_write_start_thread_entry,     
        c,            
        THREAD_STACK_SIZE, THREAD_PRIORITY, THREAD_TIMESLICE);
    if (tid != RT_NULL) {
        rt_thread_startup(tid);
		return JAVACALL_WOULD_BLOCK;
    } else {
    	free(c->pData);
		free(c);
		return JAVACALL_FAIL;
	}
}


/**
 * See pcsl_network.h for definition.
 */
javacall_result javacall_socket_write_finish(javacall_handle handle,
                                             char *pData,
                                             int len,
                                             int *pBytesWritten,
                                             void *context){

	socket_write_context_t *c = (socket_write_context_t*)context;
	int ret = c->ret;
	
	if (ret<0){
		int err = c->err;
		free(c->pData);
		free(c);
		if (err == EINTR) {
			return JAVACALL_INTERRUPTED;
		} else {
			return JAVACALL_FAIL;
		}
	}
		
	if (ret>0)
		*pBytesWritten = ret;
	
	free(c->pData);
	free(c);
	
    return ret<0?JAVACALL_FAIL:JAVACALL_OK;
}


/**
 * See pcsl_network.h for definition.
 */
javacall_result javacall_socket_available(javacall_handle handle, int *pBytesAvailable){

	//javacall_printf("javacall_socket_available: trace\n");
	
    int fd = (int)handle;
	int value = -1;
	if (0 == lwip_ioctl(fd, FIONREAD, &value)) {
		*pBytesAvailable = value;
		return JAVACALL_OK;
	} else {
		return JAVACALL_FAIL;
	}
}

/**
 * See pcsl_network.h for definition.
 */
javacall_result javacall_socket_shutdown_output(javacall_handle handle) {

	//javacall_printf("javacall_socket_shutdown_output: trace\n");
	
	int fd = (int)handle;
	int status;

	status = lwip_shutdown(fd, SHUT_WR);
	
	if (status != -1) {
		javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_NETWORK,  "javacall_socket_shutdown_output: JAVACALL_OK\n");
		return JAVACALL_OK;
	}

	javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_NETWORK,  "javacall_socket_shutdown_output: JAVACALL_FAIL\n");
	return JAVACALL_FAIL;

}


/**
 * See pcsl_network.h for definition.
 *
 * Note that this function NEVER returns PCSL_NET_WOULDBLOCK. Therefore, the 
 * finish() function should never be called and does nothing.
 */
javacall_result javacall_socket_close_start(javacall_handle handle, javacall_bool abort,
                                            void **pContext){

	//javacall_printf("javacall_socket_close_start: trace\n");
    (void)pContext;
	
	int fd = (int)handle;

    if (close_socket((javacall_handle)fd) == 0) {
		javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_NETWORK,  "javacall_socket_close_start: JAVACALL_OK\n");
        return JAVACALL_OK;
    }        

	javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_NETWORK,  "javacall_socket_close_start: JAVACALL_FAIL\n");
    return JAVACALL_FAIL;
}


/**
 * See pcsl_network.h for definition.
 */
javacall_result javacall_socket_close_finish(javacall_handle handle,
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
typedef struct {
	char *hostname;
	struct hostent *he;
} network_gethostbyname_context_t;	

static void javacall_socket_gethostbyname_start_thread_entry(void* parameter){
	network_gethostbyname_context_t *c = (network_gethostbyname_context_t*)parameter;
	c->he = gethostbyname(c->hostname);
	//javacall_printf("javacall_socket_gethostbyname_start_thread_entry: he=%d\n", he);
	
	javanotify_socket_event(JAVACALL_EVENT_NETWORK_GETHOSTBYNAME_COMPLETED, 
				(javacall_handle)-1, JAVACALL_OK);
}
 
javacall_result javacall_network_gethostbyname_start(char *hostname,
                                                     javacall_ip_version ip_version,
                                                     javacall_configuration_id id,
                                                     unsigned char *pAddress, 
                                                     int maxLen,
                                                     int *pLen,
                                                     void **pHandle,
                                                     void **pContext){

	//javacall_printf("javacall_network_gethostbyname_start: trace\n");

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

	int hlen = hostname?strlen(hostname):-1;
	if (hlen==-1)
		return JAVACALL_FAIL;
	
	network_gethostbyname_context_t *c = 
			(network_gethostbyname_context_t*)malloc(sizeof(network_gethostbyname_context_t));
	if (c == NULL) {
		return JAVACALL_FAIL;
	}
	c->hostname = (char*)malloc(hlen+1);
	if (c->hostname == NULL) {
		free(c);
		return JAVACALL_FAIL;
	}
	strcpy(c->hostname, hostname);
	javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_NETWORK,   "javacall_network_gethostbyname_start: hostname=%s\n", c->hostname);
	*pContext = c;
	rt_thread_t tid = rt_thread_create("socket_gethostbyname",
        javacall_socket_gethostbyname_start_thread_entry,     
        c,            
        THREAD_STACK_SIZE, THREAD_PRIORITY, THREAD_TIMESLICE);
    if (tid != RT_NULL) {
        rt_thread_startup(tid);
		return JAVACALL_WOULD_BLOCK;	
    } else {
		free(c->hostname);
		free(c);
		return JAVACALL_FAIL;
	}
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
	
	network_gethostbyname_context_t *c = 
			(network_gethostbyname_context_t*)context;
	struct hostent *he = c->he;
	javacall_result result = JAVACALL_FAIL;

	if (he) {
		switch(he->h_addrtype)
	    {
	        case AF_INET:
	        case AF_INET6:
	        	if (maxLen >= he->h_length) {
	        		memcpy(pAddress, he->h_addr, he->h_length);
	        		*pLen = he->h_length;
	        		result = JAVACALL_OK;
	        	}
	        	break;
	    }
	}

	free(c->hostname);
	free(c);

	return result;
}


/**
 * See pcsl_network.h for definition.
 */
javacall_result javacall_network_getsockopt(void *handle,
                                            javacall_socket_option flag,
                                            int *pOptval){

	//javacall_printf("javacall_network_getsockopt: trace\n");
	
    int *fd = (int*)handle;
    int level = SOL_SOCKET;
    int optname;
    int optsize = sizeof(optname);
    struct linger lbuf ;
    void * opttarget = (void *) pOptval ;

    switch (flag) {
    case JAVACALL_SOCK_DELAY: /* DELAY */
        level = IPPROTO_TCP;
        optname = TCP_NODELAY;
        break;

    case JAVACALL_SOCK_LINGER: /* LINGER */
    case JAVACALL_SOCK_ABORT: 
        opttarget = (void *) &lbuf ;
        optsize = sizeof (struct linger);
        optname = SO_LINGER;
        break;

    case JAVACALL_SOCK_KEEPALIVE: /* KEEPALIVE */
        optname = SO_KEEPALIVE;
        break;

    case JAVACALL_SOCK_RCVBUF: /* RCVBUF */
        optname = SO_RCVBUF;
        break;

    case JAVACALL_SOCK_SNDBUF: /* SNDBUF */
        optname = SO_SNDBUF;
        break;

    default: /* Illegal Argument */
        return JAVACALL_INVALID_ARGUMENT;
    }

    if (0 == lwip_getsockopt(*fd, level,  optname, opttarget, &optsize)) {
        if (SO_LINGER == optname) {
            if(JAVACALL_SOCK_LINGER == flag) {
            /* If linger is on return the number of seconds. */
            *pOptval = (lbuf.l_onoff == 0 ? 0 : lbuf.l_linger) ;
            } else { /* if (JAVACALL_SOCK_ABORT == flag) */
                *pOptval = ( ((lbuf.l_onoff == 1) && (lbuf.l_linger == 0)) ? 1 : 0 );
            }
        }
		return JAVACALL_OK;
    }

    return JAVACALL_FAIL;
}


/**
 * See pcsl_network.h for definition.
 */
javacall_result javacall_network_setsockopt(void *handle,
                                            javacall_socket_option flag,
                                            int optval){

	//javacall_printf("javacall_network_setsockopt: trace\n");
	
	int *fd = (int*)handle;
    int    level = SOL_SOCKET;
    int    optsize =  sizeof(optval);
    int    optname;
    struct linger lbuf ;
    void * opttarget = (void *) & optval ;

    switch (flag) {
    case JAVACALL_SOCK_DELAY: /* DELAY */
        level = IPPROTO_TCP;
        optname = TCP_NODELAY;
        break;

    case JAVACALL_SOCK_LINGER: /* LINGER */
        opttarget = (void *) &lbuf ;
        optsize = sizeof (struct linger);
        optname = SO_LINGER;
        if (optval == 0) {
            lbuf.l_onoff = 0;
            lbuf.l_linger = 0;
        } else {
            lbuf.l_onoff = 1;
            lbuf.l_linger = (unsigned short)optval;
        }
        break;

    case JAVACALL_SOCK_KEEPALIVE: /* KEEPALIVE */
        optname = SO_KEEPALIVE;
        break;

    case JAVACALL_SOCK_RCVBUF: /* RCVBUF */
        optname = SO_RCVBUF;
        break;

    case JAVACALL_SOCK_SNDBUF: /* SNDBUF */
        optname = SO_SNDBUF;
        break;

    case JAVACALL_SOCK_ABORT: 
        opttarget = (void *) &lbuf ;
        optsize = sizeof (struct linger);
        optname = SO_LINGER;

        lbuf.l_onoff = 1;
        lbuf.l_linger = (unsigned short)0;
        break;

    default: /* Illegal Argument */
        return JAVACALL_INVALID_ARGUMENT;
    }

    if (0 == lwip_setsockopt(*fd, level,  optname, opttarget, optsize)){
        return JAVACALL_OK;
    }

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

    //javacall_printf("javacall_network_get_local_ip_address_as_string: trace\n");
	return JAVACALL_FAIL;
}

/**
 * See javacall_network.h for definition.
 */
javacall_result javacall_network_init_start(void) {

    if (!netinit) {
		netinit = 1;
    }

#ifdef ENABLE_NETWORK_TRACING
    javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_NETWORK, "network_init_start\n");
#endif
    return JAVACALL_OK;
}

/**
 * See javacall_network.h for definition.
 */
javacall_result javacall_network_init_finish() {
    //javacall_printf("javacall_network_init_finish: trace\n");

#ifdef ENABLE_NETWORK_TRACING
    javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_NETWORK, "network_init_finish\n");
#endif
	return JAVACALL_OK;
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
    //javacall_printf("javacall_network_finalize_start: trace\n");

	netinit = 0;
	
#ifdef ENABLE_NETWORK_TRACING
	javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_NETWORK, "network_finalize_start\n");
#endif

	return JAVACALL_OK;
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
    //javacall_printf("javacall_network_finalize_finish: trace\n");

#ifdef ENABLE_NETWORK_TRACING
	javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_NETWORK, "network_finalize_finish\n");
#endif

	return JAVACALL_OK;
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
    //javacall_printf("javacall_network_get_local_host_name: trace\n");
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

	//javacall_printf("javacall_server_socket_open_start: trace\n");
    return JAVACALL_FAIL; 
}

/**
 * See pcsl_network.h for definition.
 */
javacall_result javacall_server_socket_open_finish(void *handle, void *context){

	//javacall_printf("javacall_server_socket_open_finish: trace\n");
	return JAVACALL_OK;
}

/**
 * See javacall_socket.h for definition.
 */
javacall_result /*OPTIONAL*/ javacall_server_socket_accept_start(
      javacall_handle handle, 
      javacall_handle *pNewhandle) {
   
    //javacall_printf("javacall_server_socket_accept_start: trace\n");
	return JAVACALL_FAIL;
}

/**
 * See javacall_socket.h for definition.
 */
javacall_result /*OPTIONAL*/ javacall_server_socket_accept_finish(
	javacall_handle handle, 
	javacall_handle *pNewhandle) {

    //javacall_printf("javacall_server_socket_accept_finish: trace\n");
	return JAVACALL_FAIL;
}

/**
 * See javacall_network.h for definition.
 */
javacall_result javacall_server_socket_set_notifier(javacall_handle handle, javacall_bool set) {
	//javacall_printf("javacall_server_socket_set_notifier: trace\n");
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
    //javacall_printf("javacall_network_get_http_proxy: trace\n");

	char *proxy;
	
	if (!pHttpProxy)
		return JAVACALL_FAIL;

	proxy = getenv("http_proxy");

	if (!proxy)
		return JAVACALL_FAIL;

	strcpy(pHttpProxy, proxy);

	return JAVACALL_OK;
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

	int fd = (int)handle;
    int status;
    struct sockaddr_in sa;
    int saLen = sizeof (sa);

    sa.sin_family = AF_INET;
    status = getsockname(fd, (struct sockaddr*)&sa, &saLen);

    if (status == -1) {
		javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_NETWORK,   "javacall_socket_getlocalport: JAVACALL_FAIL\n");
        return JAVACALL_FAIL;
    }

    *pPortNumber = ntohs(sa.sin_port);

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
    //javacall_printf("javacall_socket_getremoteport: trace\n");

	int fd = (int)handle;
    int status;
    struct sockaddr_in sa;
    int saLen = sizeof (sa);

    sa.sin_family = AF_INET;
    status = getpeername(fd, (struct sockaddr*)&sa, &saLen);
	if (status == -1) {
        return JAVACALL_FAIL;
    }

    *pPortNumber = ntohs(sa.sin_port);

	//javacall_printf("javacall_socket_getremoteport: JAVACALL_OK\n");
    return JAVACALL_OK;
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
    //javacall_printf("javacall_socket_getlocaladdr: trace\n");

	int fd = (int)handle;
    int status;
    struct sockaddr_in sa;
    int saLen = sizeof (sa);

    sa.sin_family = AF_INET;
    status = getsockname(fd, (struct sockaddr*)&sa, &saLen);
	
    if (status == -1) {
		javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_NETWORK,   "javacall_socket_getlocaladdr: JAVACALL_FAIL\n");
        return JAVACALL_FAIL;
    }

    /*
     * The string returned from inet_ntoa is only valid until the next
     * winsock API call. The string should not be freed.
     */
    strcpy(pAddress, inet_ntoa(sa.sin_addr));

	//javacall_printf("javacall_socket_getlocaladdr: JAVACALL_OK\n");
    return JAVACALL_OK;
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
    //javacall_printf("javacall_socket_getremoteaddr: trace\n");

	int fd = (int)handle;
    int status;
    struct sockaddr_in sa;
    int saLen = sizeof (sa);

    sa.sin_family = AF_INET;
    status = getpeername(fd, (struct sockaddr*)&sa, &saLen);
	
    if (status == -1) {
		
		javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_NETWORK,   "javacall_socket_getremoteaddr: JAVACALL_FAIL\n");
        return JAVACALL_FAIL;
    }

    /*
     * The string returned from inet_ntoa is only valid until the next
     * winsock API call. The string should not be freed.
     */
    strcpy(pAddress, inet_ntoa(sa.sin_addr));
	
	//javacall_printf("javacall_socket_getremoteaddr: JAVACALL_OK\n");
    return JAVACALL_OK;
}

char* javacall_inet_ntoa(javacall_ip_version ip_version, void *address) {
    //javacall_printf("javacall_inet_ntoa: trace\n");
	return inet_ntoa(*((struct in_addr*)address));
}

void javacall_network_config() {
}
