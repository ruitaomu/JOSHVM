#include "javacall_events.h"
#include "javacall_logging.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#ifdef __cplusplus
extern "C" {
#endif

#define QUEUE_LENGTH 16
#define ITEM_SIZE 16

StaticQueue_t xQueueBuffer;
uint8_t ucQueueStorage[ QUEUE_LENGTH * ITEM_SIZE ];
QueueHandle_t xQueue;


/**
 * The function javacall_events_init is called during Java VM startup, allowing the
 * platform to perform specific initializations.
 *
 * @retval JAVACALL_OK      success
 * @retval JAVACALL_FAIL    fail
 */
javacall_result javacall_events_init(void){



	xQueue = xQueueCreateStatic(QUEUE_LENGTH, ITEM_SIZE, &(ucQueueStorage[0]), &xQueueBuffer);

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
 * Waits for an incoming event message and copies it to user supplied
 * data buffer
 * @param timeTowaitInMillisec max number of milliseconds to wait,
 *              -1 means wait forever
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
    
	if (xQueueReceive(xQueue, binaryBuffer, timeTowaitInMillisec/portTICK_PERIOD_MS)) {
		return JAVACALL_OK;
	} else {
	    return JAVACALL_FAIL;
	}
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

	BaseType_t xHigherPriorityTaskWoken;
	if (xQueueSendFromISR(xQueue, binaryBuffer, &xHigherPriorityTaskWoken)) {
		if( xHigherPriorityTaskWoken ) {
	       portYIELD_FROM_ISR();
	    }
		return JAVACALL_OK;
	} else {
		javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_EVENTS, "javacall event queue send failed, queue full?\n");
		return JAVACALL_FAIL;
	}
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

