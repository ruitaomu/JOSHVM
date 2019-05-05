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

#include <sys/signal.h>
#include <sys/types.h>
#include <poll.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

static void signal_handler_IO(int, siginfo_t *, void *);

static javacall_result serial_read_common(javacall_handle hPort, unsigned char* buffer, int size ,int *bytesRead) {
	int count;
	count = read((int)hPort,buffer,size);

	if (count < 0) {
		if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
			return JAVACALL_WOULD_BLOCK;
		} else {
			return JAVACALL_FAIL;
		}
	} else {
		*bytesRead = count;
		return JAVACALL_OK;
	}
}
static javacall_result serial_write_common(javacall_handle hPort, unsigned char* buffer, int size, int *bytesWritten) {
	int count = write((int)hPort, buffer, size);

	if (count < 0) {
		if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
			return JAVACALL_WOULD_BLOCK;
		} else {
			return JAVACALL_FAIL;
		}
	}

	*bytesWritten = count;
	return JAVACALL_OK;
}

static void signal_handler_IO(int sig, siginfo_t *info, void *ucontext) {
	if (sig == SIGIO) {
		if (info->si_band == POLLIN) {
			javanotify_serial_event(JAVACALL_EVENT_SERIAL_RECEIVE, (javacall_handle)info->si_fd, JAVACALL_OK);
		} else if (info->si_band == POLLOUT) {
			javanotify_serial_event(JAVACALL_EVENT_SERIAL_WRITE, (javacall_handle)info->si_fd, JAVACALL_OK);
		}
	}
}

static javacall_result convert_uart_options(unsigned int options, tcflag_t *flags) {

	unsigned int opt_bps, opt_stopbits, opt_parity, opt_flowcontrol;

	*flags = 0;

	opt_bps = options & (JAVACALL_SERIAL_BITS_PER_CHAR_7 | JAVACALL_SERIAL_BITS_PER_CHAR_8);
	if (opt_bps == JAVACALL_SERIAL_BITS_PER_CHAR_7) {
		*flags |= CS7;
	} else if (opt_bps == JAVACALL_SERIAL_BITS_PER_CHAR_8) {
		*flags |= CS8;
	} else {
		return JAVACALL_FAIL;
	}

	opt_stopbits = options & JAVACALL_SERIAL_STOP_BITS_2;
	if (opt_stopbits != 0)	{
		*flags |= CSTOPB;
	}

	opt_parity = options & (JAVACALL_SERIAL_ODD_PARITY | JAVACALL_SERIAL_EVEN_PARITY);
	if (opt_parity != 0) {
		*flags |= PARENB;
		if (opt_parity == JAVACALL_SERIAL_ODD_PARITY) {
			*flags |= PARODD;
		}
	}

	opt_flowcontrol = options & (JAVACALL_SERIAL_AUTO_RTS | JAVACALL_SERIAL_AUTO_CTS);
	if (opt_flowcontrol != 0) {
		return JAVACALL_FAIL;
	}

	return JAVACALL_OK;
}

javacall_result convert_uart_baud(int baudRate, tcflag_t* sysBaudRate) {
	switch (baudRate) {
	case 50:
		*sysBaudRate = B50;
		break;
	case 75:
		*sysBaudRate = B75;
		break;
	case 110:
		*sysBaudRate = B110;
		break;
	case 134:
		*sysBaudRate = B134;
		break;
	case 150:
		*sysBaudRate = B150;
		break;
	case 200:
		*sysBaudRate = B200;
		break;
	case 300:
		*sysBaudRate = B300;
		break;
	case 600:
		*sysBaudRate = B600;
		break;
	case 1200:
		*sysBaudRate = B1200;
		break;
	case 1800:
		*sysBaudRate = B1800;
		break;
	case 2400:
		*sysBaudRate = B2400;
		break;
	case 4800:
		*sysBaudRate = B4800;
		break;
	case 9600:
		*sysBaudRate = B9600;
		break;
	case 19200:
		*sysBaudRate = B19200;
		break;
	case 38400:
		*sysBaudRate = B38400;
		break;
	case 57600:
		*sysBaudRate = B57600;
		break;
	case 115200:
		*sysBaudRate = B115200;
		break;
	case 230400:
		*sysBaudRate = B230400;
	default:
		return JAVACALL_FAIL;
	}
	return JAVACALL_OK;
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
	char* ports = "COM0,COM1";
	if (strlen(ports) >= maxBufLen) {
		return JAVACALL_FAIL;
	}
	strcpy(buffer, ports);
    return JAVACALL_OK;

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
	tcflag_t flags, baud;
	struct termios newtio;

	if (JAVACALL_OK != convert_uart_baud(baudRate, &baud)) {
		return JAVACALL_FAIL;
	}

	if (JAVACALL_OK != convert_uart_options(options, &flags)) {
		return JAVACALL_FAIL;
	}

	newtio.c_cflag = baud|flags|CLOCAL|CREAD;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;
	newtio.c_lflag = 0;
	newtio.c_cc[VMIN]=0;
	newtio.c_cc[VTIME]=0;
	tcsetattr((int)pHandle,TCSAFLUSH,&newtio);

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
	int fd;
	struct termios newtio;
	struct sigaction saio; 		  /* definition of signal action */
	sigset_t sigset;
	char* pName;
	tcflag_t flags, baud;

	if (!strcmp(devName, "COM0")) {
		pName = "/dev/ttyS0";
	} else if (!strcmp(devName, "COM1")) {
		pName = "/dev/ttyS1";
	} else {
		return JAVACALL_FAIL;
	}

	if (JAVACALL_OK != convert_uart_baud(baudRate, &baud)) {
		return JAVACALL_FAIL;
	}

	if (JAVACALL_OK != convert_uart_options(options, &flags)) {
		return JAVACALL_FAIL;
	}

	/* open the device to be non-blocking (read will return immediatly) */
	fd = open(pName, O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (fd < 0) {
		return JAVACALL_FAIL;
	}

	/* install the signal handler before making the device asynchronous */
	sigemptyset(&sigset);
	saio.sa_sigaction = signal_handler_IO;
	saio.sa_mask = sigset;
	saio.sa_flags = SA_SIGINFO;
	saio.sa_restorer = NULL;
	sigaction(SIGIO,&saio,NULL);

	/* allow the process to receive SIGIO */
	fcntl(fd, F_SETOWN, getpid());
	/* Make the file descriptor asynchronous */
	fcntl(fd, F_SETFL, FASYNC);

	newtio.c_cflag = baud|flags|CLOCAL|CREAD;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;
	newtio.c_lflag = 0;
	newtio.c_cc[VMIN]=0;
	newtio.c_cc[VTIME]=0;
	tcflush(fd, TCIOFLUSH);
	tcsetattr(fd,TCSANOW,&newtio);

	*hPort = (javacall_handle)fd;

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
	//lsh_oem_uart_close((int)hPort);
	close((int)hPort);
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
