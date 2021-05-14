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

#ifdef __cplusplus
extern "C" {
#endif

#define MSG_MAXSIZE 16
#define MESSAGE_BLOCKS_NUM 64

typedef struct _MsgBlock {
	int len;
	unsigned char msg[MSG_MAXSIZE];
} MsgBlock;

static MsgBlock* pGet;
static MsgBlock* pPut;
static MsgBlock* pLastBlock;
static MsgBlock* pFirstBlock;
static volatile int isMsgFull;
static volatile int isMsgAvailable;

static MsgBlock messages[MESSAGE_BLOCKS_NUM];

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

	unsigned int tickStart = HAL_GetTick();
	
	while (!isMsgAvailable) {
		if (timeTowaitInMillisec < 0) {
			continue;
		} else if (timeTowaitInMillisec == 0) {
			return JAVACALL_FAIL;
		}
		
		unsigned int tick = HAL_GetTick();
		if (tick < tickStart) {
			//Rewinded. Simpley return JAVACALL_FAIL
			return JAVACALL_FAIL;
		}

		if (tick - tickStart >= timeTowaitInMillisec) {
			return JAVACALL_FAIL;
		}
	}
	
	if (pGet->len > binaryBufferMaxLen) {
		return JAVACALL_OUT_OF_MEMORY;
	}

	binaryBufferMaxLen = pGet->len;
	*outEventLen = binaryBufferMaxLen;
	
	unsigned char* p = pGet->msg;
	while (binaryBufferMaxLen--) {
		*binaryBuffer++ = *p++;
	}

	if (pGet == pLastBlock) {
		pGet = pFirstBlock;
	} else {
		pGet++;
	}

	isMsgFull = 0;

	if (pPut == pGet) {
		isMsgAvailable = 0;
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

	if (!isMsgFull) {
		if (binaryBufferLen > MSG_MAXSIZE) {
			return JAVACALL_FAIL;
		} else {
			unsigned char* p = pPut->msg;

			pPut->len = binaryBufferLen;
			while (binaryBufferLen--) {
				*p++ = *binaryBuffer++;
			}
			
			if (pPut == pLastBlock) {
				pPut = pFirstBlock;
			} else {
				pPut++;
			}

			if (pPut == pGet) {
				isMsgFull = 1;
			}
	
			isMsgAvailable = 1;
			return JAVACALL_OK;
		}
	} else {
		return JAVACALL_FAIL;
	}
}    



/**
 * The function javacall_events_init is called during Java VM startup, allowing the
 * platform to perform specific initializations.
 *
 * @retval JAVACALL_OK      success
 * @retval JAVACALL_FAIL    fail
 */
javacall_result javacall_events_init(void){
	pGet = &messages[0];
	pPut = pGet;
	pLastBlock = &messages[MESSAGE_BLOCKS_NUM - 1];
	pFirstBlock = pGet;
	isMsgFull = 0;
	isMsgAvailable = 0;

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


