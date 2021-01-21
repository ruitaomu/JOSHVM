#include <stdio.h>
#include <string.h>
#include "javacall_serial.h"

#define BUFF_LENGTH 1024
#define RXBUFF 0
#define TXBUFF 1

#define RING_BUFF_INVALID_HANDLE ((typeRing_buff_handle)-1)

typedef struct _typeRing_buff {
	unsigned int posRead;
	unsigned int posWrite;
	int isFull;
	unsigned char buff[BUFF_LENGTH];
} typeRing_buff, *typeRing_buff_handle;

static UART_HandleTypeDef lpuart1, usart2;
static typeRing_buff ring_uart1, ring_uart2, ring_tx_uart1, ring_tx_uart2;
static unsigned char recv_byte[2];
static int is_initialized_port[2] = {0};

static typeRing_buff_handle get_uart_ring_buff(UART_HandleTypeDef* UartHandle, int bufftype) {
	if (UartHandle == &lpuart1) {
		return bufftype==RXBUFF?&ring_uart1:&ring_tx_uart1;
	} else if (UartHandle == &usart2) {
		return bufftype==RXBUFF?&ring_uart2:&ring_tx_uart2;
	} else {
		return RING_BUFF_INVALID_HANDLE;
	}
}

static void ring_buff_init(typeRing_buff_handle buff) {
	if (buff != RING_BUFF_INVALID_HANDLE) {
		memset(buff, 0, sizeof(typeRing_buff));
	}
}

static int ring_buff_is_empty(typeRing_buff_handle buff) {
	if (buff != RING_BUFF_INVALID_HANDLE) {
		return (buff->posRead == buff->posWrite) ? 1:0;
	} else {
		return 1;
	}
}

static int ring_buff_writebyte(typeRing_buff_handle buff, unsigned char b) {
	if (buff->isFull) {
		return 0;
	}

	if (buff->posWrite >= BUFF_LENGTH) {
		buff->posWrite = 0;
	}

	int posWrite = buff->posWrite;
	int posRead = buff->posRead;

	buff->buff[posWrite++] = b;

	if (posWrite == posRead) {
		buff->isFull = 1;
	}

	if (!buff->isFull) {
		buff->posWrite = posWrite;
	}
	return 1;
}

static int ring_buff_readbyte(typeRing_buff_handle buff, unsigned char* b) {
	if (buff == RING_BUFF_INVALID_HANDLE) {
		return -1;
	}

	if (ring_buff_is_empty(buff)) {
		return 0;
	}

	if (buff->posRead >= BUFF_LENGTH) {
		buff->posRead = 0;
	}

	*b = buff->buff[buff->posRead++];

	return 1;
}

static uint32_t get_uart_options_databits(unsigned int options) {
	unsigned int opt_bps = options & (JAVACALL_SERIAL_BITS_PER_CHAR_7 | JAVACALL_SERIAL_BITS_PER_CHAR_8);
	if (opt_bps == JAVACALL_SERIAL_BITS_PER_CHAR_7) {
		return UART_WORDLENGTH_7B;
	} else {
		return UART_WORDLENGTH_8B;
	}
}

static uint32_t get_uart_options_parity(unsigned int options) {
	unsigned int opt_parity = options & (JAVACALL_SERIAL_ODD_PARITY | JAVACALL_SERIAL_EVEN_PARITY);
	if (opt_parity == JAVACALL_SERIAL_EVEN_PARITY) {
		return UART_PARITY_EVEN;
	} else if (opt_parity == JAVACALL_SERIAL_ODD_PARITY) {
		return UART_PARITY_ODD;
	} else {
		return UART_PARITY_NONE;
	}
}

static uint32_t get_uart_options_stopbits(unsigned int options) {
	unsigned int opt_stopbits = options & JAVACALL_SERIAL_STOP_BITS_2;
	if (opt_stopbits != 0)	{
		return UART_STOPBITS_2;
	} else {
		return UART_STOPBITS_1;
	}
}

static uint32_t get_uart_options_flowctrl(unsigned int options) {
	unsigned int opt_flowcontrol = options & (JAVACALL_SERIAL_AUTO_RTS | JAVACALL_SERIAL_AUTO_CTS);
	if (opt_flowcontrol == JAVACALL_SERIAL_AUTO_RTS) {
		return UART_HWCONTROL_RTS;
	} else if (opt_flowcontrol == JAVACALL_SERIAL_AUTO_CTS) {
		return UART_HWCONTROL_CTS;
	} else if (opt_flowcontrol == 0) {
		return UART_HWCONTROL_NONE;
	} else {
		return  UART_HWCONTROL_RTS_CTS;
	}
}


static UART_HandleTypeDef* MID_LPUART_Init(int port, int baudRate, unsigned int options)
{
	UART_HandleTypeDef* uart;
	USART_TypeDef* instance;

	switch (port) {
		case 0:
			instance = LPUART1;
			uart = &lpuart1;
			__HAL_RCC_LPUART1_CLK_ENABLE();
			break;
		case 1:
			instance = USART2;
			uart = &usart2;
			__HAL_RCC_USART2_CLK_ENABLE();
			break;
		default:
			return NULL;
	}

	memset(uart, 0, sizeof(UART_HandleTypeDef));
	
	uart->Instance = instance;
	uart->Init.BaudRate = baudRate;
	uart->Init.Parity = get_uart_options_parity(options);
	uart->Init.WordLength = get_uart_options_databits(options);
	uart->Init.StopBits = get_uart_options_stopbits(options);
	uart->Init.HwFlowCtl = get_uart_options_flowctrl(options);
	uart->Init.Mode = UART_MODE_TX_RX;
	uart->Init.OverSampling = UART_OVERSAMPLING_16;
	
	if((HAL_UART_Init(uart) != HAL_OK)){
		return NULL;
	}

	ring_buff_init(get_uart_ring_buff(uart, RXBUFF));
	ring_buff_init(get_uart_ring_buff(uart, TXBUFF));
	
	return uart;
}

/**
 * Return an string the contains a list of available ports delimited by a comma
 * (COM1,COM2)
 * If there is no available port then buffer will be empty string and return JAVACALL OK.
 *
 * @param buffer lists of available ports. This value must be null terminated.
 * @param maxBufferLen the maximum length of buffer
 * @retval JAVACALL_OK success
 * @retval JAVACALL_FAIL fail or the return length is more than maxBufferLen characters.
 */
javacall_result
javacall_serial_list_available_ports(char* buffer, int maxBufLen)
{
	if (maxBufLen < 5) {
		return JAVACALL_FAIL;
	}
	
	buffer[0] = 'C';
	buffer[1] = 'O';
	buffer[2] = 'M';
	buffer[3] = '1';
	buffer[4] = '\0';
    return JAVACALL_OK;
}

/**
 * Opens serial link according to the given parameters
 *
 * @param devName the name of the port / device to be opened ("COM1")
 * @param baudRate the baud rate for the open connection
 * @param options the serial link option (JAVACALL_SERIAL_XXX)
 * @return <tt>hPort</tt> on success, 
 *         <tt>-1</tt> on error
 */
static javacall_handle
comm_serial_open(const char *devName, int baudRate, unsigned int options)
{
	int port;
	UART_HandleTypeDef* handle;
	
	if (!strcmp(devName, "COM0")) {
		port = 0;
	} else if (!strcmp(devName, "COM1")) {
		port = 1;
	} else {
		return (javacall_handle)-1;
	}

	if (is_initialized_port[port]) {
		if (port == 0) {
			return &lpuart1;
		} else {
			return (javacall_handle)-1;
		}
	}
	
	handle = MID_LPUART_Init(port, baudRate, options);
	if (handle != NULL) {
		is_initialized_port[port] = 1;
		HAL_UART_Receive_IT(handle, &(recv_byte[port]), 1);      
		return (javacall_handle)handle;
	} else {
		return (javacall_handle)-1;
	}
	return 0;
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
	//Do nothing
	return JAVACALL_OK;
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
	if (!hPort) {
		return JAVACALL_FAIL;
	}

	*baudRate = ((UART_HandleTypeDef*)hPort)->Init.BaudRate;
	return JAVACALL_OK;
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
javacall_result /*OPTIONAL*/ javacall_serial_configure(javacall_handle hPort, int baudRate, int options) {
	//Do nothing in fact
    return javacall_serial_set_baudRate(hPort, baudRate);
}

/**
 * Reads a specified number of bytes from serial link, 
 
 * @param hPort the port to read the data from
 * @param buffer to which data is read
 * @param size number of bytes to be read. Actual number of bytes
 *              read may be less, if less data is available
 * @param byteRead actual number the were read from the port.
 * @retval JAVACALL_OK          success
 * @retval JAVACALL_FAIL        if there was an error   
 * @retval JAVACALL_WOULD_BLOCK  if the caller must call the finish function again to complete the operation
 */
static javacall_result
comm_serial_read(javacall_handle hPort, unsigned char* buffer, int size, int *byteRead)
{
	int size2read = size;
	*byteRead = 0;

	if (size <= 0) {
		return JAVACALL_OK;
	}
	
	typeRing_buff_handle hRingBuf = get_uart_ring_buff(hPort, RXBUFF);

	if (hRingBuf == RING_BUFF_INVALID_HANDLE) {
		return JAVACALL_FAIL;
	}
	
	if (ring_buff_is_empty(hRingBuf)) {
		return JAVACALL_WOULD_BLOCK;
	}

	do {
		if (ring_buff_readbyte(hRingBuf, buffer++) != 1) {
			break;
		}
	} while (--size);


	*byteRead = size2read - size;

	return JAVACALL_OK;
} 

static javacall_result
comm_serial_write_byte(javacall_handle hPort)
{
	UART_HandleTypeDef* hUART = (UART_HandleTypeDef*)hPort;
	typeRing_buff_handle hRingBuf = get_uart_ring_buff(hUART, TXBUFF);
	static unsigned char byteToSend;

	if (hUART->gState != HAL_UART_STATE_READY) {
		if (hUART->gState == HAL_UART_STATE_BUSY_TX) {
			return JAVACALL_WOULD_BLOCK;
		} else {
			return JAVACALL_FAIL;
		}
	}

	if (ring_buff_readbyte(hRingBuf, &byteToSend) == 1) {
		HAL_UART_Transmit_IT(hUART,&byteToSend,1);
	}
	
	return JAVACALL_OK;
} 

/**
 * Writes a specified number of bytes to serial link, 
 * @param hPort the port to write the data to
 * @param buffer buffer to write data from
 * @param size number of bytes to be write.
 * @param bytesWrite the point to the number of bytes actually written
 */
static javacall_result
comm_serial_write(javacall_handle hPort, unsigned char* buffer, int size, int* bytesWritten)
{
	int i;
	for (i = 0; i < size; i++) {
		if (ring_buff_writebyte(get_uart_ring_buff(hPort, TXBUFF), buffer[i]) == 0) break;
	}

	if (i == 0) {
		//TX buffer is full, nothing put into the ring buffer
		return JAVACALL_WOULD_BLOCK;
	}
	
    if (JAVACALL_FAIL != comm_serial_write_byte(hPort)) {
		*bytesWritten = i;
		return JAVACALL_OK;
	} else {
		return JAVACALL_FAIL;
	}
}


/**
 * Closes serial link 

 * @param hPort the port to close
 * after this call, java is guaranteed not to call javacall_serial_read() or 
 * javacall_serial_write() before issuing another javacall_serial_open( ) call.
 *
 * @return <tt>JAVACALL_OK</tt> on success, 
 *         <tt>JAVACALL_FAIL</tt> or negative value on error
 */
static javacall_result
comm_serial_close(javacall_handle hPort)
{
	int port;
	
	if (hPort == &lpuart1) {
		return JAVACALL_OK; //Never really closed
	} else if (hPort == &usart2) {
		port = 1;
	} else {
		return JAVACALL_FAIL; //Bad handle
	}

	HAL_UART_DeInit(hPort);
	is_initialized_port[port] = 0;
	
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
	*pContext = NULL;
    *hPort = comm_serial_open(devName, baudRate, options);
    if ((javacall_handle)-1 == *hPort) {
        return JAVACALL_FAIL;
    } else {
        return JAVACALL_OK;
    }
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
	*pContext= NULL;
    if (comm_serial_close(hPort) == JAVACALL_OK) 
        return JAVACALL_OK;
    else
	return JAVACALL_FAIL;	   		 
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
	*pContext = NULL;
    return comm_serial_read(hPort, buffer, size, bytesRead);
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
    return comm_serial_read(hPort, buffer, size, bytesRead);;
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
	*pContext = NULL;
    return comm_serial_write(hPort, buffer, size, bytesWritten);   
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
    return comm_serial_write(hPort, buffer, size, bytesWritten);     
}

/**************************
 * Interrupt handlers
 * 
 **************************/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle)  
{
	int port;
	
	if (UartHandle == &lpuart1) {
		port = 0;
	} else if (UartHandle == &usart2) {
		port = 1;
	} else {
		return;
	}

	typeRing_buff_handle hRingBuf = get_uart_ring_buff(UartHandle, RXBUFF);
	if (hRingBuf == RING_BUFF_INVALID_HANDLE) {
		return;
	}

	int flag = ring_buff_is_empty(hRingBuf);
	ring_buff_writebyte(hRingBuf, recv_byte[port]);
	HAL_UART_Receive_IT(UartHandle, &(recv_byte[port]), 1);
	if (flag) {
		javanotify_serial_event(JAVACALL_EVENT_SERIAL_RECEIVE, 	UartHandle, JAVACALL_OK);
	}

	return;
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *UartHandle) 
{
	if (JAVACALL_FAIL == comm_serial_write_byte((javacall_handle)UartHandle)) {
		javanotify_serial_event(JAVACALL_EVENT_SERIAL_WRITE, UartHandle, JAVACALL_FAIL);
	} else {
		if (ring_buff_is_empty(get_uart_ring_buff(UartHandle, TXBUFF))) {
			javanotify_serial_event(JAVACALL_EVENT_SERIAL_WRITE, UartHandle, JAVACALL_OK);
		}
	}
}

void javacall_serial_LPUART_IRQHandler() {
	HAL_NVIC_ClearPendingIRQ(LPUART1_IRQn);
	HAL_UART_IRQHandler(&lpuart1);
}

void javacall_serial_USART2_IRQHandler() {
	HAL_UART_IRQHandler(&usart2);
}
