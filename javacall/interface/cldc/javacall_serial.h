/*
 *
 * Copyright  1990-2007 Sun Microsystems, Inc. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version
 * 2 only, as published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License version 2 for more details (a copy is
 * included at /legal/license.txt).
 * 
 * You should have received a copy of the GNU General Public License
 * version 2 along with this work; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 * 
 * Please contact Sun Microsystems, Inc., 4150 Network Circle, Santa
 * Clara, CA 95054 or visit www.sun.com if you need additional
 * information or have any questions.
 */ 

#ifndef __JAVACALL_SERIAL_H_
#define __JAVACALL_SERIAL_H_

/**
 * @file javacall_serial.h
 * @ingroup Serial
 * @brief Javacall interfaces for serial port
 */

#include "javacall_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup Serial Serial Port API
 * @ingroup JTWI
 *
 * NOTE: The following functions are optional.
 *
 * Optional API to support serial COM communication.
 *
 * @{
 */
       
/******************************************************************************
 ******************************************************************************
 ******************************************************************************
    OPTIONAL FUNCTIONS
 ******************************************************************************
 ******************************************************************************
 ******************************************************************************/
    
/**
 * @defgroup OptionalSerial Optional Serial
 * @ingroup Serial
 * @{
 */



/* COMM options */
#define JAVACALL_SERIAL_STOP_BITS_2     0x01
#define JAVACALL_SERIAL_ODD_PARITY      0x02
#define JAVACALL_SERIAL_EVEN_PARITY     0x04
#define JAVACALL_SERIAL_AUTO_RTS        0x10
#define JAVACALL_SERIAL_AUTO_CTS        0x20
#define JAVACALL_SERIAL_BITS_PER_CHAR_7 0x80
#define JAVACALL_SERIAL_BITS_PER_CHAR_8 0xC0

/*Unspecified baud rate*/
#define JAVACALL_UNSPECIFIED_BAUD_RATE  -1
/**
 * Return an string the contains a list of available ports delimited by a comma
 * (COM1,COM2)
 * If there is no available port then buffer will be empty string and return JAVACALL OK.
 *
 * @param buffer lists of available ports.This value must be null terminated.
 * @param maxBufferLen the maximum length of buffer
 * @retval JAVACALL_OK success
 * @retval JAVACALL_FAIL fail or the return length is more than maxBufferLen characters.
 */
javacall_result /*OPTIONAL*/  javacall_serial_list_available_ports(char* buffer, int maxBufLen);


/**
 * Initiates opening serial link according to the given parameters.
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
javacall_result  /*OPTIONAL*/ javacall_serial_open_start(const char *devName, int baudRate, unsigned int options, 
  /*OUT*/javacall_handle *pHandle, void **pContext);

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
 *
 * @retval JAVACALL_OK on success, 
 * @retval JAVACALL_FAIL on error
 * @retval JAVACALL_WOULD_BLOCK  if the caller must call the finish function again to complete the operation
 */
javacall_result /*OPTIONAL*/ javacall_serial_open_finish(const char *devName, int baudRate, unsigned int options, 
  /*OUT*/javacall_handle *pHandle, void *context);

/**
 * Update the baudRate of an open serial port
 *
 * @param hPort the port to configure
 * @param baudRate the new baud rate for the open connection
 * @return <tt>JAVACALL_OK</tt> on success, 
 *         <tt>JAVACALL_FAIL</tt> on error
 */
javacall_result /*OPTIONAL*/ javacall_serial_set_baudRate(javacall_handle pHandle, int baudRate);

/**
 * Retrive the current baudRate of the open serial port
 *
 * @param hPort the port to configure
 * @param baudRate pointer to where to return the baudRate
 * @return <tt>JAVACALL_OK</tt> on success, 
 *         <tt>JAVACALL_FAIL</tt> on error
 */
javacall_result /*OPTIONAL*/ javacall_serial_get_baudRate(javacall_handle hPort, /*OUT*/ int *baudRate);

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
javacall_result /*OPTIONAL*/ javacall_serial_configure(javacall_handle pHandle, int baudRate, int options);

/**
 * Initiates closing serial link.
 *
 * @param hPort the port to close
 * @param pContext filled by ptr to data for reinvocations
 * after this call, java is guaranteed not to call javacall_serial_read() or 
 * javacall_serial_write() before issuing another javacall_serial_open( ) call.
 *
 * @retval <tt>JAVACALL_OK</tt> on success, 
 * @retval <tt>JAVACALL_FAIL</tt>
 * @retval JAVACALL_WOULD_BLOCK  if the caller must call the finish function again to complete the operation
 */
javacall_result /*OPTIONAL*/ javacall_serial_close_start(javacall_handle hPort, void **pContext);

/**
 * Finishes closing serial link.
 *
 * @param hPort the port to close
 * @param context ptr to data saved before sleeping
 * @retval <tt>JAVACALL_OK</tt> on success, 
 * @retval <tt>JAVACALL_FAIL</tt>
 * @retval JAVACALL_WOULD_BLOCK  if the caller must call the finish function again to complete the operation
 */
javacall_result /*OPTIONAL*/ javacall_serial_close_finish(javacall_handle hPort, void *context);

/**
 * Initiates reading a specified number of bytes from serial link, 
 
 * @param hPort the port to read the data from
 * @param buffer to which data is read
 * @param size number of bytes to be read. Actual number of bytes
 *              read may be less, if less data is available
 * @param bytesRead actual number the were read from the port.
 * @param pContext filled by ptr to data for reinvocations
 * @retval JAVACALL_OK          success
 * @retval JAVACALL_FAIL        if there was an error   
 * @retval JAVACALL_WOULD_BLOCK  if the caller must call the finish function again to complete the operation
 */
javacall_result /*OPTIONAL*/ javacall_serial_read_start(javacall_handle hPort, unsigned char* buffer, 
  int size ,/*OUT*/int *bytesRead, void **pContext);

/**
 * Finishes reading a specified number of bytes from serial link, 
 *
 * @param hPort the port to read the data from
 * @param buffer to which data is read
 * @param size number of bytes to be read. Actual number of bytes
 *              read may be less, if less data is available
 * @param bytesRead actual number the were read from the port.
 * @param context ptr to data saved before sleeping
 * @retval JAVACALL_OK          success
 * @retval JAVACALL_FAIL        if there was an error   
 * @retval JAVACALL_WOULD_BLOCK  if the caller must call the finish function again to complete the operation
 */
javacall_result /*OPTIONAL*/ javacall_serial_read_finish(javacall_handle hPort, unsigned char* buffer, 
  int size, int *bytesRead, void *context);

/**
 * Initiates writing a specified number of bytes to serial link, 
 *
 * @param hPort the port to write the data to
 * @param buffer buffer to write data from
 * @param size number of bytes to be written.
 * @param bytesWritten the number of bytes actually written.
 * @param pContext filled by ptr to data for reinvocations
 * @retval JAVACALL_OK          success
 * @retval JAVACALL_FAIL        if there was an error   
 * @retval JAVACALL_WOULD_BLOCK  if the caller must call the finish function again to complete the operation
 */
javacall_result /*OPTIONAL*/ javacall_serial_write_start(javacall_handle hPort, unsigned char* buffer, 
  int size, int *bytesWritten, void **pContext);  

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
 * @retval JAVACALL_WOULD_BLOCK  if the caller must call the finish function again to complete the operation
 */
javacall_result /*OPTIONAL*/ javacall_serial_write_finish(javacall_handle hPort, unsigned char* buffer, int size, int *bytesWritten, void *context);  


/** @} */


/******************************************************************************
 ******************************************************************************
 ******************************************************************************

  NOTIFICATION FUNCTIONS
  - - - -  - - - - - - -  
  The following functions are implemented by Sun.
  Platform is required to invoke these function for each occurence of the
  undelying event.
  The functions need to be executed in platform's task/thread

 ******************************************************************************
 ******************************************************************************
 ******************************************************************************/
    
/**
 * @defgroup Notification functions 
 * @ingroup Port
 * @{
 */
/**
 * @enum javacall_port_callback_type
 */
typedef enum {    
    JAVACALL_EVENT_SERIAL_RECEIVE = 1000,
    JAVACALL_EVENT_SERIAL_WRITE   = 1001,
    JAVACALL_EVENT_SERIAL_OPEN    = 1002,
    JAVACALL_EVENT_SERIAL_CLOSE   = 1003
} javacall_serial_callback_type;


/**
 * A callback function to be called for notification of comm port 
 * related events.
 * The platform will invoke the call back in platform context.
 *
 * @param type type of indication: 
 *          JAVACALL_EVENT_SERIAL_RECEIVE
 * @param hPort handle of port related to the notification
 * @param operation_result <tt>JAVACALL_OK</tt> if operation 
 *        completed successfully, 
 *        <tt>JAVACALL_FAIL</tt> or negative value on failure
 */
void javanotify_serial_event(
                             javacall_serial_callback_type type, 
                             javacall_handle hPort,
                             javacall_result operation_result);

							 
/**********************************************************
 * UART functions
 *
 **********************************************************/

/**
 * @enum javacall_uart_parity
 * @brief UART parities 
 */
typedef enum {
    /** No parity bit. */
    JC_UART_PARITY_NONE = 0,
    /** ODD parity scheme. */
    JC_UART_PARITY_ODD = 1,
    /** EVEN parity scheme. */
    JC_UART_PARITY_EVEN = 2,
    /** MARK parity scheme. */
    JC_UART_PARITY_MARK = 3,
    /** SPACE parity scheme. */
    JC_UART_PARITY_SPACE = 4
} javacall_uart_parity;

/**
 * @enum javacall_uart_bits_per_char
 * @brief UART bits per char 
 */
typedef enum {
    /** 5-bit chars */
    JC_UART_BITS_PER_CHAR_5 = 5,
    /** 6-bit chars */
    JC_UART_BITS_PER_CHAR_6 = 6,
    /** 7-bit chars */
    JC_UART_BITS_PER_CHAR_7 = 7,
    /** 8-bit chars */
    JC_UART_BITS_PER_CHAR_8 = 8,
    /** 9-bit chars */
    JC_UART_BITS_PER_CHAR_9 = 9
} javacall_uart_bits_per_char;

/**
 * @enum javacall_uart_stop_bits
 * @brief UART bits per char 
 */
typedef enum {
    /* Number of STOP bits - 1. */
    JC_STOPBITS_1 = 1,
    /* Number of STOP bits - 1-1/2. */
    JC_STOPBITS_1_5 = 2,
    /* Number of STOP bits - 2. */
    JC_STOPBITS_2 = 3,
} javacall_uart_stop_bits;
    
/**
 * @enum javacall_uart_event_type
 * @brief UART event types 
 */
typedef enum {
    /** Input data available */
    INPUT_DATA_AVAILABLE = 0,
    /** Input buffer overrun */
    INPUT_BUFFER_OVERRUN = 1,
    /** Output buffer empty */
    OUTPUT_BUFFER_EMPTY = 2,
    /** Event ID indicating a break interrupt */
    BREAK_INTERRUPT = 4,
    /** Event ID indicating a parity error */
    PARITY_ERROR = 8,   
    /** Event ID indicating a parity error*/
    FRAMING_ERROR = 16
} javacall_uart_event_type;

typedef enum {
    JAVACALL_UART_OK = 0,
    JAVACALL_UART_FAIL,
    JAVACALL_UART_NOT_FOUND,
    JAVACALL_UART_BUSY,
    JAVACALL_UART_CLOSED,
    JAVACALL_UART_INVALID_CONFIG,
    JAVACALL_UART_UNSUPPORTED_ACCESS_MODE,
    JAVACALL_UART_WOULD_BLOCK,
    JAVACALL_UART_INVALID_STATE,
    JAVACALL_UART_UNSUPPORTED_OPERATION,
    JAVACALL_UART_OUT_OF_MEMORY
} javacall_uart_result;

/**
 * RTS/CTS (hardware) flow control on input.
 */
#define FLOWCONTROL_RTSCTS_IN 1
/**
  * RTS/CTS (hardware) flow control on output.
  */
#define FLOWCONTROL_RTSCTS_OUT 2
/**
 * XON/XOFF (software) flow control on input.
 */
#define FLOWCONTROL_XONXOFF_IN 4
/**
  * XON/XOFF (software) flow control on output.
  */
#define FLOWCONTROL_XONXOFF_OUT 8

/**
 * A callback function to be called for notification of uart events.
 * 
 * @param hPort     uart port handle
 * 
 * @param type      type of event: Either
 *                  -INPUT_DATA_AVAILABLE,    
 *                  -INPUT_BUFFER_OVERRUN,
 *                  -OUTPUT_BUFFER_EMPTY 
 */
void javanotify_uart_event(javacall_uart_event_type type, javacall_handle hPort,                           
                           javacall_int32 bytesProcessed, javacall_result result);
                             
/** @} */
/** @} */

/**
 * Starts opening uart according to the given parameters.
 *
 * @param devName     the name of the port / device to be opened ("COM1")
 * @param baudRate    the baud rate for the open connection
 * @param stopBits    the number of stop bits
 * @param flowControl the flow control
 * @param bitsPerchar the number of bits per character
 * @param parity      the parity
 * @param exclusive   the exclusive mode flag
 * @param pHandle     the handle of the port to be opened
 *
 * @retval JAVACALL_UART_OK               success
 * @retval JAVACALL_UART_FAIL             fail
 * @retval JAVACALL_UART_BUSY             if the device is already open with exclusive mode
 * @retval JAVACALL_UART_WOULD_BLOCK      if the caller must call the finish function again to complete the operation
 * @retval JAVACALL_UART_INVALID_CONFIG   if incoming parameters are invalid
 * @retval JAVACALL_UART_UNSUPPORTED_ACCESS_MODE  if EXCLUSIVE or
 *        SHARED mode is not supported
 */
javacall_uart_result 
javacall_uart_open_start(const char *devName, unsigned int baudRate,
                         javacall_uart_stop_bits stopBits, unsigned int flowControl,
                         javacall_uart_bits_per_char bitsPerchar, javacall_uart_parity parity,
                         const javacall_bool exclusive, /*OUT*/javacall_handle *pHandle);

/**
 * Finishes opening uart according to the given parameters.
 *
 * @param devName     the name of the port / device to be opened ("COM1")
 * @param baudRate    the baud rate for the open connection
 * @param stopBits    the number of stop bits
 * @param flowControl the flow control
 * @param bitsPerchar the number of bits per character
 * @param parity      the parity
 * @param exclusive   the exclusive mode flag
 * @param pHandle     the handle of the port to be opened
 *
 * @retval JAVACALL_UART_OK               success
 * @retval JAVACALL_UART_FAIL             fail
 * @retval JAVACALL_UART_WOULD_BLOCK      if the caller must call the finish function again to complete the operation
 * @retval JAVACALL_UART_INVALID_CONFIG if incoming parameters are invalid 
 */
javacall_uart_result 
javacall_uart_open_finish(const char *devName, unsigned int baudRate,
                         javacall_uart_stop_bits stopBits, unsigned int flowControl,
                         javacall_uart_bits_per_char bitsPerchar, javacall_uart_parity parity,
                         /*OUT*/javacall_handle *pHandle);

/**
 * Initiates closing serial link.
 *
 * @param hPort the port to close
 * @param pContext filled by ptr to data for reinvocations
 * after this call, java is guaranteed not to call javacall_uart_read_xx() or 
 * javacall_uart_write_xx() before issuing another javacall_uart_open_xx( ) call.
 *
 * @retval JAVACALL_UART_OK on success, 
 * @retval JAVACALL_UART_FAIL
 * @retval JAVACALL_UART_WOULD_BLOCK  if the caller must call the finish function again to complete the operation
 */
javacall_uart_result /*OPTIONAL*/ 
javacall_uart_close_start(javacall_handle hPort, void **pContext);

/**
 * Finishes closing serial link.
 *
 * @param hPort the port to close
 * @param context ptr to data saved before sleeping
 * @retval JAVACALL_UART_OK on success, 
 * @retval JAVACALL_UART_FAIL
 * @retval JAVACALL_UART_WOULD_BLOCK  if the caller must call the finish function again to complete the operation
 */
javacall_uart_result /*OPTIONAL*/ 
javacall_uart_close_finish(javacall_handle hPort, void *context);

/**
 * Initiates reading a specified number of bytes from serial link, 
 
 * @param hPort the port to read the data from
 * @param buffer to which data is read
 * @param size number of bytes to be read. Actual number of bytes
 *              read may be less, if less data is available
 * @param bytesRead actual number the were read from the port.
 * @param pContext filled by ptr to data for reinvocations
 * @retval JAVACALL_UART_OK          success
 * @retval JAVACALL_UART_FAIL        if there was an error   
 * @retval JAVACALL_UART_WOULD_BLOCK  if the caller must call the finish function again to complete the operation
 */
javacall_uart_result /*OPTIONAL*/ 
javacall_uart_read_start(javacall_handle hPort, unsigned char* buffer, int size ,/*OUT*/int *bytesRead, void **pContext);

/**
 * Finishes reading a specified number of bytes from serial link, 
 *
 * @param hPort the port to read the data from
 * @param buffer to which data is read
 * @param size number of bytes to be read. Actual number of bytes
 *              read may be less, if less data is available
 * @param bytesRead actual number the were read from the port.
 * @param size number of bytes to be write.
 * @param context ptr to data saved before sleeping
 * @retval JAVACALL_UART_OK          success
 * @retval JAVACALL_UART_FAIL        if there was an error   
 * @retval JAVACALL_UART_WOULD_BLOCK  if the caller must call the finish function again to complete the operation
 */
javacall_uart_result /*OPTIONAL*/ 
javacall_uart_read_finish(javacall_handle hPort, unsigned char* buffer, int size, int *bytesRead, void *context);

/**
 * Initiates writing a specified number of bytes to serial link, 
 *
 * @param hPort the port to write the data to
 * @param buffer buffer to write data from
 * @param size number of bytes to be written.
 * @param bytesWritten the number of bytes actually written.
 * @param pContext filled by ptr to data for reinvocations
 * @retval JAVACALL_OK          success
 * @retval JAVACALL_FAIL        if there was an error   
 * @retval JAVACALL_WOULD_BLOCK  if the caller must call the finish function again to complete the operation
 */
javacall_uart_result /*OPTIONAL*/ 
javacall_uart_write_start(javacall_handle hPort, unsigned char* buffer, 
  int size, int *bytesWritten, void **pContext);  

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
 * @retval JAVACALL_WOULD_BLOCK  if the caller must call the finish function again to complete the operation
 */
javacall_uart_result /*OPTIONAL*/ 
javacall_uart_write_finish(javacall_handle hPort, unsigned char* buffer, int size, int *bytesWritten, void *context);  

/**
 * Update the stopBits of an open uart port
 *
 * @param hPort the port to configure
 * @param stopBits new stopBits for the open connection
 * @retval JAVACALL_UART_OK    on success,
 * @retval JAVACALL_UART_UNSUPPORTED_OPERATION  if the stopBits 
 *         is not supported
 * @retval JAVACALL_UART_FAIL  on error
 */
javacall_uart_result /*OPTIONAL*/ 
javacall_uart_set_stop_bits(javacall_handle handle, javacall_uart_stop_bits stopBits);

/**
 * Retrive the current stopBits of the open uart port
 *
 * @param hPort the port to configure
 * @param stopBits pointer to where to return the stopBits
 * @retval JAVACALL_UART_OK on success, 
 *         JAVACALL_UART_FAIL on error
 */
javacall_uart_result /*OPTIONAL*/ 
javacall_uart_get_stop_bits(javacall_handle handle, /*OUT*/ javacall_uart_stop_bits *stopbits);

/**
 * Update the parity of an open uart port
 *
 * @param hPort the port to configure
 * @param parity new parity for the open connection
 * @retval JAVACALL_UART_OK on success,
 * @retval JAVACALL_UART_UNSUPPORTED_OPERATION if the stopBits is
 *         not supported
 * @retval JAVACALL_UART_FAIL on error
 */
javacall_uart_result /*OPTIONAL*/ 
javacall_uart_set_parity(javacall_handle handle, javacall_uart_parity parity);

/**
 * Retrive the current parity of the open uart port
 *
 * @param hPort the port to configure
 * @param parity pointer to where to return the parity
 * @retval JAVACALL_UART_OK on success, 
 *         JAVACALL_UART_FAIL on error
 */
javacall_uart_result /*OPTIONAL*/
javacall_uart_get_parity(javacall_handle handle, /*OUT*/ javacall_uart_parity *parity);

/**
 * Update the bitsPerChar of an open uart port
 *
 * @param hPort the port to configure
 * @param bitsPerChar new bits per char for the open connection
 * @retval JAVACALL_UART_OK on success,
 * @retval JAVACALL_UART_UNSUPPORTED_OPERATION if the bitsPerChar
 *         is not supported
 * @retval JAVACALL_UART_FAIL on error
 */
javacall_uart_result /*OPTIONAL*/ 
javacall_uart_set_bits_per_char(javacall_handle handle, javacall_uart_bits_per_char bitsPerChar);

/**
 * Retrive the bits per char of the open uart port
 *
 * @param hPort the port to configure
 * @param parity pointer to where to return the parity
 * @retval JAVACALL_UART_OK on success, 
 *         JAVACALL_UART_FAIL on error
 */
javacall_uart_result /*OPTIONAL*/
javacall_uart_get_bits_per_char(javacall_handle handle, /*OUT*/ javacall_uart_bits_per_char *bitsPerChar);

/**
 * Update the baudRate of an open UART port
 *
 * @param hPort the port to configure
 * @param baudRate the new baud rate for the open connection
 * @return <tt>JAVACALL_OK</tt> on success, 
 *         <tt>JAVACALL_FAIL</tt> on error
 */
javacall_result /*OPTIONAL*/ javacall_uart_set_baudrate(javacall_handle pHandle, int baudRate);

/**
 * Retrive the current baudRate of the open UART port
 *
 * @param hPort the port to configure
 * @param baudRate pointer to where to return the baudRate
 * @return <tt>JAVACALL_OK</tt> on success, 
 *         <tt>JAVACALL_FAIL</tt> on error
 */
javacall_result /*OPTIONAL*/ javacall_uart_get_baudrate(javacall_handle hPort, /*OUT*/ int *baudRate);

/**
 * Stops write operations if any pending
 *
 * @param hPort the port to configure
 * @retval JAVACALL_UART_OK on success, 
 *         JAVACALL_UART_FAIL on error
 */
javacall_uart_result /*OPTIONAL*/
javacall_uart_stop_writing(javacall_handle handle);


/**
 * Stops read operations if any pending
 *
 * @param hPort the port to configure
 * @retval JAVACALL_UART_OK on success, 
 *         JAVACALL_UART_FAIL on error
 */
javacall_uart_result /*OPTIONAL*/
javacall_uart_stop_reading(javacall_handle handle);

/**
 * Starts listening for uart events.
 *
 * @param handle serial port handle
 *
 * @param type   type of event: Either
 *                  -INPUT_DATA_AVAILABLE,
 *                  -INPUT_BUFFER_OVERRUN,
 *                  -OUTPUT_BUFFER_EMPTY
 *                  -BREAK_INTERRUPT
 *                  -FRAMING_ERROR
 *                  -PARITY_ERROR
 *
 * @retval JAVACALL_UART_OK if no error
 * @retval JAVACALL_UART_UNSUPPORTED_OPERATION if event is not 
 *         supported
 */
javacall_uart_result /*OPTIONAL*/
javacall_uart_start_event_listening(javacall_handle handle, javacall_uart_event_type eventId);

/**
 * Stops listening for uart events.
 *
 * @param handle serial port handle
 *
 * @param type   type of event: Either
 *                  -INPUT_DATA_AVAILABLE,
 *                  -INPUT_BUFFER_OVERRUN,
 *                  -OUTPUT_BUFFER_EMPTY
 *                  -BREAK_INTERRUPT
 *                  -FRAMING_ERROR
 *                  -PARITY_ERROR
 *
 * @retval JAVACALL_UART_OK if no error
 */
javacall_uart_result /*OPTIONAL*/
javacall_uart_stop_event_listening(javacall_handle handle, javacall_uart_event_type eventId);

/**
 * Returns power control group of this channel. It is used for 
 * power management notification. 
 * 
 * @param handle open device handle
 * @param grp    power managment group
 * 
 * 
 * @retval JAVACALL_UART_FAIL if the device was closed,
 *         JAVACALL_UART_OK otherwise
 */
javacall_uart_result /*OPTIONAL*/
javacall_uart_get_group_id(const javacall_handle handle, javacall_int32* const  grp);

/**
 * Attempts to lock for exclusive access the underlying 
 * peripheral device resource. 
 * <p> 
 * Checks for status and returns immediately if the resource is 
 * already locked. 
 * 
 * @param handle of open resource 
 * @param owner a pointer to current owner handle  
 * 
 * @return JAVACALL_UART_OK if exclusive access was granted, 
 *         JAVACALL_UART_FAIL if the resource is locked by other
 *         application
 */
javacall_uart_result 
javacall_uart_lock(const javacall_handle handle, javacall_handle* const owner);

/**
 * Releases from exclusive access the underlying peripheral 
 * device resource. 
 * <p> 
 * Returns silently if the resource was not 
 * locked to <code>handle</code>
 * 
 * @param handle of open resource 
 * @param owner a pointer to current owner handle  
 * 
 * @return JAVACALL_OK if no errors, 
 *         JAVACALL_FAIL if the resource is locked by other
 *         application
 */
javacall_uart_result 
javacall_uart_unlock(const javacall_handle handle);

#ifdef __cplusplus
}
#endif

#endif 


