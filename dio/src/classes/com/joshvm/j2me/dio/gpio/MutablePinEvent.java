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
package com.joshvm.j2me.dio.gpio;

import org.joshvm.j2me.dio.gpio.PinEvent;
import org.joshvm.j2me.dio.gpio.GPIOPin;

class MutablePinEvent extends PinEvent {
	public MutablePinEvent() {}
	void setValue(boolean newValue) {
		value = newValue;
	}
	void setEvent(GPIOPin pin, boolean value, long timeStamp, int timeStampMicros) {
		this.device = pin;
		this.value = value;
		this.timeStamp = timeStamp;
        this.timeStampMicros = timeStampMicros;
		this.count = 1;
	}
}


