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

#include <javacall_spi.h>
#include <javacall_logging.h>

#include "driver/spi_master.h"
#include "soc/gpio_struct.h"
#include "driver/gpio.h"
#include "esp_heap_caps.h"

#if USE_JOSH_EVB
#define PIN_NUM_MISO 21
#define PIN_NUM_MOSI 25
#define PIN_NUM_CLK  26
#define PIN_NUM_CS   -1
#else
#define PIN_NUM_MISO 25
#define PIN_NUM_MOSI 23
#define PIN_NUM_CLK  19
#define PIN_NUM_CS   22
#endif

#define MAX_TRANSFER_SIZE 8192
#define TRANS_QUEUE_LENGTH 1

static javacall_spi_cs_active cs_active_setting;

static void enable_cs_pin(int init_val) {
#if 0
	gpio_config_t io_conf;
	//disable interrupt
	io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
	//set as output mode
	io_conf.mode = GPIO_MODE_OUTPUT;
	//bit mask of the pins that you want to set,e.g.GPIO18/19
	io_conf.pin_bit_mask = BIT64(PIN_NUM_CS);
	//disable pull-down mode
	io_conf.pull_down_en = 0;
	//disable pull-up mode
	io_conf.pull_up_en = 0;
	//configure GPIO with the given settings
	gpio_config(&io_conf);
	gpio_set_level((gpio_num_t)PIN_NUM_CS, init_val);
#endif
}

javacall_dio_result javacall_spi_open(javacall_int32 busNumber,
        javacall_int32 address, javacall_spi_cs_active csActive, javacall_int32 clockFrequency,
        javacall_int32 clockMode, javacall_int32 wordLength,
        javacall_byteorder bitOrdering,
        const javacall_bool exclusive,
        /*OUT*/javacall_handle* pHandle) {
	esp_err_t ret;
	spi_host_device_t host;
	spi_device_handle_t spi;
	spi_bus_config_t buscfg={
			.miso_io_num=PIN_NUM_MISO,
			.mosi_io_num=PIN_NUM_MOSI,
			.sclk_io_num=PIN_NUM_CLK,
			.quadwp_io_num=-1,
			.quadhd_io_num=-1,
			.max_transfer_sz=MAX_TRANSFER_SIZE
	};
	spi_device_interface_config_t devcfg={
			.clock_speed_hz=clockFrequency,			//Clock out
			.mode=clockMode,						//SPI mode
			.queue_size=TRANS_QUEUE_LENGTH,			//We want to be able to queue TRANS_QUEUE_LENGTH transactions at a time
			.pre_cb=NULL,
			.post_cb=NULL,
	};

	switch (busNumber) {
	case 1:
	case JAVACALL_SPI_BUSNUMBER_DEFAULT:
		host = HSPI_HOST;
		break;
	default:
		return JAVACALL_DIO_FAIL;
	}

	if ((csActive==DAAPI_SPI_CS_NOT_CONTROLLED)||(csActive==DAAPI_SPI_CS_DEFAULT)) {
		devcfg.spics_io_num = -1;
	} else {
		devcfg.spics_io_num = PIN_NUM_CS;
		if (csActive == DAAPI_SPI_CS_ACTIVE_HIGH) {
			devcfg.flags |= SPI_DEVICE_POSITIVE_CS;
			enable_cs_pin(0);
		} else {
			devcfg.flags &= ~SPI_DEVICE_POSITIVE_CS;
			enable_cs_pin(1);
		}
	}

	cs_active_setting = csActive;

	if (bitOrdering == DAAPI_LITTLE_ENDIAN) {
		devcfg.flags |= SPI_DEVICE_BIT_LSBFIRST;
	} else {
		devcfg.flags &= ~SPI_DEVICE_BIT_LSBFIRST;
	}
#if 1
	
	ret=spi_bus_initialize(host, &buscfg, 1);
	if (ret != ESP_OK) {
		return JAVACALL_DIO_FAIL;
	}

	ret=spi_bus_add_device(host, &devcfg, &spi);
	if (ret != ESP_OK) {
		spi_bus_free(HSPI_HOST);
		return JAVACALL_DIO_FAIL;
	}
#endif
	javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_DAAPI, "javacall_spi_open: clockFrequencey = %d, clockMode = %d, csActive = %d, devcfg.flags = 0x%x, bufcfg.flags = 0x%x\n",
					clockFrequency, clockMode, csActive,  
					devcfg.flags, buscfg.flags);
	*pHandle = spi;

	return JAVACALL_DIO_OK;
}

void javacall_spi_close(javacall_handle handle) {
	esp_err_t ret;
	spi_bus_remove_device((spi_device_handle_t)handle);
	
	if ((ret = spi_bus_free(HSPI_HOST)) != ESP_OK) {
		javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_DAAPI,"javacall_spi_close: spi_bus_free failed, error code %d\n", ret);
	}
#if 0
	if (TxBuf) {
		javacall_free(TxBuf);
		TxBuf = NULL;
	}
	
	if (RxBuf) {
		javacall_free(RxBuf);
		RxBuf = NULL;
	}
#endif
	return;
}

javacall_dio_result javacall_spi_set_cs(javacall_handle handle, javacall_bool value) {
#if 0
	spi_device_handle_t spi = handle;
	
	if ((cs_active_setting == DAAPI_SPI_CS_NOT_CONTROLLED) || (cs_active_setting == DAAPI_SPI_CS_DEFAULT)) {
		return JAVACALL_OK;//Not controlled by driver
	}

	gpio_set_level(PIN_NUM_CS, value?1:0);
#endif
	
	return JAVACALL_DIO_OK;
}

#if 0
static int wait_queue_finish(spi_device_handle_t spi)
{
    spi_transaction_t *rtrans;
    esp_err_t ret;
	int completed = 0;	
	
    for (int x=0; x<TRANS_QUEUE_LENGTH; x++) {
        ret=spi_device_get_trans_result(spi, &rtrans, portMAX_DELAY);
		if (ret != ESP_OK) {
			break;
		} else {
			completed++ ;
		}
    }

	return completed;
}


static int queue = 0;
#endif
int javacall_spi_send_and_receive_sync(javacall_handle handle,const char* pTxBuf, /*OUT*/char* pRxBuf, int len) {
	spi_transaction_t t;
	esp_err_t ret;
	
    memset(&t, 0, sizeof(t));
		
	int transfered = 0;	
	while (transfered < len) {
		int toTransfer = len - transfered;
		if (toTransfer > MAX_TRANSFER_SIZE) {
			toTransfer = MAX_TRANSFER_SIZE;
		}
		
		t.length=t.rxlength=8*toTransfer;
		t.tx_buffer = pTxBuf+transfered;
		t.rx_buffer = pRxBuf+transfered;
		
	    esp_err_t ret = spi_device_transmit((spi_device_handle_t)handle, &t);
		
#if 0
		if (queue == TRANS_QUEUE_LENGTH) {
			int finished = wait_queue_finish((spi_device_handle_t)handle);
			queue -= finished;
			if (queue != 0) {				
				javacall_printf("javacall_spi_send_and_receive: Failed when get trans result\n");
				return transfered;
			}
		}

		assert(queue >= 0 && queue < TRANS_QUEUE_LENGTH);

		//javacall_printf("trans: %d, toTrans: %d, queue: %d\n",
		//	transfered, toTransfer, queue);
		
		
		ret = spi_device_queue_trans((spi_device_handle_t)handle, &t, portMAX_DELAY);
#endif
		if (ret == ESP_OK) {
			transfered += toTransfer;
			//queue++;
		} else {
			//javacall_printf("javacall_spi_send_and_receive: spi transmit %d bytes failed for reason %d, after %d bytes transfered\n", toTransfer, ret, transfered);
			if (transfered == 0) {
				return -1;
			} else {
				return transfered;
			}
		}
	}
	//wait_queue_finish((spi_device_handle_t)handle);
	return transfered;
}

