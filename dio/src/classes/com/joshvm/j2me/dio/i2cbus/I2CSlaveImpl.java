/*
 * Copyright (c) 2012, 2015, Oracle and/or its affiliates. All rights reserved.
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

package com.joshvm.j2me.dio.i2cbus;

import java.io.IOException;
import java.util.Enumeration;
import java.util.Vector;

import org.joshvm.j2me.dio.ClosedDeviceException;
import org.joshvm.j2me.dio.DeviceManager;
import org.joshvm.j2me.dio.DeviceNotFoundException;
import org.joshvm.j2me.dio.InvalidDeviceConfigException;
import org.joshvm.j2me.dio.UnavailableDeviceException;
import org.joshvm.j2me.dio.UnsupportedAccessModeException;
import org.joshvm.j2me.dio.UnsupportedDeviceTypeException;
import org.joshvm.j2me.dio.gpio.GPIOPin;
import org.joshvm.j2me.dio.gpio.GPIOPinConfig;
import org.joshvm.j2me.dio.i2cbus.I2CDevice;
import org.joshvm.j2me.dio.i2cbus.I2CDeviceConfig;

import org.joshvm.util.ByteBuffer;
import com.joshvm.j2me.dio.utils.ExceptionMessage;

public class I2CSlaveImpl implements I2CDevice {

    static final int I2C_REGULAR = 0;
    static final int I2C_COMBINED_START = 1;
    static final int I2C_COMBINED_END = 2;
    static final int I2C_COMBINED_BODY = 3;
    
    private boolean open = false;    
    private I2CDeviceConfig config;

    public I2CSlaveImpl(I2CDeviceConfig cfg, int mode) throws
            DeviceNotFoundException, InvalidDeviceConfigException {
    	
        if (cfg.getControllerName() != null) {
            throw new InvalidDeviceConfigException(
                    ExceptionMessage.format(ExceptionMessage.DEVICE_OPEN_WITH_DEVICENAME_UNSUPPORTED)
            );
        }

        try {
			open0(cfg, mode == DeviceManager.EXCLUSIVE);
			config = cfg;
			open = true;
		} catch (UnsupportedDeviceTypeException e) {
			throw new InvalidDeviceConfigException(e.getMessage());
		} catch (UnavailableDeviceException e) {
			throw new DeviceNotFoundException(e.getMessage());
		} catch (UnsupportedAccessModeException e) {
			throw new InvalidDeviceConfigException(e.getMessage());
		} catch (IOException e) {
			throw new DeviceNotFoundException(e.getMessage());
		}
    }

	public Bus getBus() throws IOException {
        return new Bus() {
            public org.joshvm.j2me.dio.i2cbus.I2CCombinedMessage createCombinedMessage() {
                return new I2CCombinedMessage();
            }
        };
    }
    
    private void doCheck(int skip, ByteBuffer buf) {
        if (buf == null)
            throw new NullPointerException();

        if (skip < 0)
            throw new IllegalArgumentException();
    }

    public ByteBuffer getInputBuffer() throws ClosedDeviceException,
            IOException {
        throw new UnsupportedOperationException();
    }

    public ByteBuffer getOutputBuffer() throws ClosedDeviceException,
            IOException {
        throw new UnsupportedOperationException();
    }

    public int read() throws IOException,
            UnavailableDeviceException, ClosedDeviceException {
        ByteBuffer dst = ByteBuffer.allocateDirect(1);
        read(dst);
        return dst.get(0);
    }

    public int read(ByteBuffer dst) throws IOException,
            UnavailableDeviceException, ClosedDeviceException {
        return read(0, dst);
    }

    public int read(int skip, ByteBuffer dst) throws IOException,
            UnavailableDeviceException, ClosedDeviceException {

        doCheck(skip, dst);

        return transfer(I2C_REGULAR, skip, dst);
    }

    public int read(int subaddress, int subaddressSize,
                    ByteBuffer dst) throws IOException, UnavailableDeviceException,
            ClosedDeviceException {
        return read(subaddress, subaddressSize, 0, dst);
    }

    public int read(int subaddress, int subaddressSize, int skip,
                    ByteBuffer dst) throws IOException, UnavailableDeviceException,
            ClosedDeviceException {

        if (subaddressSize <= 0 || subaddressSize > 4 || subaddress < 0)
            throw new IllegalArgumentException();

        doCheck(skip, dst);

        ByteBuffer tmp = ByteBuffer.wrap(new byte[4/*sizeof(int)*/]);
        //tmp.order(ByteOrder.BIG_ENDIAN);
        tmp.putInt(subaddress);
        tmp.position(4 - subaddressSize);
        I2CCombinedMessage msg = new I2CCombinedMessage();
        msg.appendWrite(this, tmp);
        msg.appendRead(this, skip, dst);
        return msg.transfer()[0];
    }

    public int write(ByteBuffer src) throws IOException,
            UnavailableDeviceException, ClosedDeviceException {
        doCheck(0, src);
        return transfer(I2C_REGULAR, -1, src);
    }


    public void write(int srcData) throws IOException,
            UnavailableDeviceException, ClosedDeviceException {
        ByteBuffer dst = ByteBuffer.allocateDirect(1);
        dst.put((byte) srcData);
        dst.flip();
        write(dst);
    }

    public int write(int subaddress, int subaddressSize,
                     ByteBuffer src) throws IOException, UnavailableDeviceException,
            ClosedDeviceException {

        if (subaddressSize <= 0 || subaddressSize > 4 || subaddress < 0) {
            throw new IllegalArgumentException();
        }

        doCheck(0, src);

        ByteBuffer tmp = ByteBuffer.allocateDirect(4/*size of int*/ + src.remaining());
        //tmp.order(ByteOrder.BIG_ENDIAN);
        tmp.putInt(subaddress);
        tmp.put(src);
        tmp.position(4 - subaddressSize);
        return write(tmp) - subaddressSize;
    }

    /**
     * @throws IllegalStateException if the bus is occupied for
     *                               communication with other peripheral or with other
     *                               transaction
     */
    synchronized int transfer(int flag, int skip, ByteBuffer appBuffer) throws
            UnavailableDeviceException, ClosedDeviceException, IOException {
        int ret = 0;
        ByteBuffer direct;

        if (!appBuffer.hasRemaining() && skip <= 0) {
            return 0;
        }
        
        if (skip > 0) {
        	direct = ByteBuffer.allocate(appBuffer.remaining());
        } else {
        	direct = appBuffer;
        }

        ret = transfer0(skip < 0, direct, flag);
        
        if (skip > 0) {
            ret = (ret > skip) ? ret - skip : 0;
            if (ret > 0) {
	            direct.position(skip);
	            appBuffer.put(direct);
            }
        }
        
        return ret;
    }

    private void open0(I2CDeviceConfig cfg, boolean b) throws InvalidDeviceConfigException, 
    														UnsupportedDeviceTypeException, 
    														DeviceNotFoundException, 
    														UnavailableDeviceException, 
    														UnsupportedAccessModeException, 
    														IOException {
   		int SDAgpio, SCLgpio;
		
    	String simu = System.getProperty("I2C.simu.enable");
		if ((simu == null) || (!simu.equals("true"))) {
			//Currently, only simulated I2C is supported
			throw new IOException("No I2C support");
		}

		try {
    		String propSDA = System.getProperty("I2C.simu.SDA");
			String propSCL = System.getProperty("I2C.simu.SCL");
			SDAgpio = Integer.parseInt(propSDA);
			SCLgpio = Integer.parseInt(propSCL);
		} catch (NumberFormatException e) {
			throw new IOException("SDA/SCL property not found");
		}
		
		I2COperation.init(SDAgpio, SCLgpio);	
	}
    
	private int transfer0(boolean write, ByteBuffer direct, int flag) throws UnavailableDeviceException, 
																			ClosedDeviceException, 
																			IOException {
		int ret = 0;
		
		I2COperation.startbit();
		I2COperation.sendSlaveAddress(config.getAddress(), config.getAddressSize(), write);

		while (direct.hasRemaining()) {
			if (write) {
				I2COperation.writebyte(direct.get());
				if (I2COperation.readAck()) {
					ret++;
				} else {				
					// NACK received, then stop
					break;
				}
			} else {
				direct.put(I2COperation.readbyte());
				ret++;
				if (direct.hasRemaining()) {
					I2COperation.sendAck();
				} else {
					I2COperation.sendNack();
				}
			}
		}
		if ((flag == I2C_REGULAR) || (flag == I2C_COMBINED_END)) {
			I2COperation.stopbit();
		}
		return ret;
	}
	
	static class I2COperation {
		private static GPIOPin SDA;
		private static GPIOPin SCL;
		
		static void init(int numSDA, int numSCL) throws InvalidDeviceConfigException, 
														UnsupportedDeviceTypeException, 
														DeviceNotFoundException, 
														UnavailableDeviceException, 
														UnsupportedAccessModeException, 
														IOException {
			GPIOPinConfig cfgSCL = new GPIOPinConfig(GPIOPinConfig.UNASSIGNED, numSCL, // SCL2
					GPIOPinConfig.DIR_BOTH_INIT_OUTPUT, GPIOPinConfig.MODE_OUTPUT_OPEN_DRAIN | GPIOPinConfig.MODE_INPUT_PULL_UP, GPIOPinConfig.TRIGGER_NONE,
					true);
			SCL = (GPIOPin) DeviceManager.open(cfgSCL, DeviceManager.EXCLUSIVE);
			GPIOPinConfig cfgSDA = new GPIOPinConfig(GPIOPinConfig.UNASSIGNED, numSDA, // SDA2
					GPIOPinConfig.DIR_BOTH_INIT_OUTPUT, GPIOPinConfig.MODE_OUTPUT_OPEN_DRAIN | GPIOPinConfig.MODE_INPUT_PULL_UP, GPIOPinConfig.TRIGGER_NONE,
					true);
			SDA = (GPIOPin) DeviceManager.open(cfgSDA, DeviceManager.EXCLUSIVE);

		}
		
		static void deinit() {
			try {
				SCL.close();
			} catch (IOException e) {
				e.printStackTrace();
			}
			try {
				SDA.close();
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
		
		static void startbit() throws IOException, UnavailableDeviceException, ClosedDeviceException {
			SDA.setValue(true);
			SCL.setValue(true);
			SDA.setValue(false);
			SCL.setValue(false);
			
			//System.out.println("[I2C]startbit");
		}

		static void stopbit() throws IOException, UnavailableDeviceException, ClosedDeviceException {
			SDA.setValue(false);
			SCL.setValue(true);
			SDA.setValue(true);
			//System.out.println("[I2C]stopbit");
		}
		
		static void sendNack() throws IOException, UnavailableDeviceException, ClosedDeviceException {
			writebit(true);
			//System.out.println("[I2C]writeNACK");
		}
		
		static void sendAck() throws IOException, UnavailableDeviceException, ClosedDeviceException {
			writebit(false);
			//System.out.println("[I2C]writeACK");
		}

		static boolean readAck() throws IOException, UnavailableDeviceException, ClosedDeviceException {
			boolean res = readbit();
			//System.out.println("[I2C]readACK "+res);
			if (res == true) {
				throw new IOException("NACK received");
			}
			return true;
		}

		static void writebit(boolean value) throws IOException, UnavailableDeviceException, ClosedDeviceException {
			if (value) {
				SDA.setValue(true);
			} else {
				SDA.setValue(false);
			}
			SCL.setValue(true);
			SCL.setValue(false);
		}

		static boolean readbit() throws IOException, UnavailableDeviceException, ClosedDeviceException {
			boolean val;
			SDA.setValue(true); //Release SDA
			SCL.setValue(true);	
			SDA.setDirection(GPIOPin.INPUT);
			val = SDA.getValue();
			SDA.setDirection(GPIOPin.OUTPUT);
			SCL.setValue(false);
			return val;
		}

		static void writebyte(byte cmd) throws IOException, UnavailableDeviceException, ClosedDeviceException {
			int bitmask = 0x80;
			while (bitmask != 0) {
				if ((cmd & (byte)(bitmask&0xff)) == 0) {
					writebit(false);
				} else {
					writebit(true);
				}
				bitmask >>>= 1;
			}
			//System.out.println("[I2C]writebyte "+cmd);
		}

		static byte readbyte() throws IOException, UnavailableDeviceException, ClosedDeviceException {
			byte val = 0;
			int bitmask = 0x80;
			while (bitmask != 0) {
				if (readbit()) {
					val |= (byte)(bitmask&0xff);
				}
				bitmask >>>= 1;
			}
			
			//System.out.println("[I2C]readByte "+val);

			return val;
		}
		
		static void sendSlaveAddress(int slave_address, int slave_address_size, boolean write) throws UnavailableDeviceException, ClosedDeviceException, IOException {
			slave_address = (slave_address << 1) & 0xfe;
			slave_address |= write?0:1;
			writebyte((byte)slave_address);
			readAck();			
		}

	}

	public void close() throws IOException {
		I2COperation.deinit();
		open = false;
	}

	public boolean isOpen() {
		return open;
	}

	public void tryLock(int timeout) throws UnavailableDeviceException, ClosedDeviceException, IOException {
		// TODO Auto-generated method stub
		
	}

	public void unlock() throws IOException {
		// TODO Auto-generated method stub
		
	}
}
