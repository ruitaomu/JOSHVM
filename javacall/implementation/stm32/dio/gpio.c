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

void javacall_gpio_init(void) {}

void javacall_gpio_deinit(void) {}

javacall_dio_result javacall_gpio_pin_open(const javacall_int32 port,
        const javacall_int32 pin, const javacall_gpio_dir direction,
        const javacall_gpio_mode mode, const javacall_gpio_trigger_mode trigger,
        const javacall_bool initValue,
        const javacall_bool exclusive,
        /*OUT*/ javacall_handle* pHandle) {
	return JAVACALL_DIO_FAIL;
}

javacall_dio_result javacall_gpio_pin_close(javacall_handle handle) {
	return JAVACALL_DIO_FAIL;
}

javacall_dio_result javacall_gpio_pin_write(const javacall_handle handle,
        const javacall_bool val)
{
	return JAVACALL_DIO_FAIL;
}

javacall_dio_result javacall_gpio_pin_read(const javacall_handle handle,
        /*OUT*/javacall_bool* pVal)
{
	return JAVACALL_DIO_FAIL;
}

javacall_dio_result javacall_gpio_pin_notification_start(const javacall_handle handle)
{
	return JAVACALL_DIO_FAIL;
}

javacall_dio_result javacall_gpio_pin_notification_stop(const javacall_handle handle)
{
	return JAVACALL_DIO_FAIL;
}

javacall_dio_result javacall_gpio_pin_direction_set(const javacall_handle handle, const javacall_bool direction) {
	return JAVACALL_DIO_FAIL;
}

javacall_dio_result javacall_gpio_pin_set_trigger(const javacall_handle handle, const javacall_gpio_trigger_mode trigger) {
	return JAVACALL_DIO_FAIL;
}

