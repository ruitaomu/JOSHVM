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

import org.joshvm.j2me.dio.DeviceEvent;

public class PinEvent extends DeviceEvent {

	protected PinEvent() {}

    /**
     * The new GPIO pin's value.
     */
    protected boolean value;

    /**
     * Creates a new {@link PinEvent} with the specified value and time-stamped with the current
     * time.
     *
     * @param pin
     *            the source GPIO pin.
     * @param value
     *            the new value.
     * @throws NullPointerException
     *             if {@code pin} is {@code null}.
     */
    public PinEvent(GPIOPin pin, boolean value) {
        this(pin, value, System.currentTimeMillis(), 0);
    }

    /**
     * Creates a new {@link PinEvent} with the specified value and timestamp.
     *
     * @param pin
     *            the source GPIO pin.
     * @param value
     *            the new value.
     * @param timeStamp
     *            the timestamp (in milliseconds).
     * @param timeStampMicros
     *            the additional microseconds to the timestamp.
     * @throws NullPointerException
     *             if {@code pin} is {@code null}.
     * @throws IllegalArgumentException
     *             if {@code timeStamp} is negative or
     *             {@code timeStampMicros} is not in the range {@code [0 - 999]}.
     */
    public PinEvent(GPIOPin pin, boolean value, long timeStamp, int timeStampMicros) {
        // checks for null
        super();
        pin.isOpen();
        if (0 > timeStamp || 0 > timeStampMicros || timeStampMicros > 999) {
            throw new IllegalArgumentException();
        }
        this.device = pin;
        this.value = value;
        this.timeStamp = timeStamp;
        this.timeStampMicros = timeStampMicros;
        count = 1;
    }

    /**
     * Returns the new GPIO pin's value.
     *
     * @return the new GPIO pin's value.
     */
    public boolean getValue() {
        return value;
    }
}

