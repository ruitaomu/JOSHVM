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
#include <javacall_gpio.h>

#include <rtthread.h>
#include <drivers/pin.h>

#define BUFFLEN	 256
#define PINS_MAX 50

void javacall_gpio_init(void) {
	
}

void javacall_gpio_deinit(void) {
}

static void hdr_callback(void *args)//回调函数
{
    int a = (int)args;//获取参数
    int v = rt_pin_read(a);
    javanotify_gpio_pin_value_changed(a, v);
}

javacall_dio_result javacall_gpio_pin_open(const javacall_int32 port,
        const javacall_int32 pin, const javacall_gpio_dir direction,
        const javacall_gpio_mode mode, const javacall_gpio_trigger_mode trigger,
        const javacall_bool initValue,
        const javacall_bool exclusive,
        /*OUT*/ javacall_handle* pHandle) {
	if (direction == JAVACALL_GPIO_BOTH_MODE_INIT_INPUT ||
		direction == JAVACALL_GPIO_BOTH_MODE_INIT_OUTPUT ||
		trigger == JAVACALL_TRIGGER_BOTH_LEVELS) {
		return JAVACALL_DIO_UNSUPPORTED_ACCESS_MODE;
	}

	rt_base_t pinMode = PIN_MODE_OUTPUT;
	if (direction == JAVACALL_GPIO_INPUT_MODE) {
		pinMode = PIN_MODE_INPUT;
		if (mode == JAVACALL_MODE_INPUT_PULL_UP) {
			pinMode = PIN_MODE_INPUT_PULLUP;
		} else if (mode == JAVACALL_MODE_INPUT_PULL_DOWN) {
			pinMode = PIN_MODE_INPUT_PULLDOWN;
		}
	} else {
		// direction: JAVACALL_GPIO_OUTPUT_MODE
		if (mode == JAVACALL_MODE_OUTPUT_OPEN_DRAIN) {
			pinMode = PIN_MODE_OUTPUT_OD;
		}
	}
	rt_pin_mode(pin, pinMode);

	if (trigger != JAVACALL_TRIGGER_NONE) {
		rt_uint32_t irqMode = PIN_IRQ_MODE_RISING;
		if (trigger == JAVACALL_TRIGGER_FALLING_EDGE) {
			irqMode = PIN_IRQ_MODE_FALLING;
		} else if (trigger == JAVACALL_TRIGGER_BOTH_EDGES) {
			irqMode = PIN_IRQ_MODE_RISING_FALLING;
		} else if (trigger == JAVACALL_TRIGGER_HIGH_LEVEL) {
			irqMode = PIN_IRQ_MODE_HIGH_LEVEL;
		} else if(trigger == JAVACALL_TRIGGER_LOW_LEVEL) {
			irqMode = PIN_IRQ_MODE_LOW_LEVEL;
		}
		rt_pin_attach_irq(pin, irqMode, hdr_callback, (void*)pin);
		rt_pin_irq_enable(pin, PIN_IRQ_ENABLE);
	}
	*pHandle = pin;
	return JAVACALL_DIO_OK;
}

javacall_dio_result javacall_gpio_pin_close(javacall_handle handle) {
	rt_pin_irq_enable((int)handle, PIN_IRQ_DISABLE);
	rt_pin_detach_irq((int)handle);
	return JAVACALL_DIO_OK;
}

javacall_dio_result javacall_gpio_pin_write(const javacall_handle handle, 
        const javacall_bool val)
{
	rt_pin_write(handle, val==JAVACALL_TRUE?PIN_HIGH:PIN_LOW);
	return JAVACALL_DIO_OK;
}

javacall_dio_result javacall_gpio_pin_read(const javacall_handle handle,
        /*OUT*/javacall_bool* pVal)
{
	*pVal = rt_pin_read(handle);
	//javacall_printf("rt_pin_read(%d)->%d\n",handle,*pVal);
	return JAVACALL_DIO_OK;
}

javacall_dio_result javacall_gpio_pin_notification_start(const javacall_handle handle)
{
	return JAVACALL_DIO_OK;
}

javacall_dio_result javacall_gpio_pin_notification_stop(const javacall_handle handle)
{
	return JAVACALL_DIO_OK;
}

javacall_dio_result javacall_gpio_pin_direction_set(const javacall_handle handle, const javacall_bool direction) {
	return JAVACALL_DIO_OK;
}

javacall_dio_result javacall_gpio_pin_set_trigger(const javacall_handle handle, const javacall_gpio_trigger_mode trigger) {
	return JAVACALL_DIO_OK;
}

