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
package org.joshvm.j2me.sensor;

public class SensorDescriptor {
	public static final int DEFAULT_SENSOR_ID = 0;
	protected int SensorID;
	private String driverName;

	public SensorDescriptor(String deviceDriverClassName) {
		driverName = deviceDriverClassName;
		SensorID = DEFAULT_SENSOR_ID;
	}

	public SensorDescriptor(String deviceDriverClassName, int id) {
		driverName = deviceDriverClassName;
		SensorID = id;
	}

	public int getSensorID() {
		return SensorID;
	}

	public String getSensorDeviceDriverClassName() {
		return driverName;
	}

	public boolean isSameDevice(SensorDescriptor devDesc) {
		if (driverName.equals(devDesc.getSensorDeviceDriverClassName()) &&
			(SensorID == devDesc.SensorID)) {
			return true;
		} else {
			return false;
		}
	}

	public boolean equals(Object obj) {
		if (obj instanceof SensorDescriptor) {
			return isSameDevice((SensorDescriptor)obj);
		} else {
			return false;
		}
	}

}


