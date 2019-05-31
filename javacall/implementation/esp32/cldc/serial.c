/**
* @file
*
*
*/

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "javacall_serial.h"
#include "javacall_logging.h"
#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "esp_log.h"

static const char *TAG = "JOSHVM UART";

static const int OPEN = 1;
static const int CLOSE = 0;

#define UART_PIN_TYPE_TX 0
#define UART_PIN_TYPE_RX 1
#define UART_PIN_TYPE_RTS 2
#define UART_PIN_TYPE_CTS 3

#define SUPPORTED_UART_NUM 3
#define PLATFORM_PORT_0 ((javacall_handle)0)
#define PLATFORM_PORT_1 ((javacall_handle)1)
#define PLATFORM_PORT_2 ((javacall_handle)2)


static uart_port_t comm_ports[SUPPORTED_UART_NUM] = {UART_NUM_0, UART_NUM_1, UART_NUM_2};
static int comm_state[SUPPORTED_UART_NUM] = {0, 0, 0};
static int comm_pins[SUPPORTED_UART_NUM][4] = {
								{UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE},
#if ENABLE_ESP32_VOICE_SUPPORT
								{GPIO_NUM_4, GPIO_NUM_12, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE},
#else
								{GPIO_NUM_4, GPIO_NUM_15, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE},
#endif
#if ENABLE_ESP32_VOICE_SUPPORT
								{GPIO_NUM_13, GPIO_NUM_15, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE}
#else
								{GPIO_NUM_26, GPIO_NUM_27, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE}
#endif
							};

static QueueHandle_t comm_queue0, comm_queue1, comm_queue2;
static QueueHandle_t* comm_queues[SUPPORTED_UART_NUM] = {&comm_queue0, &comm_queue1, &comm_queue2};

#define BUF_SIZE (1024)
#define RD_BUF_SIZE (BUF_SIZE)

static void set_comm_state(javacall_handle hPort, int state) {
	if ((int)hPort < 0 || (int)hPort >= SUPPORTED_UART_NUM) 
		return;
	
	comm_state[(int)hPort] = state;
}

static int is_comm_state_open(javacall_handle hPort) {
	return comm_state[(int)hPort];
}

static QueueHandle_t* get_uart_queue(javacall_handle hPort) {
	return comm_queues[(int)hPort];
}

static int get_uart_pin(int type, javacall_handle hPort) {
	return comm_pins[(int)hPort][type];
}

static uart_port_t get_uart_port(javacall_handle hPort) {
	return comm_ports[(int)hPort];
}

static uart_word_length_t get_uart_options_databits(unsigned int options) {
	unsigned int opt_bps = options & (JAVACALL_SERIAL_BITS_PER_CHAR_7 | JAVACALL_SERIAL_BITS_PER_CHAR_8);
	if (opt_bps == JAVACALL_SERIAL_BITS_PER_CHAR_7) {
		return UART_DATA_7_BITS;
	} else {
		return UART_DATA_8_BITS;
	}
}

static uart_parity_t get_uart_options_parity(unsigned int options) {
	unsigned int opt_parity = options & (JAVACALL_SERIAL_ODD_PARITY | JAVACALL_SERIAL_EVEN_PARITY);
	if (opt_parity == JAVACALL_SERIAL_EVEN_PARITY) {
		return UART_PARITY_EVEN;
	} else if (opt_parity == JAVACALL_SERIAL_ODD_PARITY) {
		return UART_PARITY_ODD;
	} else {
		return UART_PARITY_DISABLE;
	}
}

static uart_stop_bits_t get_uart_options_stopbits(unsigned int options) {
	unsigned int opt_stopbits = options & JAVACALL_SERIAL_STOP_BITS_2;
	if (opt_stopbits != 0)	{
		return UART_STOP_BITS_2;
	} else {
		return UART_STOP_BITS_1;
	}
}

static uart_hw_flowcontrol_t get_uart_options_flowctrl(unsigned int options) {
	unsigned int opt_flowcontrol = options & (JAVACALL_SERIAL_AUTO_RTS | JAVACALL_SERIAL_AUTO_CTS);
	if (opt_flowcontrol == JAVACALL_SERIAL_AUTO_RTS) {
		return UART_HW_FLOWCTRL_RTS;
	} else if (opt_flowcontrol == JAVACALL_SERIAL_AUTO_CTS) {
		return UART_HW_FLOWCTRL_CTS;
	} else if (opt_flowcontrol == 0) {
		return UART_HW_FLOWCTRL_DISABLE;
	} else {
		return  UART_HW_FLOWCTRL_CTS_RTS;
	}
}


static void uart_event_task(void *pvParameters) {
    uart_event_t event;
	QueueHandle_t* uart_queue;
    size_t buffered_size;
	int* flag;
	javacall_handle handle = (javacall_handle)pvParameters;
	uart_port_t port = get_uart_port(handle);
    uint8_t* dtmp = (uint8_t*) malloc(RD_BUF_SIZE);

	uart_queue = get_uart_queue(handle);

	if (uart_queue == NULL) {
		return;
	}
	
	//ESP_LOGI(TAG, "uart %d task start", port);	
    for(;;) {
        //Waiting for UART event.
        if(xQueueReceive(*uart_queue, (void * )&event, (portTickType)portMAX_DELAY)) {
		     bzero(dtmp, RD_BUF_SIZE);
            //ESP_LOGI(TAG, "uart[%d] event:", port);
            switch(event.type) {
                //Event of UART receving data
                /*We'd better handler data event fast, there would be much more data events than
                other types of events. If we take too much time on data event, the queue might
                be full.*/
                case UART_DATA:
                    //ESP_LOGI(TAG, "[UART %d DATA]: %d", port, event.size);
					if (is_comm_state_open(handle)) {
                    	javanotify_serial_event(JAVACALL_EVENT_SERIAL_RECEIVE, handle, JAVACALL_OK);
					}
                    break;
                //Event of HW FIFO overflow detected
                case UART_FIFO_OVF:
                    ESP_LOGI(TAG, "hw fifo overflow");
                    // If fifo overflow happened, you should consider adding flow control for your application.
                    // The ISR has already reset the rx FIFO,
                    // As an example, we directly flush the rx buffer here in order to read more data.
                    uart_flush_input(port);
                    xQueueReset(uart_queue);
                    break;
                //Event of UART ring buffer full
                case UART_BUFFER_FULL:
                    ESP_LOGI(TAG, "ring buffer full");
                    // If buffer full happened, you should consider encreasing your buffer size
                    // As an example, we directly flush the rx buffer here in order to read more data.
                    uart_flush_input(port);
                    xQueueReset(uart_queue);
                    break;
                //Event of UART RX break detected
                case UART_BREAK:
                    ESP_LOGI(TAG, "uart rx break");
                    break;
                //Event of UART parity check error
                case UART_PARITY_ERR:
                    ESP_LOGI(TAG, "uart parity error");
                    break;
                //Event of UART frame error
                case UART_FRAME_ERR:
                    ESP_LOGI(TAG, "uart frame error");
                    break;
                //Others
                default:
                    ESP_LOGI(TAG, "uart event type: %d", event.type);
                    break;
            }
        }
    }
    free(dtmp);
    dtmp = NULL;
    vTaskDelete(NULL);
	ESP_LOGI(TAG, "uart task finish");
}

/**
 * Update the baudRate of an open serial port
 *
 * @param hPort the port to configure
 * @param baudRate the new baud rate for the open connection
 * @return <tt>JAVACALL_OK</tt> on success, 
 *         <tt>JAVACALL_FAIL</tt> on error
 */
javacall_result
javacall_serial_set_baudRate(javacall_handle hPort, int baudRate)
{
   if (ESP_OK == uart_set_baudrate(get_uart_port(hPort), baudRate))
   	return JAVACALL_OK;
   else
   	return JAVACALL_FAIL;
}

/**
 * Retrive the current baudRate of the open serial port
 *
 * @param hPort the port to configure
 * @param baudRate pointer to where to return the baudRate
 * @return <tt>JAVACALL_OK</tt> on success, 
 *         <tt>JAVACALL_FAIL</tt> on error
 */
javacall_result /*OPTIONAL*/
javacall_serial_get_baudRate(javacall_handle hPort, int *baudRate)
{
	if (ESP_OK == uart_get_baudrate(get_uart_port(hPort), (uint32_t*)baudRate))
	 return JAVACALL_OK;
	else
	 return JAVACALL_FAIL;

}

/**
 * Configure serial port
 *
 * @param hPort the port to configure
 * @param baudRate the new baud rate for the open connection
 * @param options options for the serial port:
 * bit 0: 0 - 1 stop bit, 1 - 2 stop bits 
 * bit 2-1: 00 - no parity, 01 - odd parity, 10 - even parity 
 * bit 4: 0 - no auto RTS, 1 - set auto RTS 
 * bit 5: 0 - no auto CTS, 1 - set auto CTS 
 * bit 7-6: 01 - 7 bits per symbol, 11 - 8 bits per symbol 
 * @return <tt>JAVACALL_OK</tt> on success, 
 *         <tt>JAVACALL_FAIL</tt> on error
 */
javacall_result /*OPTIONAL*/ javacall_serial_configure(javacall_handle pHandle, int baudRate, int options) {
	uart_port_t port;
	uart_config_t uart_config;

	bzero(&uart_config, sizeof(uart_config_t));
	
	uart_config.baud_rate = baudRate;
    uart_config.data_bits = get_uart_options_databits(options);
    uart_config.parity    = get_uart_options_parity(options);
    uart_config.stop_bits = get_uart_options_stopbits(options);
    uart_config.flow_ctrl = get_uart_options_flowctrl(options);

	port = get_uart_port(pHandle);

	uart_param_config(port, &uart_config);

	return JAVACALL_OK;
}

/**
 * Initiates opening serial link according to the given parameters
 *
 * @param devName the name of the port / device to be opened ("COM1")
 * @param baudRate the baud rate for the open connection. in case
 *        bauseRate=JAVACALL_UNSPECIFIED_BAUD_RATE
 *        the baudRate that was specified by the platform should be used
 * @param options the serial link option (JAVACALL_SERIAL_XXX)
 * @param pHandle the handle of the port to be opend
 * @param pContext filled by ptr to data for reinvocations
 * @retval JAVACALL_OK          success
 * @retval JAVACALL_FAIL        if there was an error   
 * @retval JAVACALL_WOULD_BLOCK  if the caller must call the finish function again to complete the operation
 */
javacall_result  /*OPTIONAL*/
javacall_serial_open_start(const char *devName, int baudRate, unsigned int options ,javacall_handle *hPort, void **pContext)
{
	javacall_handle handle;
	uart_port_t port;
	uart_config_t uart_config;

	bzero(&uart_config, sizeof(uart_config_t));
	
	uart_config.baud_rate = baudRate;
    uart_config.data_bits = get_uart_options_databits(options);
    uart_config.parity    = get_uart_options_parity(options);
    uart_config.stop_bits = get_uart_options_stopbits(options);
    uart_config.flow_ctrl = get_uart_options_flowctrl(options);
    

	if (!strcmp(devName, "COM0")) {
		handle = PLATFORM_PORT_0;
		if (is_comm_state_open(handle)) {
			//
			//If Logging port has been open, it's returned the remembered handle for reuse
			//
			*hPort = handle;
			javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_SERIAL, "Serial port reopen ok: %d\n", *hPort);
			return JAVACALL_OK;
		}
		
	} else if (!strcmp(devName, "COM1") && !is_comm_state_open(PLATFORM_PORT_1)) {
		handle = PLATFORM_PORT_1;
	} else if (!strcmp(devName, "COM2") && !is_comm_state_open(PLATFORM_PORT_2)) {
		handle = PLATFORM_PORT_2;
	} else {
		javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_SERIAL, "Serial port open failed: %s\n", devName);
		return JAVACALL_FAIL;
	}

	port = get_uart_port(handle);

	QueueHandle_t* uart_queue = get_uart_queue(handle);
	
    uart_param_config(port, &uart_config);
    uart_set_pin(port, get_uart_pin(UART_PIN_TYPE_TX, handle), get_uart_pin(UART_PIN_TYPE_RX, handle),
		         get_uart_pin(UART_PIN_TYPE_RTS, handle), get_uart_pin(UART_PIN_TYPE_CTS, handle));

	if (ESP_OK == uart_driver_install(port, BUF_SIZE * 2, BUF_SIZE * 2, 20, uart_queue, 0)) {
		//Because we never delete the UART driver, uart_driver_install() may return with ESP_FAIL
		//when the comm port to open after previous close. But we can't tell if the ESP_FAIL
		//is caused by reopen or really fail, so we have to ignore the case that real failed 
		//(e.g. not enough memory). The uart_event_task would not be started repeatly, only
		//be started once, one task per port.
		if (pdPASS != xTaskCreate(uart_event_task, "uart_event_task", 2048, (void*)handle, 12, NULL)) {
			//If task create fail, delete the driver and return JAVACALL_FAIL
			uart_driver_delete(port);
			javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_SERIAL, "Serial port %d open failed for task failure\n", handle);
			return JAVACALL_FAIL;
		}
	}
	*hPort = handle;
	set_comm_state(handle, OPEN);

	javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_SERIAL, "Serial port %d open successfully\n", *hPort);
	return JAVACALL_OK;
}

/**
 * Finishes opening serial link according to the given parameters
 *
 * @param devName the name of the port / device to be opened ("COM1")
 * @param baudRate the baud rate for the open connection. in case
 *        bauseRate=JAVACALL_UNSPECIFIED_BAUD_RATE
 *        the baudRate that was specified by the platform should be used
 * @param options the serial link option (JAVACALL_SERIAL_XXX)
 * @param pHandle the handle of the port to be opend
 * @param context ptr to the data of reinvokation context
 * @retval JAVACALL_OK on success, 
 * @retval JAVACALL_FAIL on error
 * @retval JAVACALL_WOULD_BLOCK  if the caller must call the finish function again to complete the operation
 */
javacall_result  /*OPTIONAL*/ javacall_serial_open_finish(const char *devName, int baudRate, unsigned int options, 
	  /*OUT*/javacall_handle *pHandle, void *context)

{
    return JAVACALL_FAIL;
}

/**
 * Initiates closing serial link 
 *
 * @param hPort the port to close
 * @param pContext filled by ptr to data for reinvocations
 * after this call, java is guaranteed not to call javacall_serial_read() or 
 * javacall_serial_write() before issuing another javacall_serial_open( ) call.
 *
 * @return <tt>JAVACALL_OK</tt> on success, 
 *         <tt>JAVACALL_FAIL</tt>
 * @retval JAVACALL_WOULD_BLOCK  if the caller must call the finish function again to complete the operation
 */
javacall_result /*OPTIONAL*/
javacall_serial_close_start(javacall_handle hPort, void **pContext)
{
	if (hPort != PLATFORM_PORT_0) {
		//PLATFORM_PORT_0 is special port for Josh Console connection, so never close nor change port setting
		set_comm_state(hPort, CLOSE);
	}
	javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_SERIAL, "Serial port %d closed successfully\n", hPort);
	return JAVACALL_OK;

}

/**
 * Finishes closing serial link.
 *
 * @param hPort the port to close
 * @param context ptr to data saved before sleeping
 * @retval <tt>JAVACALL_OK</tt> on success, 
 * @retval <tt>JAVACALL_FAIL</tt>
 * @retval JAVACALL_WOULD_BLOCK  if the caller must call the finish function again to complete the operation
 */
javacall_result /*OPTIONAL*/
javacall_serial_close_finish(javacall_handle hPort, void *context)
{
    return JAVACALL_OK;
}

static javacall_result comm_read_common(uart_port_t hPort, unsigned char* buffer, int size ,int *bytesRead) {
	int len = uart_read_bytes(hPort, buffer, size, 0);
	if (len == 0) {
		*bytesRead = 0;
		return JAVACALL_WOULD_BLOCK;
	} else if (len > 0) {
		*bytesRead = len;
		return JAVACALL_OK;
	} else {
		return JAVACALL_FAIL;
	}
}

/**
 * Initiates reading a specified number of bytes from serial link, 
 
 * @param hPort the port to read the data from
 * @param buffer to which data is read
 * @param size number of bytes to be read. Actual number of bytes
 *              read may be less, if less data is available
 * @param byteRead actual number the were read from the port.
 * @param pContext filled by ptr to data for reinvocations
 * @retval JAVACALL_OK          success
 * @retval JAVACALL_FAIL        if there was an error   
 * @retval JAVACALL_WOULD_BLOCK  if the caller must call the finish function again to complete the operation
 */
javacall_result /*OPTIONAL*/
javacall_serial_read_start(javacall_handle hPort, unsigned char* buffer, int size ,int *bytesRead, void **pContext)
{
	return comm_read_common(get_uart_port(hPort), buffer, size, bytesRead);
}

/**
 * Finishes reading a specified number of bytes from serial link, 
 *
 * @param hPort the port to read the data from
 * @param buffer to which data is read
 * @param size number of bytes to be read. Actual number of bytes
 *              read may be less, if less data is available
 * @param byteRead actual number the were read from the port.
 * @param context ptr to data saved before sleeping
 * @retval JAVACALL_OK          success
 * @retval JAVACALL_FAIL        if there was an error   
 * @retval JAVACALL_WOULD_BLOCK  if the caller must call the finish function again to complete the operation
 */
javacall_result /*OPTIONAL*/
javacall_serial_read_finish(javacall_handle hPort, unsigned char* buffer, int size, int *bytesRead, void *context)
{
	return comm_read_common(get_uart_port(hPort), buffer, size, bytesRead);
}

/**
 * Initiates writing a specified number of bytes to serial link, 
 *
 * @param hPort the port to write the data to
 * @param buffer buffer to write data from
 * @param size number of bytes to be write.
 * @param bytesWritten the number of bytes actually written.
 * @param pContext filled by ptr to data for reinvocations
 * @retval JAVACALL_OK          success
 * @retval JAVACALL_FAIL        if there was an error   
 * @retval JAVACALL_WOULD_BLOCK  if the caller must call the finish function again to complete the operation
 */
javacall_result /*OPTIONAL*/
javacall_serial_write_start(javacall_handle hPort, unsigned char* buffer, int size, int *bytesWritten, void **pContext)
{ 
	uart_write_bytes(get_uart_port(hPort), (const char *) buffer, size);
	*bytesWritten = size;
	return JAVACALL_OK;
}

/**
 * Finishes writing a specified number of bytes to serial link, 
 *
 * @param hPort the port to write the data to
 * @param buffer buffer to write data from
 * @param size number of bytes to be write.
 * @param bytesWritten the number of bytes actually written.
 * @param context ptr to data saved before sleeping
 * @retval JAVACALL_OK          success
 * @retval JAVACALL_FAIL        if there was an error   
 */
javacall_result /*OPTIONAL*/
javacall_serial_write_finish(javacall_handle hPort, unsigned char* buffer, int size, int *bytesWritten, void *context)
{
    return JAVACALL_FAIL;
}
