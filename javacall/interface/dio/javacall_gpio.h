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

#ifndef __JAVACALL_GPIO_H
#define __JAVACALL_GPIO_H

#ifdef __cplusplus
extern "C"{
#endif

/**
 * @file javacall_gpio.h
 * @ingroup GPIOAPI
 * @brief Javacall interfaces for GPIO device access JSR
 *
 */

#include "javacall_defs.h"
#include "javacall_dio.h"

/**
 * @defgroup GPIOAPI GPIO API
 * @ingroup DeviceAccess
 * @{
 */

/**
 * @defgroup MandatoryGPIO Mandatory GPIO API
 * @ingroup GPIOAPI
 *
 *
 * @{
 */

/**
 * @enum javacall_gpio_dir
 * @brief pin/port direction values
 */
typedef enum {
    /** Input only direction */
    JAVACALL_GPIO_INPUT_MODE  = 0,
    /** Output only direction */
    JAVACALL_GPIO_OUTPUT_MODE = 1,
    /** Both direction mode, initial state is input */
    JAVACALL_GPIO_BOTH_MODE_INIT_INPUT = 2,
    /** Both direction mode, initial state is output */
    JAVACALL_GPIO_BOTH_MODE_INIT_OUTPUT = 3
}javacall_gpio_dir;

/** Used to indicate that the default value of a configuration parameter is necessary*/
#define JAVACALL_DEFAULT_INT32_VALUE    (-1)
/**
 * @enum  javacall_gpio_mode
 * @brief The drive mode of the pin or the port is got with
 * bitwise OR of this values */
typedef enum {
    /** Input mode, I/O pull-up  */
    JAVACALL_MODE_INPUT_PULL_UP  = 1,
    /** Input mode, I/O pull-down */
    JAVACALL_MODE_INPUT_PULL_DOWN = 2,
    /** Output mode, I/O push-up */
    JAVACALL_MODE_OUTPUT_PUSH_PULL = 4,
    /** Output mode, I/O open-drain */
    JAVACALL_MODE_OUTPUT_OPEN_DRAIN  = 8
} javacall_gpio_mode;


/**
 * @enum javacall_gpio_trigger_mode
 * @brief the interrupt trigger events
 */
typedef enum {
    /** No interrupt trigger. */
    JAVACALL_TRIGGER_NONE = 0,
    /** Falling edge trigger. */
    JAVACALL_TRIGGER_FALLING_EDGE = 1,
    /** Rising edge trigger. */
    JAVACALL_TRIGGER_RISING_EDGE = 2,
    /** Rising edge trigger. */
    JAVACALL_TRIGGER_BOTH_EDGES = 3,
    /** High level trigger. */
    JAVACALL_TRIGGER_HIGH_LEVEL = 4,
    /** Low level trigger. */
    JAVACALL_TRIGGER_LOW_LEVEL = 5,
    /** Both levels trigger. */
    JAVACALL_TRIGGER_BOTH_LEVELS = 6,
} javacall_gpio_trigger_mode;

/**
 * Open GPIO pin by given pin number.
 * <p>
 * A peripheral device may be opened in shared mode if supported
 * by the underlying driver and hardware and if it is not
 * already opened in exclusive mode. A peripheral device may be
 * opened in exclusive mode if supported by the underlying
 * driver and hardware and if it is not already opened.
 *
 * @param port hardware GPIO Port's number
 * @param pin hardware GPIO Pin's number in the port
 * @param direction direction for pin JAVACALL_GPIO_INPUT_MODE, JAVACALL_GPIO_OUTPUT_MODE,
 *                  JAVACALL_GPIO_BOTH_MODE_INIT_INPUT or JAVACALL_GPIO_BOTH_MODE_INIT_OUTPUT
 * @param mode the drive mode of the pin, a bitwise OR of drive mode possible values
 * @param trigger the interrupt trigger events, one of javacall_gpio_trigger_mode
 * @param initValue the initial value of the pin when direction set for output
 * @param exclusive      exclusive mode flag: JAVACALL_TRUE
 *                       means EXCLUSIVE mode, SHARED for the
 *                       rest
 * @param pHandle pointer to store the GPIO pin handle
 *
 * @retval JAVACALL_DIO_OK          success
 * @retval JAVACALL_DIO_FAIL        general I/O error
 * @retval JAVACALL_DIO_NOT_FOUND   Peripheral is not found
 * @retval JAVACALL_DIO_BUSY   attempt to open already opened pin
 *         in exclusive mode or pin was locked by {@link
 *         #javacall_gpio_pin_lock(const javacall_handle,
 *         javacall_handle* const)}
 * @retval JAVACALL_DIO_UNSUPPORTED_ACCESS_MODE    if EXCLUSIVE or SHARED
 *         mode is not supported
 *
 * @note   call parameters must conform specifications
 */
javacall_dio_result javacall_gpio_pin_open(const javacall_int32 port,
        const javacall_int32 pin, const javacall_gpio_dir direction,
        const javacall_gpio_mode mode, const javacall_gpio_trigger_mode trigger,
        const javacall_bool initValue,
        const javacall_bool exclusive,
        /*OUT*/ javacall_handle* pHandle);

/**
 * Release and close the GPIO pin.
 *
 * @param handle handle of the GPIO pin.
 *
 * @retval JAVACALL_DIO_OK          success
 * @retval JAVACALL_DIO_FAIL        if there was an error
 */
javacall_dio_result javacall_gpio_pin_close(javacall_handle handle);

/**
*Read data from given GPIO pin number
*
*@param handle GPIO       pin handle
*@param val                       the pointer to the variable to receive value of GPIO pin
*
*@retval JAVACALL_DIO_OK        read success
*@retval JAVACALL_DIO_FAIL      on read error
*/
javacall_dio_result javacall_gpio_pin_read(const javacall_handle handle,
        /*OUT*/javacall_bool* pVal);

/**
* Write data to GPIO pin
*
*@param     handle GPIO pin handle
*@param     val value to be written to pin
*
*@retval JAVACALL_DIO_OK   write success
*@retval JAVACALL_DIO_FAIL write fail
*/
javacall_dio_result javacall_gpio_pin_write(const javacall_handle handle,
        const javacall_bool val);

/**
* Start notification of given GPIO pin
*
*@see javanotify_gpio_pin_value_changed
*
*@param handle GPIO pin handle
*
*@retval JAVACALL_DIO_OK
*@retval JAVACALL_DIO_UNSUPPORTED_OPERATION  if pin was setup
*          for output
*@retval JAVACALL_DIO_FAIL  general IO error
*/
javacall_dio_result javacall_gpio_pin_notification_start(const javacall_handle handle);

/**
* Stop notification of given GPIO pin
*
*@see javanotify_gpio_pin_value_changed
*
*@param handle GPIO pin handle
*
*@retval JAVACALL_DIO_OK
*/
javacall_dio_result javacall_gpio_pin_notification_stop(const javacall_handle handle);

/**
* change direction of given GPIO pin
*
*@param handle GPIO pin handle
*@param direction 1 is output mode ,0 is input mode
*
*@retval JAVACALL_DIO_OK
* @retval JAVACALL_DIO_UNSUPPORTED_OPERATION  if
*         <code>direction</code> value is not supported
* @retval JAVACALL_DIO_FAIL  general IO error
*/
javacall_dio_result javacall_gpio_pin_direction_set(const javacall_handle handle,
        const javacall_bool direction);

/**
*get current direction of given GPIO pin
*
*@param handle GPIO pin handle
*@param pDirection the pointer to the variable to receive GPIO pin mode
*
*@retval JAVACALL_DIO_OK read pin status success
*/
javacall_dio_result javacall_gpio_pin_direction_get(const javacall_handle handle,
        /*OUT*/javacall_bool* const pDirection);

/**
 * Returns power control group of this pin. It is used for
 * power management notification.
 *
 * @param handle open device handle
 * @param grp    power management group
 *
 * @return javacall_dio_result JAVACALL_DIO_FAIL if the device was
 *         closed, JAVACALL_DIO_OK otherwise
 */
javacall_dio_result javacall_gpio_pin_get_group_id(const javacall_handle handle, javacall_int32* const grp);

/**
 * Attempts to lock for exclusive access the underlying
 * peripheral device resource.
 * <p>
 * Checks for status and returns immediately if the resource is
 * already locked.
 *
 * @param handle of open pin
 * @param owner a pointer to current owner handle if attempt
 *              failed
 *
 * @retval JAVACALL_DIO_OK if exclusive access was granted,
 *         JAVACALL_DIO_FAIL if the resource is locked by other
 *         application
 */
javacall_dio_result javacall_gpio_pin_lock(const javacall_handle handle, javacall_handle* const owner);

/**
 * Releases from exclusive access the underlying peripheral
 * device resource.
 * <p>
 * Returns silently if the resource was not
 * locked to <code>handle</code>
 *
 * @param handle open resource handle
 *
 * @retval JAVACALL_DIO_OK if <code>handle</code> is owner of the
 *         resource and the resuorce is released
 * @retval JAVACALL_DIO_FAIL otherwise
 *
 */
javacall_dio_result javacall_gpio_pin_unlock(const javacall_handle handle);

/**
 * Changes trigger mode of a pin.
 *
 *
 * @param handle    open pin handle
 * @param trigger   trigger mode
 *
 * @retval JAVACALL_DIO_OK if trigger mode was changed
 * @retval JAVACALL_DIO_UNSUPPORTED_OPERATION  if <code>trigger</code> value is not
 *         supported
 * @retval JAVACALL_DIO_FAIL  general IO error
 */
javacall_dio_result javacall_gpio_pin_set_trigger(const javacall_handle handle, const javacall_gpio_trigger_mode trigger);

/**
 * Returns trigger mode of a pin.
 *
 *
 * @param handle    open pin handle
 * @param trigger   a pointer to trigger mode storage
 *
 * @return JAVACALL_DIO_OK if trigger mode was acquired
 */
javacall_dio_result javacall_gpio_pin_get_trigger(const javacall_handle handle, javacall_gpio_trigger_mode* const trigger);


void javacall_gpio_init(void);
void javacall_gpio_deinit(void);
/******************************************************************************
 ******************************************************************************
 ******************************************************************************

  NOTIFICATION FUNCTIONS
  - - - -  - - - - - - -
  The following functions are implemented by CLDC.
  Platform is required to invoke these function for each occurence of the
  undelying event.
  The functions need to be executed in platform's task/thread

 ******************************************************************************
 ******************************************************************************
 ******************************************************************************/

/**
 * @defgroup NotificationGPIOPin Notification API for GPIOPin
 * @ingroup GPIOAPI
 * @{
 */

/**
 * A callback function to be called by platform to notify that an GPIO Pin event
 * has generated.
 * The platfrom will invoke the call back in platform context for
 * each registered GPIO Pin event. See javacall_gpio_pin_notification_start().
 *
 * @param handle Handle value returned from javacall_gpio_pin_open()
 * @param val The current value of Pin
 */
void javanotify_gpio_pin_value_changed(const javacall_handle handle, const int val);

/**
 * A callback function to be called by platform to notify that an GPIO Pin
 * has been closed.
 *
 * @param handle Handle value returned from javacall_gpio_pin_open()
 */
void javanotify_gpio_pin_closed(const javacall_handle handle);


/** @} */
/** @} */


#ifdef __cplusplus
}
#endif

#endif //__JAVACALL_GPIO_H
