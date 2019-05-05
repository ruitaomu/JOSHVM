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
package org.joshvm.j2me.dio.gpio;

import java.io.IOException;
import org.joshvm.j2me.dio.UnavailableDeviceException;
import org.joshvm.j2me.dio.ClosedDeviceException;

import org.joshvm.j2me.dio.Device;

public interface GPIOPin extends Device {

    /**
     * Input port direction.
     */
    static final int INPUT = 0;
    /**
     * Output port direction.
     */
    static final int OUTPUT = 1;

    /**
     * Returns the current direction of this GPIO pin. If the direction was not set previously using
     * {@link #setDirection setDirection} the device configuration-specific default value is
     * returned.
     *
     * @return {@link GPIOPin#OUTPUT} if this GPIO pin is currently set as output;
     *         {@link GPIOPin#INPUT} otherwise (the GPIO pin is set as input).
     * @throws IOException
     *             if some other I/O error occurs.
     * @throws UnavailableDeviceException
     *             if this device is not currently available - such as it is locked by another
     *             application.
     * @throws ClosedDeviceException
     *             if the device has been closed.
     */
    int getDirection() throws IOException, UnavailableDeviceException, ClosedDeviceException;

    /**
     * Returns the current pin interrupt trigger. If the trigger mode was not set previously using
     * {@link #setTrigger setTrigger} the device configuration-specific default value is
     * returned. Setting the trigger mode to {@link GPIOPinConfig#TRIGGER_NONE} disables
     * the notification of the {@link PinListener} instance (see {@link #setInputListener
     * setInputListener} without unregistering it. The value returned by this
     * method is unaffected by changes of the direction on a bidirectional GPIO pin.
     *
     * @return the current pin interrupt trigger, one of: {@link GPIOPinConfig#TRIGGER_NONE},
     *         {@link GPIOPinConfig#TRIGGER_FALLING_EDGE}, {@link GPIOPinConfig#TRIGGER_RISING_EDGE}
     *         , {@link GPIOPinConfig#TRIGGER_BOTH_EDGES}, {@link GPIOPinConfig#TRIGGER_HIGH_LEVEL},
     *         {@link GPIOPinConfig#TRIGGER_LOW_LEVEL}, {@link GPIOPinConfig#TRIGGER_BOTH_LEVELS};
     *         {@code GPIOPinConfig.TRIGGER_NONE} if this GPIO pin is configured for output-only.
     * @throws IOException
     *             if some other I/O error occurs.
     * @throws UnavailableDeviceException
     *             if this device is not currently available - such as it is locked by another
     *             application.
     * @throws ClosedDeviceException
     *             if the device has been closed.
     */
    int getTrigger() throws IOException, UnavailableDeviceException, ClosedDeviceException;

    /**
     * Returns the current value of this GPIO pin. If the value was not set previously using
     * {@link #setValue setValue} the device configuration-specific default value is returned.
     * <p>
     * This method can be called on both output and input pins.
     * </p>
     *
     * @return true if this pin is currently <em>high</em>.
     * @throws IOException
     *             if an I/O error occurred such as the pin is not readable.
     * @throws UnavailableDeviceException
     *             if this device is not currently available - such as it is locked by another
     *             application.
     * @throws ClosedDeviceException
     *             if the device has been closed.
     */
    boolean getValue() throws IOException, UnavailableDeviceException, ClosedDeviceException;

    /**
     * Sets this GPIO pin for output or input. The direction can only be set to {@link GPIOPin#INPUT}
     * if the GPIO pin is configured for input-only (see {@link GPIOPinConfig#DIR_INPUT_ONLY})
     * or is configured as bidirectional (see {@link GPIOPinConfig#DIR_BOTH_INIT_INPUT} and {@link GPIOPinConfig#DIR_BOTH_INIT_OUTPUT}).
     * The direction can only be set to {@link GPIOPin#OUTPUT}
     * if the GPIO pin is configured for output-only (see {@link GPIOPinConfig#DIR_OUTPUT_ONLY})
     * or is configured as bidirectional (see {@link GPIOPinConfig#DIR_BOTH_INIT_INPUT} and {@link GPIOPinConfig#DIR_BOTH_INIT_OUTPUT}).
     * The value returned by {@link #getTrigger() getTrigger} is unaffected by changes of the direction on a bidirectional GPIO pin.
     *
     * @param direction
     *            {@link GPIOPin#OUTPUT} for output; {@link GPIOPin#INPUT} for input.
     * @throws IOException
     *             if some other I/O error occurs.
     * @throws UnsupportedOperationException
     *             if this GPIO pin cannot be configured for the desired direction.
     * @throws IllegalArgumentException
     *             if {@code direction} is not equal to {@link GPIOPin#OUTPUT} or
     *             {@link GPIOPin#INPUT}.
     * @throws UnavailableDeviceException
     *             if this device is not currently available - such as it is locked by another
     *             application.
     * @throws ClosedDeviceException
     *             if the device has been closed.
     */
    void setDirection(int direction) throws IOException, UnsupportedOperationException, IllegalArgumentException, UnavailableDeviceException, ClosedDeviceException;

    /**
     * Sets this GPIO pin trigger mode. Setting the trigger mode to {@link GPIOPinConfig#TRIGGER_NONE} disables
     * the notification of the {@link PinListener} instance (see {@link #setInputListener
     * setInputListener} without unregistering it.
     *
     * @param trigger
     *            the interrupt trigger events, one of: {@link GPIOPinConfig#TRIGGER_NONE},
     *            {@link GPIOPinConfig#TRIGGER_FALLING_EDGE},
     *            {@link GPIOPinConfig#TRIGGER_RISING_EDGE},
     *            {@link GPIOPinConfig#TRIGGER_BOTH_EDGES}, {@link GPIOPinConfig#TRIGGER_HIGH_LEVEL}
     *            , {@link GPIOPinConfig#TRIGGER_LOW_LEVEL},
     *            {@link GPIOPinConfig#TRIGGER_BOTH_LEVELS}.
     * @throws IOException
     *             if an IO error occurred.
     * @throws UnsupportedOperationException
     *             if this GPIO pin is currently configured for output or cannot otherwise
     *             be configured with the desired trigger mode.
     * @throws IllegalArgumentException
     *             if {@code trigger} is not one of the defined values.
     * @throws UnavailableDeviceException
     *             if this device is not currently available - such as it is locked by another
     *             application.
     * @throws ClosedDeviceException
     *             if the device has been closed.
     */
    void setTrigger(int trigger) throws IOException, UnsupportedOperationException, IllegalArgumentException, UnavailableDeviceException, ClosedDeviceException;

    /**
     * Registers a {@link PinListener} instance which will get asynchronously notified when this
     * GPIO pin's value changes and according to the current trigger mode (see {@link #getTrigger
     * getTrigger}). Notification will automatically begin after registration completes.
     * <p>
     * If this {@code GPIOPin} is open in {@link org.joshvm.j2me.dio.DeviceManager#SHARED} access mode and if this
     * {@code GPIOPin} is currently configured for input, the listeners registered by all the
     * applications sharing the underlying GPIO pin will get notified when its value changes.
     * </p><p>
     * A listener can only be registered for a GPIO pin currently configured for input.
     * </p><p>
     * If {@code listener} is {@code null} then the previously registered listener is removed.
     * </p><p>
     * Only one listener can be registered at a particular time.
     * </p>
     *
     * @param listener
     *            the {@link PinListener} instance to be notified when this GPIO pin's value
     *            changes.
     * @throws IOException
     *             if some other I/O error occurs.
     * @throws UnsupportedOperationException
     *             if this GPIO pin is currently configured for output.
     * @throws IllegalStateException
     *             if {@code listener} is not {@code null} and a listener is already registered.
     * @throws ClosedDeviceException
     *             if the device has been closed.
     */
    void setInputListener(PinListener listener) throws IOException, UnsupportedOperationException, IllegalStateException, ClosedDeviceException;

    /**
     * Sets the value of this GPIO pin.
     * <p>
     * An attempt to set the value on a GPIO pin currently configured for input will result in a
     * {@link UnsupportedOperationException} being thrown.
     * </p>
     *
     * @param value
     *            the new pin value: {@code true} for <em>high</em>, {@code false} for <em>low</em>.
     * @throws UnsupportedOperationException
     *             if trying to set the value for pin configured for input.
     * @throws IOException
     *             if an I/O error occurred such as the pin is not writable.
     * @throws UnavailableDeviceException
     *             if this device is not currently available - such as it is locked by another
     *             application.
     * @throws ClosedDeviceException
     *             if the device has been closed.
     */
    void setValue(boolean value) throws IOException, UnsupportedOperationException, UnavailableDeviceException, ClosedDeviceException;
}

