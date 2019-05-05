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

import java.util.Hashtable;

class PinListenerThread implements Runnable {
	private Hashtable pin_table;
	private boolean newValue;
	private int ref_count;
	private boolean stop;
	
	public PinListenerThread(Hashtable pin_table) {
		this.pin_table = pin_table;
		newValue = false;
		ref_count = 0;
		stop = false;
	}

	public synchronized void ref() {
		ref_count++;
		if (ref_count == 1) {
			stop = false;
			new Thread(this).start();
		}
	}

	public synchronized void deRef() {
		if (ref_count <= 0) {
			return;
		}
		
		ref_count--;
		if (ref_count == 0) {
			stop = true;
		}
	}

	public void run() {
		IntegerKey key = new IntegerKey();
		MutablePinEvent event = new MutablePinEvent();
		GPIOPinDevice pin;
		int handle;
		while (!stop) {
			handle = waitingForGPIOPinEvent();
			if (handle != -1) {
				key.value = handle;
				pin = (GPIOPinDevice)pin_table.get(key);
				if ((pin != null) && pin.isOpen()) {
					event.setEvent(pin, newValue, System.currentTimeMillis(), 0);
					try {
						pin.invokeListener(event);
					} catch (Exception e) {
						e.printStackTrace();
					}
				}
			}
		}
	}

	private native int waitingForGPIOPinEvent();
}

