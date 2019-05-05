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
package org.joshvm.j2me.dio.spibus;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import com.joshvm.j2me.dio.utils.Utils;

import org.joshvm.j2me.dio.DeviceConfig;
import org.joshvm.j2me.dio.InvalidDeviceConfigException;

/**
 * The {@code SPIDeviceConfig} class encapsulates the hardware addressing information, and static and dynamic
 * configuration parameters of an SPI slave device.
 * <p>
 * Some hardware addressing, static or dynamic configuration parameters may be
 * set to {@link #UNASSIGNED UNASSIGNED} or {@code null} (see
 * <a href="{@docRoot}/jdk/dio/DeviceConfig.html#default_unassigned">Unassigned, Default or Unused Parameter Values</a>).
 * </p>
 * <h3><a name="mode">SPI Clock Modes</a></h3>
 * The clock mode is a number from 0 to 3 which represents the combination of the CPOL (SPI Clock Polarity Bit) and CPHA
 * (SPI Clock Phase Bit) signals where CPOL is the high order bit and CPHA is the low order bit:
 * <blockquote><table style="border:1px solid black;border-collapse:collapse;" summary="The clock mode is a number which represents the combination of the CPOL (SPI Clock Polarity Bit) and CPHA (SPI Clock Phase Bit) signals.">
 * <tr style="border:1px solid black;">
 * <th id="t1" style="border:1px solid black;">Mode</th>
 * <th id="t2" style="border:1px solid black;">CPOL</th>
 * <th id="t3" style="border:1px solid black;">CPHA</th>
 * </tr>
 * <tr style="border:1px solid black;">
 * <td headers="t1" style="border:1px solid black;">0</td>
 * <td headers="t2" style="border:1px solid black;">0 = Active-high clocks selected.</td>
 * <td headers="t3" style="border:1px solid black;">0 = Sampling of data occurs at odd edges of the SCK clock</td>
 * </tr>
 * <tr style="border:1px solid black;">
 * <td headers="t1" style="border:1px solid black;">1</td>
 * <td headers="t2" style="border:1px solid black;">0 = Active-high clocks selected.</td>
 * <td headers="t3" style="border:1px solid black;">1 = Sampling of data occurs at even edges of the SCK clock</td>
 * </tr>
 * <tr style="border:1px solid black;">
 * <td headers="t1" style="border:1px solid black;">2</td>
 * <td headers="t2" style="border:1px solid black;">1 = Active-low clocks selected.</td>
 * <td headers="t3" style="border:1px solid black;">0 = Sampling of data occurs at odd edges of the SCK clock</td>
 * </tr>
 * <tr style="border:1px solid black;">
 * <td headers="t1" style="border:1px solid black;">3</td>
 * <td headers="t2" style="border:1px solid black;">1 = Active-low clocks selected.</td>
 * <td headers="t3" style="border:1px solid black;">1 = Sampling of data occurs at even edges of the SCK clock</td>
 * </tr>
 * </table></blockquote>
 * <p>
 * An instance of {@code SPIDeviceConfig} can be passed to the {@link org.joshvm.j2me.dio.DeviceManager#open(DeviceConfig) open(DeviceConfig, ...)}
 * methods of the {@link org.joshvm.j2me.dio.DeviceManager} to open the designated SPI slave device with the
 * specified configuration. A {@link InvalidDeviceConfigException} is thrown when attempting to open a
 * device with an invalid or unsupported configuration.
 * </p>
 *
 * @see org.joshvm.j2me.dio.DeviceManager#open(DeviceConfig)
 * @see org.joshvm.j2me.dio.DeviceManager#open(DeviceConfig, int)
 * @since 1.0
 */
public final class SPIDeviceConfig implements DeviceConfig, DeviceConfig.HardwareAddressing {

    /**
     * High Chip Select active level.
     */
    public static final int CS_ACTIVE_HIGH = 0;

    /**
     * Low Chip Select active level.
     */
    public static final int CS_ACTIVE_LOW = 1;

    /**
     * Chip Select not controlled by driver. If this mode is configured, the Chip Select signal
     * is assumed to be controlled independently such as with an independent {@link org.joshvm.j2me.dio.gpio.GPIOPin GPIOPin}.
     */
    public static final int CS_NOT_CONTROLLED = 2;

    private String controllerName;
    private int address = UNASSIGNED;
    private int csActive = UNASSIGNED;
    private int controllerNumber = UNASSIGNED;
    private int bitOrdering = UNASSIGNED;
    private int clockFrequency = UNASSIGNED;
    private int clockMode;
    private int wordLength = UNASSIGNED;
    private int inputBufferSize = UNASSIGNED;
    private int outputBufferSize = UNASSIGNED;

    /**
     * The {@code Builder} class allows for creating and initializing
     * {@code SPIDeviceConfig} objects. Calls can be chained in the following
     * manner:
     * <blockquote>
     * <pre>
     *   SPIDeviceConfig config = new SPIDeviceConfig.Builder()
     *           .setControllerNumber(1)
     *           .setAddress(0x8)
     *           .setClockFrequency(8000000)
     *           .setClockMode(2)
     *           .setWordLength(10)
     *           .setBitOrdering(LITTLE_ENDIAN)
     *           .build();
     * </pre>
     * </blockquote>
     *
     * @since 1.1
     */
    public static final  class Builder {

        private final SPIDeviceConfig instance = new SPIDeviceConfig();

        /**
         * Creates a new {@code Builder} instance.
         */
        public Builder() {

        }

        /**
         * Creates a new {@code SPIDeviceConfig} instance initialized with the
         * values set for each configuration parameters. If a configuration
         * parameter was not explictly set its default value will be used.
         *
         * @return a new initialized {@code SPIDeviceConfig} instance.
         * @throws IllegalStateException if any of the following is true:
         * <ul>
         * <li>the Chip Select address of the slave device on the bus is not set.</li>
         * </ul>
         */
        public SPIDeviceConfig build() {
            if (UNASSIGNED == instance.address) {
                throw new IllegalStateException();
            }

			SPIDeviceConfig newinstance = new SPIDeviceConfig();
			newinstance.controllerName = instance.controllerName;
			newinstance.address = instance.address;
			newinstance.csActive = instance. csActive;
			newinstance.controllerNumber = instance.controllerNumber;
			newinstance.bitOrdering = instance.bitOrdering;
			newinstance.clockFrequency = instance.clockFrequency;
			newinstance.clockMode = instance.clockMode;
			newinstance.wordLength = instance.wordLength;
			newinstance.inputBufferSize = instance.inputBufferSize;
			newinstance.outputBufferSize = instance.outputBufferSize;
            return newinstance;
        }

        /**
         * Sets the name of the bus the slave device is connected to (default
         * value is {@code null} if not set).
         *
         * @param controllerName the name of the bus the slave device is
         * connected to (such as its <em>device file</em> name on UNIX systems) or {@code null}.
         * @return this {@code Builder} instance.
         */
        public Builder setControllerName(String controllerName) {
            instance.controllerName = controllerName;
            return this;
        }

        /**
         * Sets the Chip Select address of the slave device on the bus.
         *
         * @param address the Chip Select address of the slave device on the bus
         * (a positive or zero integer).
         * @return this {@code Builder} instance.
         * @throws IllegalArgumentException if any of the following is true:
         * <ul>
         * <li>{@code address} is not in the defined range;</li>
         * </ul>
         */
        public Builder setAddress(int address) {
            Utils.checkIntZeroOrPozitive(address);
            instance.address = address;
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
         * {@code UNASSIGNED}.
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
         * {@code UNASSIGNED}.
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
         * Sets the Chip Select active level (default value is
         * {@code UNASSIGNED} if not set).
         *
         * @param csActive the Chip Select active level, one of {@link #CS_ACTIVE_LOW},
         *            {@link #CS_ACTIVE_HIGH}, {@link #CS_NOT_CONTROLLED} or
         * {@link #UNASSIGNED UNASSIGNED}.
         * @return this {@code Builder} instance.
         * @throws IllegalArgumentException if {@code csActive} is not in the
         * defined range.
         */
        public Builder setCSActiveLevel(int csActive) {
            checkCsActive(csActive);
            instance.csActive = csActive;
            return this;
        }

        /**
         * Sets the bit (shifting) ordering of the slave device (default value
         * is {@code UNASSIGNED} if not set).
         *
         * @param bitOrdering the bit (shifting) ordering of the slave device,
         * one of: {@link SPIDevice#BIG_ENDIAN},
         *            {@link SPIDevice#LITTLE_ENDIAN}, {@link SPIDevice#MIXED_ENDIAN} or
         * {@link #UNASSIGNED UNASSIGNED}.
         * @return this {@code Builder} instance.
         * @throws IllegalArgumentException if {@code bitOrdering} is not in the
         * defined range.
         */
        public Builder setBitOrdering(int bitOrdering) {
            checkBitOrdering(bitOrdering);
            instance.bitOrdering = bitOrdering;
            return this;
        }

        /**
         * Sets the clock mode (default value is {@code 0} if not set).
         *
         * @param clockMode the clock mode, one of:
         * {@code 0}, {@code 1}, {@code 2} or {@code 3} (see  <a href="{@docRoot}/jdk/dio/spibus/SPIDeviceConfig.html#mode">SPI
         * Clock Modes</a>).
         * @return this {@code Builder} instance.
         * @throws IllegalArgumentException if {@code clockMode} is not in the
         * defined range.
         */
        public Builder setClockMode(int clockMode) {
            checkClockMode(clockMode);
            instance.clockMode = clockMode;
            return this;
        }

        /**
         * Sets the word length of the slave device (default value is
         * {@code UNASSIGNED} if not set).
         *
         * @param wordLength the word length of the slave device (a positive
         * integer) or {@link #UNASSIGNED UNASSIGNED}.
         * @return this {@code Builder} instance.
         * @throws IllegalArgumentException if {@code wordLength} is not in the
         * defined range.
         */
        public Builder setWordLength(int wordLength) {
            Utils.checkGreaterThanZero(wordLength);
            instance.wordLength = wordLength;
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
    SPIDeviceConfig(){}

    /**
     * Creates a new {@code SPIDeviceConfig} with the specified hardware addressing information and configuration
     * parameters. The Chip Select active level is platform and/or driver-dependent (i.e. {@link #UNASSIGNED UNASSIGNED}).
     * <p>
     * The controller name is set to {@code null}.
     * The requested input and output buffer sizes are set to {@code UNASSIGNED}.
     * </p>
     *
     * @param controllerNumber
     *            the number of the bus the slave device is connected to (a positive or zero integer) or
     *            {@link #UNASSIGNED UNASSIGNED}.
     * @param address
     *            the Chip Select address of the slave device on the bus (a positive or zero integer).
     * @param clockFrequency
     *            the clock frequency of the slave device in Hz (a positive integer) or {@link #UNASSIGNED UNASSIGNED}.
     * @param clockMode
     *            the clock mode, one of: {@code 0}, {@code 1}, {@code 2} or {@code 3} (see <a href="#mode">SPI Clock
     *            Modes</a>).
     * @param wordLength
     *            the word length of the slave device (a positive integer) or {@link #UNASSIGNED UNASSIGNED}.
     * @param bitOrdering
     *            the bit (shifting) ordering of the slave device, one of: {@link SPIDevice#BIG_ENDIAN},
     *            {@link SPIDevice#LITTLE_ENDIAN}, {@link SPIDevice#MIXED_ENDIAN} or {@link #UNASSIGNED UNASSIGNED}.
     * @throws IllegalArgumentException
     *             if any of the following is true:
     *             <ul>
     *             <li>{@code controllerNumber} is not in the defined range;</li>
     *             <li>{@code address} is not in the defined range;</li>
     *             <li>{@code clockFrequency} is not in the defined range;</li>
     *             <li>{@code clockMode} is not one of the defined values;</li>
     *             <li>{@code wordLength} is not in the defined range;</li>
     *             <li>{@code bitOrdering} is not one of the defined values.</li>
     *             </ul>
     *
     * @deprecated As of 1.1, use {@link Builder} instead.
     */
    public SPIDeviceConfig(int controllerNumber, int address, int clockFrequency, int clockMode, int wordLength,
            int bitOrdering) {
        this(controllerNumber, address, UNASSIGNED, clockFrequency, clockMode, wordLength, bitOrdering);
    }

    /**
     * Creates a new {@code SPIDeviceConfig} with the specified hardware addressing information and configuration
     * parameters.
     * <p>
     * The controller name is set to {@code null}.
     * The requested input and output buffer sizes are set to {@code UNASSIGNED}.
     * </p>
     *
     * @param controllerNumber
     *            the number of the bus the slave device is connected to (a positive or zero integer) or
     *            {@link #UNASSIGNED UNASSIGNED}.
     * @param address
     *            the Chip Select address of the slave device on the bus (a positive or zero integer).
     * @param csActive
     *            the Chip Select active level, one of {@link #CS_ACTIVE_LOW},
     *            {@link #CS_ACTIVE_HIGH}, {@link #CS_NOT_CONTROLLED} or {@link #UNASSIGNED UNASSIGNED}.
     * @param clockFrequency
     *            the clock frequency of the slave device in Hz (a positive integer) or {@link #UNASSIGNED UNASSIGNED}.
     * @param clockMode
     *            the clock mode, one of: {@code 0}, {@code 1}, {@code 2} or {@code 3} (see <a href="#mode">SPI Clock
     *            Modes</a>).
     * @param wordLength
     *            the word length of the slave device (a positive integer) or {@link #UNASSIGNED UNASSIGNED}.
     * @param bitOrdering
     *            the bit (shifting) ordering of the slave device, one of: {@link SPIDevice#BIG_ENDIAN},
     *            {@link SPIDevice#LITTLE_ENDIAN}, {@link SPIDevice#MIXED_ENDIAN} or {@link #UNASSIGNED UNASSIGNED}.
     * @throws IllegalArgumentException
     *             if any of the following is true:
     *             <ul>
     *             <li>{@code controllerNumber} is not in the defined range;</li>
     *             <li>{@code address} is not in the defined range;</li>
     *             <li>{@code clockFrequency} is not in the defined range;</li>
     *             <li>{@code clockMode} is not one of the defined values;</li>
     *             <li>{@code wordLength} is not in the defined range;</li>
     *             <li>{@code bitOrdering} is not one of the defined values.</li>
     *             </ul>
     *
     * @deprecated As of 1.1, use {@link Builder} instead.
     */
    public SPIDeviceConfig(int controllerNumber, int address, int csActive, int clockFrequency, int clockMode, int wordLength,
            int bitOrdering) {
        this.controllerNumber = controllerNumber;
        this.address = address;
        this.csActive = csActive;
        this.clockFrequency = clockFrequency;
        this.clockMode = clockMode;
        this.wordLength = wordLength;
        this.bitOrdering = bitOrdering;
        checkParameters();
    }

    /**
     * Creates a new {@code SPIDeviceConfig} with the specified hardware addressing information and configuration
     * parameters. The Chip Select active level is platform and/or driver-dependent (i.e. {@link #UNASSIGNED UNASSIGNED}).
     * <p>
     * The controller number is set to {@code UNASSIGNED}.
     * The requested input and output buffer sizes are set to {@code UNASSIGNED}.
     * </p>
     *
     * @param controllerName
     *            the controller name (such as its <em>device file</em> name on UNIX systems).
     * @param address
     *            the Chip Select address of the slave device on the bus (a positive or zero integer).
     * @param clockFrequency
     *            the clock frequency of the slave device in Hz (a positive integer) or {@link #UNASSIGNED UNASSIGNED}.
     * @param clockMode
     *            the clock mode, one of: {@code 0}, {@code 1}, {@code 2} or {@code 3} (see <a href="#mode">SPI Clock
     *            Modes</a>).
     * @param wordLength
     *            the word length of the slave device (a positive integer) or {@link #UNASSIGNED UNASSIGNED}.
     * @param bitOrdering
     *            the bit (shifting) ordering of the slave device, one of: {@link SPIDevice#BIG_ENDIAN},
     *            {@link SPIDevice#LITTLE_ENDIAN}, {@link SPIDevice#MIXED_ENDIAN} or {@link #UNASSIGNED UNASSIGNED}.
     * @throws IllegalArgumentException
     *             if any of the following is true:
     *             <ul>
     *             <li>{@code address} is not in the defined range;</li>
     *             <li>{@code clockFrequency} is not in the defined range;</li>
     *             <li>{@code clockMode} is not one of the defined values;</li>
     *             <li>{@code wordLength} is not in the defined range;</li>
     *             <li>{@code bitOrdering} is not one of the defined values.</li>
     *             </ul>
     * @throws NullPointerException
     *             if {@code controllerName} is {@code null}.
     *
     * @deprecated As of 1.1, use {@link Builder} instead.
     */
    public SPIDeviceConfig(String controllerName, int address, int clockFrequency, int clockMode, int wordLength,
            int bitOrdering) {
        this(controllerName, address, UNASSIGNED, clockFrequency, clockMode, wordLength, bitOrdering);
    }

    /**
     * Creates a new {@code SPIDeviceConfig} with the specified hardware addressing information and configuration
     * parameters.
     * <p>
     * The controller number is set to {@code UNASSIGNED}.
     * The requested input and output buffer sizes are set to {@code UNASSIGNED}.
     * </p>
     *
     * @param controllerName
     *            the controller name (such as its <em>device file</em> name on UNIX systems).
     * @param address
     *            the Chip Select address of the slave device on the bus (a positive or zero integer).
     * @param csActive
     *            the Chip Select active level, one of {@link #CS_ACTIVE_LOW},
     *            {@link #CS_ACTIVE_HIGH}, {@link #CS_NOT_CONTROLLED} or {@link #UNASSIGNED UNASSIGNED}.
     * @param clockFrequency
     *            the clock frequency of the slave device in Hz (a positive integer) or {@link #UNASSIGNED UNASSIGNED}.
     * @param clockMode
     *            the clock mode, one of: {@code 0}, {@code 1}, {@code 2} or {@code 3} (see <a href="#mode">SPI Clock
     *            Modes</a>).
     * @param wordLength
     *            the word length of the slave device (a positive integer) or {@link #UNASSIGNED UNASSIGNED}.
     * @param bitOrdering
     *            the bit (shifting) ordering of the slave device, one of: {@link SPIDevice#BIG_ENDIAN},
     *            {@link SPIDevice#LITTLE_ENDIAN}, {@link SPIDevice#MIXED_ENDIAN} or {@link #UNASSIGNED UNASSIGNED}.
     * @throws IllegalArgumentException
     *             if any of the following is true:
     *             <ul>
     *             <li>{@code address} is not in the defined range;</li>
     *             <li>{@code clockFrequency} is not in the defined range;</li>
     *             <li>{@code clockMode} is not one of the defined values;</li>
     *             <li>{@code wordLength} is not in the defined range;</li>
     *             <li>{@code bitOrdering} is not one of the defined values.</li>
     *             </ul>
     * @throws NullPointerException
     *             if {@code controllerName} is {@code null}.
     *
     * @deprecated As of 1.1, use {@link Builder} instead.
     */
    public SPIDeviceConfig(String controllerName, int address, int csActive, int clockFrequency, int clockMode, int wordLength,
            int bitOrdering) {
        this(UNASSIGNED, address, csActive, clockFrequency, clockMode, wordLength, bitOrdering);
        // checks for null
        controllerName.length();
        this.controllerName  = controllerName;
    }

    /**
     * Gets the configured address of the SPI slave device.
     *
     * @return the Chip Select address of the slave device on the bus (a positive or zero integer).
     */
    public int getAddress() {
        return address;
    }

    /**
     * Gets the configured controller number (the controller number of the SPI bus adapter the slave device is connected to).
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
     * Gets the requested or allocated input buffer size. The platform/underlying
     * driver may or may not allocate the requested size
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
     * Gets the configured bit (shifting) ordering of the SPI slave device.
     *
     * @return the bit ordering of the slave device, one of: {@link SPIDevice#BIG_ENDIAN},
     *         {@link SPIDevice#LITTLE_ENDIAN}, {@link SPIDevice#MIXED_ENDIAN} or {@link #UNASSIGNED UNASSIGNED}.
     */
    public int getBitOrdering() {
        return bitOrdering;
    }

    /**
     * Gets the clock frequency (in Hz) supported by the SPI slave device.
     *
     * @return the clock frequency of the slave device in Hz (a positive integer) or {@link #UNASSIGNED UNASSIGNED}.
     */
    public int getClockFrequency() {
        return clockFrequency;
    }

    /**
     * Gets the configured clock mode (combining clock polarity and phase) for communicating with the SPI slave device.
     *
     * @return the clock mode, one of: {@code 0}, {@code 1}, {@code 2} or {@code 3} (see <a href="#mode">SPI Clock
     *         Modes</a>).
     */
    public int getClockMode() {
        return clockMode;
    }

    /**
     * Gets the configured Chip Select active level for selecting the SPI slave device.
     *
     * @return the Chip Select active level,one of {@link #CS_ACTIVE_LOW},
     *            {@link #CS_ACTIVE_HIGH}, {@link #CS_NOT_CONTROLLED} or {@link #UNASSIGNED UNASSIGNED}.
     */
    public int getCSActiveLevel() {
        return csActive;
    }

    /**
     * Gets the configured word length for communicating with the SPI slave device.
     *
     * @return the word length of the slave device (a positive integer) or {@link #UNASSIGNED UNASSIGNED}.
     */
    public int getWordLength() {
        return wordLength;
    }


    private static void checkCsActive(int csActive) {
        if (csActive < UNASSIGNED || csActive > CS_NOT_CONTROLLED) {
            throw new IllegalArgumentException(Integer.toString(csActive));
        }
    }

    private static void checkClockMode(int clockMode) {
        if (clockMode < 0 || clockMode > 3) {
            throw new IllegalArgumentException(Integer.toString(clockMode));
        }
    }

    private static void checkBitOrdering(int bitOrdering) {
        if (bitOrdering < UNASSIGNED || bitOrdering > SPIDevice.MIXED_ENDIAN) {
            throw new IllegalArgumentException(Integer.toString(bitOrdering));
        }
    }

    private void checkParameters(){
        Utils.checkIntValue(controllerNumber);
        Utils.checkIntZeroOrPozitive(address);
        Utils.checkGreaterThanZero(clockFrequency);
        checkCsActive(csActive);
        checkClockMode(clockMode);
        checkBitOrdering(bitOrdering);
        Utils.checkGreaterThanZero(wordLength);
    }
}
