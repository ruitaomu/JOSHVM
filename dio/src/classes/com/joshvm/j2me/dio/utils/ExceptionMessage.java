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

 /*
 * Copyright (c) 2014, 2015, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.  Oracle designates this
 * particular file as subject to the "Classpath" exception as provided
 * by Oracle in the LICENSE file that accompanied this code.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 */

package com.joshvm.j2me.dio.utils;

public final class ExceptionMessage {
    private ExceptionMessage() {}

    public static String format(int format) {
        return strings[format];
    }

	public static String format(int format, Object param1) {
        return strings[format] + param1.toString();
    }

	public static String format(int format, Object param1, Object param2) {
        return strings[format] + param1.toString() + "," + param2.toString();
    }

	public static String format(int format, int param1) {
        return new String(strings[format] + param1);
    }

    public static final int DEVICE_FIRST = 0;
    public static final int DEVICE_LOCKED_BY_OTHER_APP               = DEVICE_FIRST + 0;
    public static final int DEVICE_NULL_CONFIG_OR_INTF               = DEVICE_FIRST + 1;
    public static final int DEVICE_CONFIG_PROBLEM                    = DEVICE_FIRST + 2;
    public static final int DEVICE_EXCLUSIVE_MODE_UNSUPPORTED        = DEVICE_FIRST + 3;
    public static final int DEVICE_NULL_INTF                         = DEVICE_FIRST + 4;
    public static final int DEVICE_NOT_FOUND                         = DEVICE_FIRST + 5;
    public static final int DEVICE_HAS_DIFFERENT_TYPE                = DEVICE_FIRST + 6;
    public static final int DEVICE_NULL_NAME_AND_PROPERTIES          = DEVICE_FIRST + 7;
    public static final int DEVICE_INVALID_ID                        = DEVICE_FIRST + 8;
    public static final int DEVICE_NONUNIQUE_ID                      = DEVICE_FIRST + 9;
    public static final int DEVICE_ALREADY_EXISTING_CONFIG           = DEVICE_FIRST + 10;
    public static final int DEVICE_NEGATIVE_ID                       = DEVICE_FIRST + 11;
    public static final int DEVICE_INVALID_CLASSNAME                 = DEVICE_FIRST + 12;
    public static final int DEVICE_FAULT_CONFIG_INSTANCE             = DEVICE_FIRST + 13;
    public static final int DEVICE_FOUND_BUT_PERIPHERAL_IS_BUSY      = DEVICE_FIRST + 14;
    public static final int DEVICE_DRIVERS_NOT_MATCH                 = DEVICE_FIRST + 15;
    public static final int DEVICE_DRIVER_MISSING                    = DEVICE_FIRST + 16;
    public static final int DEVICE_NULL_ACTIONS                      = DEVICE_FIRST + 17;
    public static final int DEVICE_NULL_NAME                         = DEVICE_FIRST + 18;
    public static final int DEVICE_EMPTY_ACTIONS                     = DEVICE_FIRST + 19;
    public static final int DEVICE_INVALID_PERMISSION                = DEVICE_FIRST + 20;
    public static final int DEVICE_READONLY_PERMISSION_COLLECTION    = DEVICE_FIRST + 21;
    public static final int DEVICE_OPEN_WITH_DEVICENAME_UNSUPPORTED  = DEVICE_FIRST + 22;
    public static final int BUFFER_IS_MODIFIED                       = DEVICE_FIRST + 23;
    public static final int CLONE_ERROR                              = DEVICE_FIRST + 24;
    public static final int ZERO_CAPACITY_BUFFER                     = DEVICE_FIRST + 25;
    public static final int NULL_LISTENER                            = DEVICE_FIRST + 26;
    public static final int DEVICE_LAST = NULL_LISTENER;

    public static final int ADC_FIRST = DEVICE_LAST + 1;
    public static final int ADC_ANOTHER_OPERATION_PROGRESS           = ADC_FIRST + 0;
    public static final int ADC_NONPOSITIVE_INTERVAL                 = ADC_FIRST + 1;
    public static final int ADC_OUT_OF_RANGE_INTERVAL                = ADC_FIRST + 2;
    public static final int ADC_CANNOT_START_ACQUISITION             = ADC_FIRST + 3;
    public static final int ADC_ARGUMENT_LOW_GREATER_THAN_HIGH       = ADC_FIRST + 4;
    public static final int ADC_LAST = ADC_ARGUMENT_LOW_GREATER_THAN_HIGH;

    public static final int ATCMD_FIRST = ADC_LAST + 1;
    public static final int ATCMD_DATA_CONNECTION_UNSUPPORTED        = ATCMD_FIRST + 0;
    public static final int ATCMD_LAST = ATCMD_DATA_CONNECTION_UNSUPPORTED;

    public static final int COUNTER_FIRST = ATCMD_LAST + 1;
    public static final int COUNTER_INVALID_CONTROLLER_NUMBER        = COUNTER_FIRST + 0;
    public static final int COUNTER_INVALID_CHANNEL_NUMBER           = COUNTER_FIRST + 1;
    public static final int COUNTER_INVALID_TYPE                     = COUNTER_FIRST + 2;
    public static final int COUNTER_CONFIG_CANNOT_BE_USED            = COUNTER_FIRST + 3;
    public static final int COUNTER_NOT_STARTED                      = COUNTER_FIRST + 4;
    public static final int COUNTER_NOT_SUSPENDED                    = COUNTER_FIRST + 5;
    public static final int COUNTER_IS_STARTED                       = COUNTER_FIRST + 6;
    public static final int COUNTER_NONPOSITIVE_LIMIT_AND_INTERVAL   = COUNTER_FIRST + 7;
    public static final int COUNTER_IS_SUSPENDED                     = COUNTER_FIRST + 8;
    public static final int COUNTER_LAST = COUNTER_IS_SUSPENDED;

    public static final int DAC_FIRST = COUNTER_LAST + 1;
    public static final int DAC_GENERATION_IS_ACTIVE                 = DAC_FIRST + 0;
    public static final int DAC_NONPOSITIVE_INTERVAL                 = DAC_FIRST + 1;
    public static final int DAC_OUT_OF_RANGE_INTERVAL                = DAC_FIRST + 2;
    public static final int DAC_UNACCEPTABLE_VALUE                   = DAC_FIRST + 3;
    public static final int DAC_CANNOT_START_CONVERSION              = DAC_FIRST + 4;
    public static final int DAC_LAST = DAC_CANNOT_START_CONVERSION;

    public static final int GPIO_FIRST = DAC_LAST + 1;
    public static final int GPIO_INVALID_TRIGGER                     = GPIO_FIRST + 0;
    public static final int GPIO_INVALID_DIRECTION                   = GPIO_FIRST + 1;
    public static final int GPIO_INVALID_MODE                        = GPIO_FIRST + 2;
    public static final int GPIO_ILLEGAL_DIR                         = GPIO_FIRST + 3;
    public static final int GPIO_DIR_UNSUPPORTED_BY_PIN_CONFIG       = GPIO_FIRST + 4;
    public static final int GPIO_SET_TO_INPUT_PIN                    = GPIO_FIRST + 5;
    public static final int GPIO_REGISTER_LISTENER_TO_OUTPUT_PIN     = GPIO_FIRST + 6;
    public static final int GPIO_CANNOT_START_NOTIFICATION           = GPIO_FIRST + 7;
    public static final int GPIO_LISTENER_ALREADY_ASSIGNED           = GPIO_FIRST + 8;
    public static final int GPIO_DIR_SHOULD_BE_INPUT_OR_OUTPUT       = GPIO_FIRST + 9;
    public static final int GPIO_INCOMPATIBLE_DIR                    = GPIO_FIRST + 10;
    public static final int GPIO_WRITE_TO_INPUT_PORT                 = GPIO_FIRST + 11;
    public static final int GPIO_REGISTER_LISTENER_TO_OUTPUT_PORT    = GPIO_FIRST + 12;
    public static final int GPIO_INCOMPATIBLE_MODE                   = GPIO_FIRST + 13;
    public static final int GPIO_LAST = GPIO_INCOMPATIBLE_MODE;

    public static final int I2CBUS_FIRST = GPIO_LAST + 1;
    public static final int I2CBUS_ALREADY_TRANSFERRED_MESSAGE       = I2CBUS_FIRST + 0;
    public static final int I2CBUS_NEGATIVE_SKIP_ARG                 = I2CBUS_FIRST + 1;
    public static final int I2CBUS_DIFFERENT_BUS_SLAVE_OPERATION     = I2CBUS_FIRST + 2;
    public static final int I2CBUS_BUFFER_GIVEN_TWICE                = I2CBUS_FIRST + 3;
    public static final int I2CBUS_CLOSED_DEVICE                     = I2CBUS_FIRST + 4;
    public static final int I2CBUS_FIRST_MESSAGE                     = I2CBUS_FIRST + 5;
    public static final int I2CBUS_LAST_MESSAGE                      = I2CBUS_FIRST + 6;
    public static final int I2CBUS_LAST = I2CBUS_LAST_MESSAGE;

    public static final int MMIO_FIRST = I2CBUS_LAST + 1;
    public static final int MMIO_NEGATIVE_SIZE                       = MMIO_FIRST + 0;
    public static final int MMIO_NEGATIVE_OFFSET                     = MMIO_FIRST + 1;
    public static final int MMIO_INVALID_TYPE                        = MMIO_FIRST + 2;
    public static final int MMIO_INVALID_DEVICE_PARAMETERS           = MMIO_FIRST + 3;
    public static final int MMIO_ADDRESS_ACCESS_NOT_ALLOWED          = MMIO_FIRST + 4;
    public static final int MMIO_REGISTER_TYPE_UNSUPPORTED           = MMIO_FIRST + 5;
    public static final int MMIO_INVALID_BLOCK_PARAMETERS            = MMIO_FIRST + 6;
    public static final int MMIO_MIXED_ENDIANNESS_UNSUPPORTED        = MMIO_FIRST + 7;
    public static final int MMIO_INVALID_INDEX                       = MMIO_FIRST + 8;
    public static final int MMIO_LAST = MMIO_INVALID_INDEX;

    public static final int POWER_FIRST = MMIO_LAST + 1;
    public static final int POWER_INVALID_STATE_MASK                 = POWER_FIRST + 0;
    public static final int POWER_INVALID_STATE                      = POWER_FIRST + 1;
    public static final int POWER_INVALID_DURATION                   = POWER_FIRST + 2;
    public static final int POWER_ALREADY_ASSIGNED_HANDLER           = POWER_FIRST + 3;
    public static final int POWER_STANDBY_MODE                       = POWER_FIRST + 4;
    public static final int POWER_LAST = POWER_STANDBY_MODE;

    public static final int PWM_FIRST = POWER_LAST + 1;
    public static final int PWM_OUTPUT_PIN_NOT_CONFIGURED            = PWM_FIRST + 0;
    public static final int PWM_NONPOSITIVE_PERIOD                   = PWM_FIRST + 1;
    public static final int PWM_OUT_OF_RANGE_PERIOD                  = PWM_FIRST + 2;
    public static final int PWM_ILLEGAL_WIDTH_OR_COUNT               = PWM_FIRST + 3;
    public static final int PWM_GENERATION_SESSION_ACTIVE            = PWM_FIRST + 4;
    public static final int PWM_LAST = PWM_GENERATION_SESSION_ACTIVE;

    public static final int SPIBUS_FIRST = PWM_LAST + 1;
    public static final int SPIBUS_SLAVE_WORD_LENGTH                 = SPIBUS_FIRST + 0;
    public static final int SPIBUS_BYTE_NUMBER_BELIES_WORD_LENGTH    = SPIBUS_FIRST + 1;
    public static final int SPIBUS_BYTE_BUFFER_MODIFICATION          = SPIBUS_FIRST + 2;
    public static final int SPIBUS_TRANSFER_INTERRUPTED              = SPIBUS_FIRST + 3;
    public static final int SPIBUS_LAST = SPIBUS_TRANSFER_INTERRUPTED;

    public static final int UART_FIRST = SPIBUS_LAST + 1;
    public static final int UART_CANT_GET_PORT_NAME                  = UART_FIRST + 0;
    public static final int UART_UTF8_UNCONVERTIBLE_DEVNAME          = UART_FIRST + 1;
    public static final int UART_ACTIVE_READ_OPERATION               = UART_FIRST + 2;
    public static final int UART_ACTIVE_WRITE_OPERATION              = UART_FIRST + 3;
    public static final int UART_UNKNOWN_SIGNAL_ID                   = UART_FIRST + 4;
    public static final int UART_SIGNALS_NOT_BITWISE_COMBINATION     = UART_FIRST + 5;
    public static final int UART_LISTENER_ALREADY_REGISTERED         = UART_FIRST + 6;
    public static final int UART_NEGATIVE_TIMEOUT                    = UART_FIRST + 7;

    private static final String strings[] = {

        // core classes messages
        "Locked by other application",
        "config or intf is null",
        "There is problem with configuration: ",
        "EXCLUSIVE access mode is not supported",
        "intf is null",
        "Device not found: ",
        "Device has different type: ",
        "Name and properties are NULL",
        "id is not equal to UNSPECIFIED_ID or is not greater than or equal to 0",
        "Device ID must be unique",
        "There is configuration with such name, type and proprties",
        "Device ID must be positive or 0",
        "Invalid class name: ",
        "Invalid instance of DeviceConfig class",
        "Device found by driver loader but peripheral is busy",
        "Neither embedded nor installed driver knows about peripheral",
        "There is no driver",
        "actions is null",
        "Name is null",
        "actions are empty",
        "Invalid permission class: ",
        "Cannot add a Permission to a readonly PermissionCollection",
        "Opening with controllerName is unsupported",
        "Buffer was modified by application",
        "The object can't be cloned",
        "The buffer has a zero-capacity",
        "listener is null",

        // adc messages
        "Another operation on ADC channel is in progress",
        "'interval' is negative or 0",
        "'interval' is out of the supported range",
        "Cannot start acquisition",
        "Argument 'low' is greater than 'high'",

        // atcmd messages
        "Emulator does not support data connection",

        // counter messages
        "Invalid controllerNumber",
        "Invalid channelNumber",
        "Invalid type",
        "Provided config cannot be used as pulse input",
        "Counting has not been started yet",
        "Counting wasn't suspended",
        "Counting is already started",
        "Both limit and interval are equal or less than 0",
        "Counting is already suspended",

        // dac messages
        "Generation is already active",
        "'interval' is negative or 0",
        "'interval' is out of the supported range",
        "'value' is out of an allowed range",
        "Cannot start conversion",

        // gpio messages
        "Invalid trigger",
        "Invalid direction",
        "Invalid mode",
        "Illegal direction or initValue",
        "Pin config does not support required direction",
        "Try to Set value to input pin",
        "Try to register Listener to output pin",
        "Cannot start notification",
        "The listener is already assigned",
        "Direction should be INPUT or OUTPUT",
        "Incompatible direction",
        "Trying to write to input port",
        "Try to register Listener to output port",
        "Incompatible mode",


        // i2c bus messages
        "the message has already been transferred once",
        "'skip' argument is negative",
        "operation to a slave on a different bus",
        "the same buffer is given twice",
        "combined message with closed device",
        "first message",
        "last message",

        // mmio messages
        "size is negative",
        "offset is negative",
        "Invalid type: ",
        "Invalid MMIO device parameters",
        "Address access is not allowed: ",
        "Unsupported register type: ",
        "Invalid block parameters",
        "Mixed endiannes is not supported",
        "invalid index value",

        // power messages
        "Invalid power state mask ",
        "Invalid power state ",
        "Invalid duration",
        "Handler is already assigned",
        "Standby mode",

        // pwm messages
        "output pin is not configured for output",
        "Period is negative or zero: ",
        "Period is out of the supported range: ",
        "width or count is illegal",
        "pulse generation session is already active",

        // spi bus messages
        "Slave Word Length is ",
        "the number of bytes to receive/send belies word length",
        "The original read buffer was modified after append",
        "Delay operation in composite message was interrupted",

        // uart messages
        "Cannot get serial port name",
        "Unable to convert dev name to UTF-8",
        "another synchronous or asynchronous read operation is already active",
        "another synchronous or asynchronous write operation is already active",
        "signalID is not one of the defined values",
        "signals is not a bit-wise combination of valid signal IDs.",
        "listener is not null and a listener is already registered",
        "timeout cannot be negative",
    };
}
