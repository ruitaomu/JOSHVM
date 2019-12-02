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

#include "javacall_serial.h"
#include <rtthread.h>
#include <rtdevice.h>

#define MAX_BUF_LEN 1024

static rt_device_t device_com0 = 0;
static int com0_configured = 0;
static int log_port_opened = 0;
static javacall_handle uart_handle = 0;
static int err_count = 0;

static rt_thread_t serial_tid = RT_NULL;
static struct rt_mailbox jc_serial_mb;
static char jc_serial_mb_pool[128];


#define JC_SERIAL_HANDLE_MIN 2000
#define SERIAL_HANDLE_COUNT 8

static int next_serial_handle = JC_SERIAL_HANDLE_MIN;

static struct {
    int handle;
    int native_handle;
    int flag;
} serial_handle_data[SERIAL_HANDLE_COUNT] = {0};

static int serial_add_handle(int native_handle) {
    for (int i = 0; i < SERIAL_HANDLE_COUNT; i++) {
        if (serial_handle_data[i].native_handle == 0) {
            serial_handle_data[i].native_handle = native_handle;
            serial_handle_data[i].handle = next_serial_handle;
            next_serial_handle++;
            if (next_serial_handle < JC_SERIAL_HANDLE_MIN) {
                next_serial_handle = JC_SERIAL_HANDLE_MIN;
            }
            serial_handle_data[i].flag = 0;
            return serial_handle_data[i].handle;
        }
    }
    return -1;
}

static void serial_remove_handle(int native_handle) {
    for (int i = 0; i < SERIAL_HANDLE_COUNT; i++) {
        if (serial_handle_data[i].native_handle == native_handle) {
            serial_handle_data[i].native_handle = 0;
            serial_handle_data[i].handle = 0;
            serial_handle_data[i].flag = 0;
            return;
        }
    }
}

static int serial_get_handle(int native_handle) {
    for (int i = 0; i < SERIAL_HANDLE_COUNT; i++) {
        if (serial_handle_data[i].native_handle == native_handle) {
            return serial_handle_data[i].handle;
        }
    }
    return 0;
}

static int serial_get_native_handle(int handle) {
    for (int i = 0; i < SERIAL_HANDLE_COUNT; i++) {
        if (serial_handle_data[i].handle == handle) {
            return serial_handle_data[i].native_handle;
        }
    }
    return 0;
}

static int serial_check_and_set_flag(int native_handle) {
    int prev_flag = 0;
    for (int i = 0; i < SERIAL_HANDLE_COUNT; i++) {
        if (serial_handle_data[i].native_handle == native_handle) {
            prev_flag = serial_handle_data[i].flag;
            serial_handle_data[i].flag = 1;
            break;
        }
    }
    return prev_flag;
}

static void serial_unset_flag(int native_handle) {
    for (int i = 0; i < SERIAL_HANDLE_COUNT; i++) {
        if (serial_handle_data[i].native_handle == native_handle) {
            serial_handle_data[i].flag = 0;
            return;
        }
    }
}

static void waitSerialEventEntry(void* parameter) {
    rt_uint32_t value;
    javacall_handle handle;
    while (1) {
        if (rt_mb_recv(&jc_serial_mb, &value, RT_WAITING_FOREVER) == RT_EOK) {
            handle = (javacall_handle)serial_get_handle(value);
            javanotify_serial_event(JAVACALL_EVENT_SERIAL_RECEIVE, handle, JAVACALL_OK);
        }
    }
}

static void sendSerialEvent(rt_device_t dev) {
    if (serial_check_and_set_flag(dev)) {
        // already set
        return;
    }
    if (serial_tid != RT_NULL) {
        rt_mb_send(&jc_serial_mb, (rt_uint32_t)dev);
    }
}

static void init_wait_thread() {
    if (serial_tid != RT_NULL) {
        return;
    }

    rt_mb_init(&jc_serial_mb,
        "serial_mbt",
        &jc_serial_mb_pool[0],
        sizeof(jc_serial_mb_pool)/4,
        RT_IPC_FLAG_FIFO);

    serial_tid = rt_thread_create("serial_notify",
        waitSerialEventEntry,
        RT_NULL,
        1024, 20, 10);
    if (serial_tid != RT_NULL) {
        rt_thread_startup(serial_tid);
    }
}

static javacall_result serial_read_common(javacall_handle hPort, unsigned char* buffer, int size ,int *bytesRead) {
    rt_device_t device = (rt_device_t)serial_get_native_handle(hPort);
    if (device == 0) {
        return JAVACALL_FAIL;
    }

    serial_unset_flag(device);
    int count = rt_device_read(device, 0, buffer, size > MAX_BUF_LEN ? MAX_BUF_LEN : size);
    // if (count > 0) {
    //  javacall_printf("in serial_read. %d\n", count);
    // }
    if (count < 0) {
        return JAVACALL_FAIL;
    } else if (count == 0) {
        init_wait_thread();
        return JAVACALL_WOULD_BLOCK;
    }

    *bytesRead = count;
    return JAVACALL_OK;
}

static javacall_result serial_write_common(javacall_handle hPort, unsigned char* buffer, int size, int *bytesWritten) {
    rt_device_t device = (rt_device_t)serial_get_native_handle(hPort);
    if (device == 0) {
        return JAVACALL_FAIL;
    }

    int count = rt_device_write(device, 0, buffer, size);
    if (count <= 0) {
        return JAVACALL_FAIL;
    }

    *bytesWritten = count;
    return JAVACALL_OK;
}

static javacall_result serial_configure(javacall_handle hPort, int baudRate, int options) {
    if (baudRate < 0) {
        return JAVACALL_OK;
    }

    if (hPort == device_com0) {
        if (com0_configured) {
            return JAVACALL_OK;
        }
        com0_configured = 1;
    }
    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;
    config.baud_rate = baudRate;
    // javacall_printf("in serial_configure. baudRate is %d\n", baudRate);
    // javacall_printf("in serial_configure. buffsz is %d\n", config.bufsz);

    unsigned int opt_bps;
    opt_bps = options & (JAVACALL_SERIAL_BITS_PER_CHAR_7 | JAVACALL_SERIAL_BITS_PER_CHAR_8);
    if (opt_bps == JAVACALL_SERIAL_BITS_PER_CHAR_7) {
        config.data_bits = DATA_BITS_7;
    } else if (opt_bps == JAVACALL_SERIAL_BITS_PER_CHAR_8) {
        config.data_bits = DATA_BITS_8;
    } else {
        return JAVACALL_FAIL;
    }

    config.stop_bits = options & JAVACALL_SERIAL_STOP_BITS_2;
    config.parity = (options & (JAVACALL_SERIAL_ODD_PARITY | JAVACALL_SERIAL_EVEN_PARITY)) >> 1;
    rt_err_t result = rt_device_control(hPort, RT_DEVICE_CTRL_CONFIG, &config);
    if (result != RT_EOK) {
        return JAVACALL_FAIL;
    }
    return JAVACALL_OK;
}

static rt_err_t serial_rx_ind(rt_device_t dev, rt_size_t size) {
    sendSerialEvent(dev);
    return RT_EOK;
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
//  return serial_configure(pHandle, baudRate, options);
    return JAVACALL_FAIL;
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
    char* pCOM0Name = "uart4";
    int isCOM0 = 0;
    char* pName;
    if (!strcmp(devName, "COM0")) {
        isCOM0 = 1;
        pName = pCOM0Name;
    } else if (!strcmp(devName, "COM1")) {
        pName = "uart1";
    } else if (!strcmp(devName, "COM2")) {
        pName = "uart2";
    } else if (!strcmp(devName, "COM3")) {
        pName = "uart3";
    } else if (!strcmp(devName, "COM4")) {
        pName = "uart4";
    } else if (!strcmp(devName, "COM5")) {
        pName = "uart5";
    } else if (!strcmp(devName, "COM6")) {
        pName = "uart6";
    } else if (!strcmp(devName, "COM7")) {
        pName = "uart7";
    } else if (!strcmp(devName, "COM8")) {
        pName = "uart8";
    } else {
        return JAVACALL_FAIL;
    }

    if (!isCOM0 && !strcmp(pName, pCOM0Name)) {
        return JAVACALL_FAIL;
    }

    rt_device_t device = rt_device_find(pName);
    if (device == RT_NULL) {
        return JAVACALL_FAIL;
    }

    int vm_handle = serial_get_handle(device);
    if (vm_handle > 0) {
        // has opened
        if (isCOM0) {
            serial_configure(device, baudRate, options);
            *hPort = vm_handle;
            return JAVACALL_OK;
        } else {
            return JAVACALL_FAIL;
        }
    }

    if (isCOM0) {
        device_com0 = device;
    }

    serial_configure(device, baudRate, options);
    rt_device_set_rx_indicate(device, serial_rx_ind);
    rt_device_open(device, RT_DEVICE_OFLAG_RDWR|RT_DEVICE_FLAG_INT_RX);
    *hPort = serial_add_handle(device);

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
    rt_device_t device = (rt_device_t)serial_get_native_handle(hPort);
    if (device == 0) {
        return JAVACALL_FAIL;
    }
    if (device == device_com0) {
        return JAVACALL_OK;
    }
    rt_device_close(device);
    serial_remove_handle(device);
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
    return serial_read_common(hPort, buffer, size, bytesRead);
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
    return serial_read_common(hPort, buffer, size, bytesRead);
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
    return serial_write_common(hPort, buffer, size, bytesWritten);
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
    return JAVACALL_OK;
}

static void enableLogPort() {
    if (!log_port_opened) {
        javacall_result result = javacall_serial_open_start("COM0", -1, 0, &uart_handle, NULL);
        if (result != JAVACALL_OK) {
            return;
        }
        log_port_opened = 1;
    }
}

void printToUart(char* buffer) {
    int bytesWritten;
    javacall_result result;

    if (!log_port_opened) {
        enableLogPort();
        if (!log_port_opened) {
            return;
        }
    }

    result = javacall_serial_write_start(uart_handle, (unsigned char *)buffer, strlen(buffer), &bytesWritten, NULL);
    if (result == JAVACALL_OK) {
        err_count = 0;
    } else {
        err_count++;
        if (err_count >= 5) {
            javacall_serial_close_start(uart_handle, NULL);
            log_port_opened = 0;
            err_count = 0;
        }
    }
}
