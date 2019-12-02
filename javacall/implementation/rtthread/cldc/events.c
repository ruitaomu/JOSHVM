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
#include "javacall_events.h"
#include "javacall_logging.h"
#include <rtthread.h>



#ifdef __cplusplus
extern "C" {
#endif

typedef struct EventMessage_ {
    struct EventMessage_* next;
    unsigned char* data;
    int dataLen;
}EventMessage;

volatile EventMessage *g_eventqueue_head, *g_eventqueue_tail;
int g_event_init = 0;
static int lastError;

static struct rt_mailbox mb;
static char mb_pool[128];

#define MUTEX_INIT 
#define MUTEX_FIN
#define MUTEX_LOCK rt_enter_critical()
#define MUTEX_UNLOCK rt_exit_critical()

#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif


void eventqueue_init(){
	//javacall_printf("eventqueue_init\n");
	g_eventqueue_head = 0;
	g_eventqueue_tail = 0;
}

void eventqueue_destroy(){
	//javacall_printf("eventqueue_destroy\n");

	EventMessage *e, *c;

	for (e=g_eventqueue_head;e;){
		c = e;
		e = e->next;
		rt_free(c->data);	
		rt_free(c);
	}
}


void eventqueue_enqueue(unsigned char* data, int dataLen){
	//javacall_printf("eventqueue_enqueue\n");

	EventMessage *e = (EventMessage *)rt_malloc(sizeof(EventMessage));
	if (e == NULL) {
		javacall_logging_printf(JAVACALL_LOGGING_WARNING, JC_EVENTS, "Event queue failed for no memory\n");
		return;
	}
	e->data = rt_malloc(dataLen);
	if (e->data == NULL) {
		javacall_logging_printf(JAVACALL_LOGGING_WARNING, JC_EVENTS, "Event queue failed for no memory\n");
		rt_free(e);
		return;
	}
	e->dataLen = dataLen;
	e->next = NULL;
	rt_memcpy(e->data, data, dataLen);

	MUTEX_LOCK;
	if (g_eventqueue_tail){
		g_eventqueue_tail->next = e;
	}else{
		g_eventqueue_head = e;		
	}
	g_eventqueue_tail = e;
	MUTEX_UNLOCK;
}

int eventqueue_dequeue(unsigned char* data, int dataLen){
	//javacall_printf("eventqueue_dequeue\n");

	EventMessage* h;
	int len, ret;
	
	if (g_eventqueue_head == NULL){
		return 0;
	}

	len = min(dataLen, g_eventqueue_head->dataLen);
	ret = len;
	if (len < g_eventqueue_head->dataLen){
		ret = -1; // incomplete data
	}
	
	rt_memcpy(data, g_eventqueue_head->data, len);
	MUTEX_LOCK;
	h = g_eventqueue_head;
	g_eventqueue_head = g_eventqueue_head->next;
	if (g_eventqueue_head == NULL) {
		g_eventqueue_tail = 0;
	}
	MUTEX_UNLOCK;
	rt_free(h->data);
	rt_free(h);
	return ret;

}


int check_for_events(int miliseconds){
	//javacall_printf("check_for_events\n");

	rt_err_t ret;
	rt_uint32_t mail;
	
	switch (miliseconds){
		case -1:
			ret = rt_mb_recv(&mb, (rt_uint32_t*)&mail, RT_WAITING_FOREVER);
			break;
		case 0:
			ret = rt_mb_recv(&mb, (rt_uint32_t*)&mail, 0);
			break;
		default:
			ret = rt_mb_recv(&mb, (rt_uint32_t*)&mail, MILISECS_TO_TICKS(miliseconds));
			break;
	}
	
	if (ret==RT_EOK)
		return 1;

	return 0;
}

void gen_event(){
	//javacall_printf("gen_event\n");

	rt_err_t result = rt_mb_send(&mb, 0);
	if (result == RT_EOK) {
		return;
	} else {
		javacall_logging_printf(JAVACALL_LOGGING_WARNING, JC_EVENTS, "Event mailbox full\n");
	}
}

    
/**
 * Waits for an incoming event message and copies it to user supplied
 * data buffer
 * @param waitForever indicate if the function should block forever
 * @param timeTowaitInMillisec max number of seconds to wait
 *              if waitForever is false
 * @param binaryBuffer user-supplied buffer to copy event to
 * @param binaryBufferMaxLen maximum buffer size that an event can be 
 *              copied to.
 *              If an event exceeds the binaryBufferMaxLen, then the first
 *              binaryBufferMaxLen bytes of the events will be copied
 *              to user-supplied binaryBuffer, and JAVACALL_OUT_OF_MEMORY will 
 *              be returned
 * @param outEventLen user-supplied pointer to variable that will hold actual 
 *              event size received
 *              Platform is responsible to set this value on success to the 
 *              size of the event received, or 0 on failure.
 *              If outEventLen is NULL, the event size is not returned.
 * @return <tt>JAVACALL_OK</tt> if an event successfully received, 
 *         <tt>JAVACALL_FAIL</tt> or if failed or no messages are avaialable
 *         <tt>JAVACALL_OUT_OF_MEMORY</tt> If an event's size exceeds the 
 *         binaryBufferMaxLen 
 */
javacall_result javacall_event_receive(
                            long                     timeTowaitInMillisec,
                            /*OUT*/ unsigned char*  binaryBuffer,
                            /*IN*/  int             binaryBufferMaxLen,
                            /*OUT*/ int*            outEventLen){

	
	
	int event = 0;
	
	if (!g_event_init){
		javacall_events_init();
	}

	if (binaryBuffer == NULL || binaryBufferMaxLen <= 0){
		//javacall_printf("javacall_event_receive, binaryBuffer is NULL or binaryBufferMaxLen <= 0\n");
		return JAVACALL_FAIL;
	}

	event = check_for_events(timeTowaitInMillisec);
	if (event==0){
		//javacall_printf("javacall_event_receive: event not found\n");
		return JAVACALL_FAIL;
	}

	*outEventLen = eventqueue_dequeue(binaryBuffer, binaryBufferMaxLen);
	//javacall_printf("javacall_event_receive: outEventLen=%d\n", *outEventLen);
	if (*outEventLen == 0){
		return JAVACALL_FAIL;
	}else if(*outEventLen == -1){
		return JAVACALL_OUT_OF_MEMORY;
	}
	
    return JAVACALL_OK;
}
/**
 * copies a user supplied event message to a queue of messages
 *
 * @param binaryBuffer a pointer to binary event buffer to send
 *        The platform should make a private copy of this buffer as
 *        access to it is not allowed after the function call.
 * @param binaryBufferLen size of binary event buffer to send
 * @return <tt>JAVACALL_OK</tt> if an event successfully sent, 
 *         <tt>JAVACALL_FAIL</tt> or negative value if failed
 */
javacall_result javacall_event_send(unsigned char* binaryBuffer,
                                    int binaryBufferLen){

	
	if (!g_event_init){
		javacall_events_init();
	}

	if (binaryBuffer == NULL || binaryBufferLen <= 0){
		//javacall_printf("javacall_event_receive, binaryBuffer is NULL or binaryBufferLen <= 0\n");
		return JAVACALL_FAIL;
	}

	eventqueue_enqueue(binaryBuffer, binaryBufferLen);
	gen_event();
	
	return JAVACALL_OK;

	
}    



/**
 * The function javacall_events_init is called during Java VM startup, allowing the
 * platform to perform specific initializations.
 *
 * @retval JAVACALL_OK      success
 * @retval JAVACALL_FAIL    fail
 */
javacall_result javacall_events_init(void){
	//javacall_printf("javacall_events_init\n");

	int ret;

	if (g_event_init)
		return JAVACALL_OK;
	
	MUTEX_INIT;
	MUTEX_LOCK;
	eventqueue_init();
	rt_mb_init(&mb,
		"mbt",             
		&mb_pool[0],       
		sizeof(mb_pool)/4, 
		RT_IPC_FLAG_FIFO); 
	
	g_event_init = 1;
	MUTEX_UNLOCK;

	return JAVACALL_OK;
}


/**
 * The function javacall_lcd_finalize is called during Java VM shutdown,
 * allowing the platform to perform specific events-related shutdown
 * operations.
 *
 * @retval JAVACALL_OK      success
 * @retval JAVACALL_FAIL    fail
 */
javacall_result javacall_events_finalize(void){
	//javacall_printf("javacall_events_finalize\n");

	if (!g_event_init)
		return JAVACALL_OK;

	MUTEX_LOCK;
	eventqueue_destroy();
	rt_mb_detach(&mb);
	g_event_init = 0;
	MUTEX_UNLOCK;
	
	MUTEX_FIN;

	return JAVACALL_OK;                           
}


/**
 * The platform calls this function in slave mode to inform VM of new events.
 */
void javanotify_inform_event(void){
	return;                           
}



/*************************** KEYPRESS *************************************/



/***************************** SMS *************************************/
/*                   1. INCOMING SMS CALLBACK                          */
/*                   2. SMS SENDING RESULT CALLBACK                    */
/***********************************************************************/




/*************************** SOCKET *************************************/

/*************************** NETWORK *************************************/


/*************************** PAUSE/RESUME *************************************/

/*************************** TEXTFIELD *************************************/





/*************************** MMAPI *************************************/
/*                   1. END-OF-MEDIA CALLBACK                          */
/*                   2. DURATION UPDATE CALLBACK                       */
/***********************************************************************/

/******************** MMAPI (2) : DURATION UPDATE CALLBACK *************/


    
#ifdef __cplusplus
}
#endif


