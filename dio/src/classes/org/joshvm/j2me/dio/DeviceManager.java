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

import java.io.IOException;
import com.joshvm.j2me.dio.gpio.GPIOPinDevice;
import org.joshvm.j2me.dio.gpio.GPIOPin;
import org.joshvm.j2me.dio.gpio.GPIOPinConfig;
import org.joshvm.j2me.dio.spibus.SPIDevice;
import org.joshvm.j2me.dio.spibus.SPIDeviceConfig;
import com.joshvm.j2me.dio.spibus.SPISlaveImpl;
import org.joshvm.j2me.dio.i2cbus.I2CDevice;
import org.joshvm.j2me.dio.i2cbus.I2CDeviceConfig;
import com.joshvm.j2me.dio.i2cbus.I2CSlaveImpl;

public class DeviceManager {
	public static final int	EXCLUSIVE = 1;
	public static final int	SHARED = 2;
	public static final int	UNSPECIFIED_ID = -1;

	private DeviceManager() {}

	public static Device open(DeviceConfig config, int mode) throws java.io.IOException,
                                    InvalidDeviceConfigException,
                                    UnsupportedDeviceTypeException,
                                    DeviceNotFoundException,
                                    UnavailableDeviceException,
                                    UnsupportedAccessModeException {
        if (null == config) {
            throw new NullPointerException("DEVICE_NULL_CONFIG_OR_INTF");
        }

		if (config instanceof GPIOPinConfig) {			
			return openGPIOPinDevice((GPIOPinConfig)config, mode);
		} else if (config instanceof SPIDeviceConfig) {
			return openSPIDevice((SPIDeviceConfig)config, mode);
		} else if (config instanceof I2CDeviceConfig) {
			return openI2CDevice((I2CDeviceConfig)config, mode);
		} else {
			throw new UnsupportedDeviceTypeException("Not supported device type");
		}
	}

	public static Device open(DeviceConfig config) throws java.io.IOException,
                                    InvalidDeviceConfigException,
                                    UnsupportedDeviceTypeException,
                                    DeviceNotFoundException,
                                    UnavailableDeviceException,
                                    UnsupportedAccessModeException {
		return open(config, EXCLUSIVE);
	}

	private static GPIOPin openGPIOPinDevice(GPIOPinConfig config, int mode) throws IOException,
                                                                   DeviceNotFoundException,
                                                                   UnavailableDeviceException,
                                                                   UnsupportedAccessModeException {
        if (mode != EXCLUSIVE) {
			throw new UnsupportedAccessModeException("Only EXCLUSIVE mode is supported for GPIOPin device");
		}
        GPIOPinDevice pin = new GPIOPinDevice(config);
		pin.open();
		return pin;
	}

	private static SPIDevice openSPIDevice(SPIDeviceConfig config, int mode) throws IOException,
                                                                   DeviceNotFoundException,
                                                                   UnavailableDeviceException,
                                                                   UnsupportedAccessModeException,
                                                                   InvalidDeviceConfigException {
        if (mode != EXCLUSIVE) {
			throw new UnsupportedAccessModeException("Only EXCLUSIVE mode is supported for SPI device");
		}
        SPISlaveImpl spi = new SPISlaveImpl(config, mode);
		return spi;
	}

	private static I2CDevice openI2CDevice(I2CDeviceConfig config, int mode) throws IOException,
                                                                   DeviceNotFoundException,
                                                                   UnavailableDeviceException,
                                                                   UnsupportedAccessModeException,
                                                                   InvalidDeviceConfigException {
        if (mode != EXCLUSIVE) {
			throw new UnsupportedAccessModeException("Only EXCLUSIVE mode is supported for I2C device");
		}
        I2CSlaveImpl i2c = new I2CSlaveImpl(config, mode);
		return i2c;
	}

}

