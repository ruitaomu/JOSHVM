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
package org.joshvm.j2me.dio;

public abstract class DeviceEvent {

    /**
     * The number of underlying coalesced hardware interrupts or software signals this event may
     * represent.
     */
    protected int count;
    /**
     * The time (in milliseconds) when the last coalesced event occurred. If events were not
     * coalesced then the time is the same as that of the first event.
     */
    protected long lastTimeStamp;
    /**
     * The additional microseconds (in the range {@code [0 - 999]}) to the timestamp for when the last coalesced event occurred. If
     * events were not coalesced then this is the same as that of the first event.
     * <p>
     * The actual last timestamp in microseconds is equal to: <i>(lastTimeStamp * 1000) +
     * lastTimeStampMicros</i>.
     * </p>
     */
    protected int lastTimeStampMicros;
    /**
     * The {@code Device} instance that fired this event or for which this event was fired.
     */
    protected Device device;
    /**
     * The time (in milliseconds) when this event (first) occurred. If events were coalesced then
     * the time is that of the first event.
     */
    protected long timeStamp;
    /**
     * The additional microseconds (in the range {@code [0 - 999]}) to the timestamp for when this event (first) occurred. If events
     * were coalesced then this is that of the first event.
     * <p>
     * The actual timestamp in microseconds is equal to: <i>(timeStamp * 1000) +
     * timeStampMicros</i>.
     * </p>
     */
    protected int timeStampMicros;

    /**
     * Returns the number of underlying coalesced hardware interrupts or software signals this event
     * may represent.
     *
     * @return the number of underlying coalesced hardware interrupts software signals this event
     *         may represent; {@code 1} if no coalescing occurred.
     */
    public final int getCount() {
        return count;
    }

    /**
     * Returns the time (in milliseconds) when the last coalesced event occurred. If events were not
     * coalesced then the time is the same as that of the first event.
     *
     * @return the time (in milliseconds) when this event last occurred.
     */
    public final long getLastTimeStamp() {
        return lastTimeStamp;
    }

    /**
     * Returns the additional microseconds to the timestamp for when the last coalesced event
     * occurred. If events were not coalesced then this is the same as that of the first event.
     *
     * @return the additional microseconds (in the range {@code [0 - 999]}) to the timestamp for when the last coalesced event
     *         occurred.
     */
    public final int getLastTimeStampMicros() {
        return lastTimeStampMicros;
    }

    /**
     * Returns the {@code Device} instance that fired this event or for which this event was
     * fired.
     *
     * @return the {@code Device} instance that fired this event.
     */
    public final Device getDevice() {
        return device;
    }

    /**
     * Returns the time (in milliseconds) when this event (first) occurred. If events were coalesced
     * then the time is that of the first event.
     *
     * @return the time (in milliseconds) when this event (first) occurred.
     */
    public final long getTimeStamp() {
        return timeStamp;
    }

    /**
     * Returns the additional microseconds to the timestamp for when this event (first) occurred. If
     * events were coalesced then the time is that of the first event.
     *
     * @return the additional microseconds (in the range {@code [0 - 999]}) to the timestamp for when this event (first) occurred.
     */
    public final int getTimeStampMicros() {
        return timeStampMicros;
    }
}

