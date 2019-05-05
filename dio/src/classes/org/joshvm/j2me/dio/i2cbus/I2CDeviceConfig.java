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
 * Copyright (c) 2013, 2015, Oracle and/or its affiliates. All rights reserved.
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
package org.joshvm.j2me.dio.i2cbus;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import org.joshvm.j2me.dio.DeviceConfig;
import org.joshvm.j2me.dio.DeviceManager;
import org.joshvm.j2me.dio.InvalidDeviceConfigException;
import org.joshvm.j2me.dio.DeviceConfig.HardwareAddressing;

import com.joshvm.j2me.dio.utils.Utils;

/**
 * The {@code I2CDeviceConfig} class encapsulates the hardware addressing information, and static
 * and dynamic configuration parameters of an I2C slave device.
 * <p>
 * Some hardware addressing, static or dynamic configuration parameters may be
 * set to {@link #UNASSIGNED UNASSIGNED} or {@code null} (see
 * <a href="{@docRoot}/org/joshvm/dio/DeviceConfig.html#default_unassigned">Unassigned, Default or Unused Parameter Values</a>).
 * </p><p>
 * An instance of {@code I2CDeviceConfig} can be passed to the
 * {@link DeviceManager#open(DeviceConfig) open(DeviceConfig, ...)} and
 * {@link DeviceManager#open(Class, DeviceConfig) open(Class, DeviceConfig, ...)}
 * methods of the {@link DeviceManager} to open the designated I2C slave
 * device with the specified configuration. A {@link InvalidDeviceConfigException} is thrown
 * when attempting to open a device with an invalid or unsupported configuration.
 * </p>
 *
 * @see DeviceManager#open(DeviceConfig)
 * @see DeviceManager#open(DeviceConfig, int)
 * @see DeviceManager#open(Class, DeviceConfig)
 * @see DeviceManager#open(Class, DeviceConfig, int)
 * @since 1.0
 */
public final class I2CDeviceConfig implements DeviceConfig, DeviceConfig.HardwareAddressing {

    /**
     * 7 bit slave address.
     */
    public static final int ADDR_SIZE_7 = 7;
    /**
     * 10 bit slave address.
     */
    public static final int ADDR_SIZE_10 = 10;

    private String controllerName;
    private int address = UNASSIGNED;
    private int addressSize = UNASSIGNED;
    private int controllerNumber = UNASSIGNED;
    private int clockFrequency = UNASSIGNED;
    private int inputBufferSize = UNASSIGNED;
    private int outputBufferSize = UNASSIGNED;

    /**
     * The {@code Builder} class allows for creating and initializing
     * {@code I2CDeviceConfig} objects. Calls can be chained in the following
     * manner:
     * <blockquote>
     * <pre>
     *   I2CDeviceConfig config = new I2CDeviceConfig.Builder()
     *           .setControllerNumber(1)
     *           .setAddress(0x300, ADDR_SIZE_10)
     *           .setClockFrequency(8000000)
     *           .build();
     * </pre>
     * </blockquote>
     *
     * @since 1.1
     */
    public static final  class Builder {

        private  final I2CDeviceConfig instance = new I2CDeviceConfig();

        /**
         * Creates a new {@code Builder} instance.
         */
        public Builder() {

        }

        /**
         * Creates a new {@code I2CDeviceConfig} instance initialized with the
         * values set for each configuration parameters. If a configuration
         * parameter was not explictly set its default value will be used.
         *
         * @return a new initialized {@code I2CDeviceConfig} instance.
         * @throws IllegalStateException if any of the following is true:
         * <ul>
         * <li>the address of the slave device on the bus is not set.</li>
         * </ul>
         */
        public I2CDeviceConfig build() {
            if (UNASSIGNED == instance.address) {
                throw new IllegalStateException();
            }
            I2CDeviceConfig newinstance = new I2CDeviceConfig();
            newinstance.controllerName = instance.controllerName;
            newinstance.address = instance.address;
            newinstance.addressSize = instance.addressSize;
            newinstance.controllerNumber = instance.controllerNumber;
            newinstance.clockFrequency = instance.clockFrequency;
            newinstance.inputBufferSize = instance.inputBufferSize;
            newinstance.outputBufferSize = instance.outputBufferSize;
            return newinstance;
        }

        /**
         * Sets the name of the bus the slave device is connected to (default value is {@code null} if not set).
         *
         * @param controllerName the name of the bus the slave device is connected to (such as its <em>device
         * file</em> name on UNIX systems) or {@code null}.
         * @return this {@code Builder} instance.
         */
        public Builder setControllerName(String controllerName) {
            instance.controllerName = controllerName;
            return this;
        }

        /**
         * Sets the address of the slave device on the bus.
         *
         * @param address the address of the slave device on the bus (a positive
         * or zero integer, strictly lesser than 2<sup>{@code addressSize}</sup>).
         * @param addressSize the address size: {@link #ADDR_SIZE_7} bits,
         * {@link #ADDR_SIZE_10} bits.
         * @return this {@code Builder} instance.
         * @throws IllegalArgumentException if any of the following is true:
         * <ul>
         * <li>{@code address} is not in the defined range;</li>
         * <li>{@code addressSize} is not in the defined range;</li>
         * </ul>
         */
        public Builder setAddress(int address, int addressSize) {
            Utils.checkIntZeroOrPozitive(address);
            checkAddressSize(address, addressSize);
            instance.address = address;
            instance.addressSize = addressSize;
            return this;
        }

        /**
         * Sets the number of the bus the slave device is connected to (default
         * value is {@code UNASSIGNED} if not set).
         *
         * @param controllerNumber the number of the bus the slave device is
         * connected to (a positive or zero integer) or {@link #UNASSIGNED UNASSIGNED}.
         * @return this {@code Builder} instance.
         * @throws IllegalArgumentException if {@code controllerNumber} is not
         * in the defined range.
         */
        public Builder setControllerNumber(int controllerNumber) {
            Utils.checkIntValue(controllerNumber);
            instance.controllerNumber = controllerNumber;
            return this;
        }

        /**
         * Sets the requested input buffer size (default value is
         * {@code UNASSIGNED} if not set).
         *
         * @param inputBufferSize the requested input buffer size in number of
         * samples (a positive or zero integer) or
         * {@link #UNASSIGNED UNASSIGNED}.
         * @return this {@code Builder} instance.
         * @throws IllegalArgumentException if {@code inputBufferSize} is not in
         * the defined range.
         */
        public Builder setInputBufferSize(int inputBufferSize) {
            Utils.checkIntValue(inputBufferSize);
            instance.inputBufferSize = inputBufferSize;
            return this;
        }

        /**
         * Sets the requested output buffer size (default value is
         * {@code UNASSIGNED} if not set).
         *
         * @param outputBufferSize the requested output buffer size in number of
         * samples (a positive or zero integer) or
         * {@link #UNASSIGNED UNASSIGNED}.
         * @return this {@code Builder} instance.
         * @throws IllegalArgumentException if {@code outputBufferSize} is not
         * in the defined range.
         */
        public Builder setOutputBufferSize(int outputBufferSize) {
            Utils.checkIntValue(outputBufferSize);
            instance.outputBufferSize = outputBufferSize;
            return this;
        }

        /**
         * Sets the clock frequency of the slave device in Hz (default value is
         * {@code UNASSIGNED} if not set).
         *
         * @param clockFrequency the clock frequency of the slave device in Hz
         * (a positive integer) or {@link #UNASSIGNED UNASSIGNED}.
         * @return this {@code Builder} instance.
         * @throws IllegalArgumentException if {@code clockFrequency} is not in
         * the defined range.
         */
        public Builder setClockFrequency(int clockFrequency) {
            Utils.checkGreaterThanZero(clockFrequency);
            instance.clockFrequency = clockFrequency;
            return this;
        }
    }




    // hidden constructor for serializer
    I2CDeviceConfig() {}

    /**
     * Creates a new {@code I2CDeviceConfig} with the specified hardware addressing information and
     * configuration parameters.
     * <p>
     * The controller name is set to {@code null}.
     * The requested input and output buffer sizes are set to {@code UNASSIGNED}.
     * </p>
     *
     * @param controllerNumber
     *            the number of the bus the slave device is connected to (a positive or zero
     *            integer) or {@link #UNASSIGNED UNASSIGNED}.
     * @param address the address of the slave device on the bus (a positive or zero integer, strictly lesser than
     * 2<sup>{@code addressSize}</sup>).
     * @param addressSize the address size: {@link #ADDR_SIZE_7} bits, {@link #ADDR_SIZE_10} bits.
     * @param clockFrequency
     *            the clock frequency of the slave device in Hz (a positive integer) or
     *            {@link #UNASSIGNED UNASSIGNED}.
     * @throws IllegalArgumentException
     *             if any of the following is true:
     *             <ul>
     *             <li>{@code controllerNumber} is not in the defined range;</li>
     *             <li>{@code address} is not in the defined range;</li>
     *             <li>{@code addressSize} is not in the defined range;</li>
     *             <li>{@code clockFrequency} is not in the defined range.</li>
     *             </ul>
     *
     * @deprecated As of 1.1, use {@link Builder} instead.
     */
    public I2CDeviceConfig(int controllerNumber, int address, int addressSize, int clockFrequency) {
        this.controllerNumber = controllerNumber;
        this.address = address;
        this.addressSize = addressSize;
        this.clockFrequency = clockFrequency;
        checkValues();
    }

    /**
     * Creates a new {@code I2CDeviceConfig} with the specified hardware addressing information and
     * configuration parameters.
     * <p>
     * The controller number is set to {@code UNASSIGNED}.
     * The requested input and output buffer sizes are set to {@code UNASSIGNED}.
     * </p>
     *
     * @param controllerName
     *            the name of the bus the slave device is connected to (such as its <em>device file</em> name on UNIX systems).
     * @param address the address of the slave device on the bus (a positive or zero integer, strictly lesser than
     * 2<sup>{@code addressSize}</sup>).
     * @param addressSize the address size: {@link #ADDR_SIZE_7} bits, {@link #ADDR_SIZE_10} bits.
     * @param clockFrequency
     *            the clock frequency of the slave device in Hz (a positive integer) or
     *            {@link #UNASSIGNED UNASSIGNED}.
     * @throws IllegalArgumentException
     *             if any of the following is true:
     *             <ul>
     *             <li>{@code address} is not in the defined range;</li>
     *             <li>{@code addressSize} is not in the defined range;</li>
     *             <li>{@code clockFrequency} is not in the defined range.</li>
     *             </ul>
     * @throws NullPointerException
     *             if {@code controllerName} is {@code null}.
     *
     * @deprecated As of 1.1, use {@link Builder} instead.
     */
    public I2CDeviceConfig(String controllerName, int address, int addressSize, int clockFrequency) {
        this(UNASSIGNED, address, addressSize, clockFrequency);
        // check for null
        controllerName.length();
        this.controllerName = controllerName;
    }
    
    /**
     * Gets the configured address of the I2C slave device.
     *
     * @return the address of the slave device on the bus (a positive or zero integer).
     */
    public int getAddress() {
        return address;
    }

    /**
     * Gets the configured address size of the I2C slave device.
     *
     * @return the address size: {@link #ADDR_SIZE_7} bits, {@link #ADDR_SIZE_10} bits or
     *         {@link #UNASSIGNED UNASSIGNED}.
     */
    public int getAddressSize() {
        return addressSize;
    }

    /**
     * Gets the configured controller number (the controller number the I2C bus adapter the I2C slave device
     * is connected to).
     *
     * @return the controller number (a positive or zero integer) or {@link #UNASSIGNED UNASSIGNED}.
     */
    public int getControllerNumber() {
        return controllerNumber;
    }

    /**
     * Gets the configured controller name (such as its <em>device file</em> name on UNIX systems).
     *
     * @return the controller name or {@code null}.
     */
    public String getControllerName() {
        return controllerName;
    }

    /**
     * Gets the requested input buffer size. The platform/underlying driver may or may not allocate the requested size
     * for the input buffer.
     * When querying the configuration of an opened or registered device the
     * allocated buffer size is returned.
     *
     * @return the requested or allocated input buffer size (a positive or zero integer) or {@link #UNASSIGNED UNASSIGNED}.
     *
     * @since 1.1
     */
    public int getInputBufferSize() {
        return inputBufferSize;
    }

    /**
     * Gets the requested or allocated output buffer size. The platform/underlying
     * driver may or may not allocate the requested size
     * for the output buffer.
     * When querying the configuration of an opened or registered device the
     * allocated buffer size is returned.
     *
     * @return the requested or allocated output buffer size (a positive or zero integer) or {@link #UNASSIGNED UNASSIGNED}.
     *
     * @since 1.1
     */
    public int getOutputBufferSize() {
        return outputBufferSize;
    }

    /**
     * Gets the configured clock frequency (in Hz) supported by the I2C slave device.
     *
     * @return the clock frequency of the slave device in Hz (a positive integer) or
     *         {@link #UNASSIGNED UNASSIGNED}.
     */
    public int getClockFrequency() {
        return clockFrequency;
    }

    private static void checkAddressSize(int address, int addressSize) {
        if (ADDR_SIZE_7 != addressSize && ADDR_SIZE_10 != addressSize) {
            throw new IllegalArgumentException(Integer.toString(addressSize));
        }

        int expAddressSize = 2;

        for (int i = 0; i < addressSize - 1; i++) {
            expAddressSize = expAddressSize * 2;
        }

        if (address >= expAddressSize) {
            throw new IllegalArgumentException(Integer.toString(address));
        }
    }

    private void checkValues() {
        Utils.checkIntValue(controllerNumber);
        Utils.checkIntZeroOrPozitive(address);
        Utils.checkGreaterThanZero(clockFrequency);
        checkAddressSize(address, addressSize);
    }

}
