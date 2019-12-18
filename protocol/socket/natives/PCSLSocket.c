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

// Note the "&& 0" here. As of 2004/08/25 this code compiles with the
// PCSL network library on linux/i386. However, the only existing PCSL
// implementation requires QT, which is not used by the vanilla
// VM, so it will fail in the linking stage.  As a result, I
// have temporarily disabled PCSLSocket.cpp.
//
// If you wish to link a stand-alone VM with the PCSL library
// on your platform, please remove the "&& 0" in this file and the "||
// 1" on top of BSDSocket.cpp.
//
// Usually the PCSL network library is used in the MIDP layer. The
// only reason for you to compile on this file is to build a
// stand-alone VM, without MIDP, in order to execute the CLDC-TCK.
#include <jvm.h>
#include <kni.h>
#include <kni_globals.h>
#include <sni.h>
#include <sni_event.h>

#include <pcsl_network.h>
#include <josh_logging.h>

#ifndef NULL
#define NULL 0
#endif

static int inited = 0;

KNIEXPORT KNI_RETURNTYPE_INT
Java_com_sun_cldc_io_j2me_socket_Protocol_open0() {
	void *pcslHandle = INVALID_HANDLE;
	int status;
	void* context = NULL;
	SNIReentryData* info;
	int res;

	int in_addr = KNI_GetParameterAsInt(1);
	int port = KNI_GetParameterAsInt(2);

  	if (!inited) {
		info = (SNIReentryData*)SNI_GetReentryData(NULL);
		if (info == NULL) {
    		res = pcsl_network_init_start(NULL);
			REPORT_INFO(LC_PROTOCOL, "pcsl_network_init_start returns\n");
		} else {
			if (info->status == 0) {
				res = pcsl_network_init_finish();
				REPORT_INFO(LC_PROTOCOL, "pcsl_network_init_finish returns\n");
			} else {
				REPORT_INFO(LC_PROTOCOL, "pcsl_network_init_start error\n");
				res = PCSL_NET_IOERROR;
			}
		}
		if (res == PCSL_NET_WOULDBLOCK) {
			REPORT_INFO(LC_PROTOCOL, "Would block on network up signal...\n");
			SNIEVT_wait(NETWORK_UP_SIGNAL, (int)0, NULL);
		} else if (res == PCSL_NET_SUCCESS) {
			REPORT_INFO(LC_PROTOCOL, "Network up!\n");
			inited = 1;
			{
	            SNI_BEGIN_RAW_POINTERS;
	            status = pcsl_socket_open_start(
	                     (unsigned char*)&in_addr,
	                     port, &pcslHandle, &context);
	            SNI_END_RAW_POINTERS;

	            if (status == PCSL_NET_SUCCESS) {
					
	            } else if (status == PCSL_NET_IOERROR) {
	                KNI_ThrowNew(KNIIOException, "IOError in socket open");
	            } else if (status == PCSL_NET_CONNECTION_NOTFOUND) {
	                KNI_ThrowNew(KNIConnectionNotFoundException, "ConnectionNotFound error in socket open");
	            } else if (status == PCSL_NET_WOULDBLOCK) {
	                SNIEVT_wait(NETWORK_WRITE_SIGNAL, (int)pcslHandle,
	                    context);
	            } else {
	                KNI_ThrowNew(KNIIOException, NULL);
	            }
	        }			
		} else {
			KNI_ThrowNew(KNIIOException, "Initialize network error!\n");
		}
  	} else {
	    info = (SNIReentryData*)SNI_GetReentryData(NULL);
	    if (info == NULL) {   /* First invocation */

	        {
	            SNI_BEGIN_RAW_POINTERS;
	            status = pcsl_socket_open_start(
	                     (unsigned char*)&in_addr,
	                     port, &pcslHandle, &context);
	            SNI_END_RAW_POINTERS;

	            if (status == PCSL_NET_SUCCESS) {
					
	            } else if (status == PCSL_NET_IOERROR) {
	                KNI_ThrowNew(KNIIOException, "IOError in socket open");
	            } else if (status == PCSL_NET_CONNECTION_NOTFOUND) {
	                KNI_ThrowNew(KNIConnectionNotFoundException, "ConnectionNotFound error in socket open");
	            } else if (status == PCSL_NET_WOULDBLOCK) {
	                SNIEVT_wait(NETWORK_WRITE_SIGNAL, (int)pcslHandle,
	                    context);
	            } else {
	                KNI_ThrowNew(KNIIOException, NULL);
	            }
	        }
	    } else {  /* Reinvocation after unblocking the thread */
	        pcslHandle = (void *) info->descriptor;
	        context = (void *)info->pContext;
			status = info->status;

			if (status == PCSL_NET_SUCCESS) {
	        	status = pcsl_socket_open_finish(pcslHandle, context);
			}

	        if (status == PCSL_NET_SUCCESS) {
	            
	        } else if (status == PCSL_NET_WOULDBLOCK) {
	            SNIEVT_wait(NETWORK_WRITE_SIGNAL, (int)pcslHandle, context);
	        } else  {            
	        	void* dummy;
	        	pcsl_socket_close_start(pcslHandle, &dummy);
	            KNI_ThrowNew(KNIConnectionNotFoundException, "error in socket open");
	        }
	    }    	
	}
	KNI_ReturnInt((jint)pcslHandle);
}


KNIEXPORT KNI_RETURNTYPE_INT
Java_com_sun_cldc_io_j2me_socket_Protocol_getHostByName0() {

	void *pcslHandle = INVALID_HANDLE;
    int status;
    void* context = NULL;
    SNIReentryData* info;
	int in_address = 0;
	int in_len;
	int res;

	KNI_StartHandles(1);
	
	KNI_DeclareHandle(hostname_object);
	KNI_GetParameterAsObject(1, hostname_object);
	// hostname is always NUL terminated. See socket/Protocol.java for detail.
	unsigned char *hostname = (unsigned char*) SNI_GetRawArrayPointer(hostname_object);


	if (!inited) {
		info = (SNIReentryData*)SNI_GetReentryData(NULL);
		if (info == NULL) {
    		res = pcsl_network_init_start(NULL);
			REPORT_INFO(LC_PROTOCOL, "pcsl_network_init_start returns\n");
		} else {
			if (info->status == 0) {
				res = pcsl_network_init_finish();
				REPORT_INFO(LC_PROTOCOL, "pcsl_network_init_finish returns\n");
			} else {
				REPORT_INFO(LC_PROTOCOL, "pcsl_network_init_start error\n");
				res = PCSL_NET_IOERROR;
			}
		}
		if (res == PCSL_NET_WOULDBLOCK) {
			REPORT_INFO(LC_PROTOCOL, "Would block on network up signal...\n");
			SNIEVT_wait(NETWORK_UP_SIGNAL, (int)0, NULL);
		} else if (res == PCSL_NET_SUCCESS) {
			REPORT_INFO(LC_PROTOCOL, "Network up!\n");
			inited = 1;
			{
	            SNI_BEGIN_RAW_POINTERS;
	            status = pcsl_network_gethostbyname_start(
	                     (char*)hostname, (unsigned char*)&in_address, 4, &in_len, &pcslHandle, &context);
	            SNI_END_RAW_POINTERS;

	            if (status == PCSL_NET_SUCCESS) {
					
	            } else if (status == PCSL_NET_IOERROR) {
	                KNI_ThrowNew(KNIIOException, "IOError in socket gethostbyname");
	            } else if (status == PCSL_NET_WOULDBLOCK) {
	                SNIEVT_wait(HOST_NAME_LOOKUP_SIGNAL, (int)pcslHandle,
	                    context);
	            } else {
	                KNI_ThrowNew(KNIIOException, NULL);
	            }
	        }			
		} else {
			KNI_ThrowNew(KNIIOException, "Initialize network error!\n");
		}
  	} else {
	    info = (SNIReentryData*)SNI_GetReentryData(NULL);
	    if (info == NULL) {   /* First invocation */

	        {
	            SNI_BEGIN_RAW_POINTERS;
	            status = pcsl_network_gethostbyname_start(
	                     (char*)hostname, (unsigned char*)&in_address, 4, &in_len, &pcslHandle, &context);
	            SNI_END_RAW_POINTERS;

	            if (status == PCSL_NET_SUCCESS) {
					
	            } else if (status == PCSL_NET_IOERROR) {
	                KNI_ThrowNew(KNIIOException, "IOError in socket gethostbyname");
	            } else if (status == PCSL_NET_WOULDBLOCK) {
	                SNIEVT_wait(HOST_NAME_LOOKUP_SIGNAL, (int)pcslHandle,
	                    context);
	            } else {
	                KNI_ThrowNew(KNIIOException, NULL);
	            }
	        }
	    } else {  /* Reinvocation after unblocking the thread */
	        pcslHandle = (void *) info->descriptor;
	        context = (void *)info->pContext;
			status = info->status;

			if (status == PCSL_NET_SUCCESS) {
	        	status = pcsl_network_gethostbyname_finish((unsigned char*)&in_address, 4, &in_len, pcslHandle, context);
			}

	        if (status == PCSL_NET_SUCCESS) {
	        } else if (status == PCSL_NET_WOULDBLOCK) {
	            SNIEVT_wait(HOST_NAME_LOOKUP_SIGNAL, (int)pcslHandle, context);
	        } else  {            
	            KNI_ThrowNew(KNIConnectionNotFoundException, "error in socket gethostbyname");
	        }
    	}
  	}

    KNI_EndHandles();
    KNI_ReturnInt(in_address);
}


static int do_pcsl_read(void *handle, char *buffer, int length) {
	SNIReentryData* info;
	void *context;
	int nread;  
	int status;

	info = (SNIReentryData*)SNI_GetReentryData(NULL);
	if (info == NULL) {   /* First invocation */
		status = pcsl_socket_read_start(handle, (unsigned char*)buffer, length, 
			                            &nread, &context);
  	} else {
  		context = info->pContext;
		status = info->status;
		if (status == PCSL_NET_SUCCESS) {
		    status = pcsl_socket_read_finish(handle, (unsigned char*) buffer, length,
                                         &nread, context);
	    }
	}

	if (status == PCSL_NET_WOULDBLOCK) {
		SNIEVT_wait(NETWORK_READ_SIGNAL, (int)handle, context);
		return 0;
  	} else if (status == PCSL_NET_SUCCESS) {
    	return nread;
  	} else if (status == PCSL_NET_INTERRUPTED) {
    	return -1;
  	} else {
  		return -2;
  	}
}

static int do_pcsl_write(void *handle, char *buffer, int length) {
  SNIReentryData* info;
  void *context;
  int nwrite;  
  int status;
  
  info = (SNIReentryData*)SNI_GetReentryData(NULL);
  if (info == NULL) {	/* First invocation */
	  status = pcsl_socket_write_start(handle, buffer, length, 
	  	                               &nwrite, &context);
  } else {
  	  context = info->pContext;
	  status = info->status;
	  if (status == PCSL_NET_SUCCESS) {
	  	status = pcsl_socket_write_finish(handle, buffer, length,
									   &nwrite, context);
      } 
  }
  
  if (status == PCSL_NET_WOULDBLOCK) {
	  SNIEVT_wait(NETWORK_WRITE_SIGNAL, (int)handle, context);
	  return 0;
  } else if (status == PCSL_NET_SUCCESS) {
	  return nwrite;
  } else if (status == PCSL_NET_INTERRUPTED){
	  return -1;
  } else {
  	  return -2;
  }

}

KNIEXPORT KNI_RETURNTYPE_INT
Java_com_sun_cldc_io_j2me_socket_Protocol_readBuf() {
  void *handle = (void*)KNI_GetParameterAsInt(1);
  int offset = KNI_GetParameterAsInt(3);
  int length = KNI_GetParameterAsInt(4);
  int result;

  KNI_StartHandles(1);
  {
    KNI_DeclareHandle(buffer_object);
    KNI_GetParameterAsObject(2, buffer_object);
    char *buffer = (char *) SNI_GetRawArrayPointer(buffer_object) + offset;

    result = do_pcsl_read(handle, buffer, length);
	if (result == -1) {
		KNI_ThrowNew(KNIInterruptedIOException, "socket reading operation has been interrupted");
	} else if (result == 0) {
		result = -1;
	} else if (result < -1) {
		KNI_ThrowNew(KNIIOException, "socket read error");
	}
  }
  KNI_EndHandles();
  KNI_ReturnInt(result);
}

KNIEXPORT KNI_RETURNTYPE_INT
Java_com_sun_cldc_io_j2me_socket_Protocol_readByte() {
  void *handle = (void*)KNI_GetParameterAsInt(1);
  unsigned char c;
  int res = do_pcsl_read(handle, (char*)&c, 1);
  if (res == -1) {
  	KNI_ThrowNew(KNIInterruptedIOException, "socket reading operation has been interrupted");    
  } else if (res == 0) {
  	res = -1;
  } else if (res < -1) {
  	KNI_ThrowNew(KNIIOException, "socket read error");
  } else {
	KNI_ReturnInt((int)c);
  }
  KNI_ReturnInt(res);
}

KNIEXPORT KNI_RETURNTYPE_INT
Java_com_sun_cldc_io_j2me_socket_Protocol_writeBuf() {
  void *handle = (void*)KNI_GetParameterAsInt(1);
  int offset = KNI_GetParameterAsInt(3);
  int length = KNI_GetParameterAsInt(4);
  int result;

  KNI_StartHandles(1);
  {
    KNI_DeclareHandle(buffer_object);
    KNI_GetParameterAsObject(2, buffer_object);
    char *buffer = (char *) SNI_GetRawArrayPointer(buffer_object) + offset;

    result = do_pcsl_write(handle, buffer, length);
	if (result == -1) {
  		KNI_ThrowNew(KNIInterruptedIOException, "socket writing operation has been interrupted");    
  	} else if (result < -1) {
  		KNI_ThrowNew(KNIIOException, "socket write error");
  	}
  }
  KNI_EndHandles();
  KNI_ReturnInt(result);
}

KNIEXPORT KNI_RETURNTYPE_INT
Java_com_sun_cldc_io_j2me_socket_Protocol_writeByte() {
  void *handle = (void*)KNI_GetParameterAsInt(1);
  char byte = (char) KNI_GetParameterAsInt(2);

  int result = do_pcsl_write(handle, &byte, 1);
  if (result == -1) {
  	KNI_ThrowNew(KNIInterruptedIOException, "socket writing operation has been interrupted");
  } else if (result < -1) {
  	KNI_ThrowNew(KNIIOException, "socket write error");
  }

  return result;
}

KNIEXPORT KNI_RETURNTYPE_INT
Java_com_sun_cldc_io_j2me_socket_Protocol_available0() {
  // unsupported. Not used by CLDC TCK
  KNI_ReturnInt(0);
}

KNIEXPORT KNI_RETURNTYPE_VOID
Java_com_sun_cldc_io_j2me_socket_Protocol_close0() {
  void *handle = (void*)KNI_GetParameterAsInt(1);
  void *context;
  int status;
  SNIReentryData *info;
  
  info = (SNIReentryData*)SNI_GetReentryData(NULL);
  if (info == NULL) {	/* First invocation */
	  status = pcsl_socket_close_start(handle, &context);
  } else {
	  status = pcsl_socket_close_finish(handle, context);
  }
  
  if (status == PCSL_NET_WOULDBLOCK) {
	  SNIEVT_wait(NETWORK_READ_SIGNAL, (int)handle, context);		  
  }

  KNI_ReturnVoid();
}

extern "C" void notify_network_down() {
	inited = 0;
}
