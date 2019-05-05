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

import java.io.IOException;
import java.util.Hashtable;

import org.joshvm.j2me.dio.ClosedDeviceException;
import org.joshvm.j2me.dio.UnavailableDeviceException;

public final class SensorHub {
	private static Hashtable registry;
	
	private static boolean isLoadedDriver(SensorDescriptor sensorDesc) {
		return registry.containsKey(sensorDesc);
	}

	private static void registerDriver(SensorDescriptor sensorDesc, SensorDevice sensorDriver) {
		registry.put(sensorDesc, sensorDriver);
	}

	private static Sensor getLoadedDriver(SensorDescriptor sensorDesc) {
		return (Sensor)registry.get(sensorDesc);
	}
	
 	public static Sensor getSensor(SensorDescriptor sensorDeviceDescriptor) throws IOException, UnavailableDeviceException, ClassNotFoundException {
		if (!isLoadedDriver(sensorDeviceDescriptor)) {
			try {
				String clsName = sensorDeviceDescriptor.getSensorDeviceDriverClassName();
				Class cls = Class.forName(clsName);
				SensorDevice sensorDriver = (SensorDevice)cls.newInstance();
				sensorDriver.startDevice(sensorDeviceDescriptor);
				registerDriver(sensorDeviceDescriptor, sensorDriver);
				return sensorDriver.getSensor(sensorDeviceDescriptor);
			} catch (InstantiationException ie) {
				throw new UnavailableDeviceException("Sensor driver class initantiation failed");
			} catch (IllegalAccessException iae) {
				throw new UnavailableDeviceException("Can not access to sensor driver constructor");
			}
		} else {
			return getLoadedDriver(sensorDeviceDescriptor);
		}		
	}

	private SensorHub() {}
}

