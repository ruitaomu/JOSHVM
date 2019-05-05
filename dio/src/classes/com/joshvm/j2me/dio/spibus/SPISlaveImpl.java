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

package com.joshvm.j2me.dio.spibus;

import java.io.IOException;

import com.joshvm.j2me.dio.utils.Constants;
import com.joshvm.j2me.dio.utils.ExceptionMessage;
import org.joshvm.util.ByteBuffer;
import org.joshvm.j2me.dio.ClosedDeviceException;
import org.joshvm.j2me.dio.DeviceConfig;
import org.joshvm.j2me.dio.DeviceManager;
import org.joshvm.j2me.dio.DeviceNotFoundException;
import org.joshvm.j2me.dio.InvalidDeviceConfigException;
import org.joshvm.j2me.dio.UnavailableDeviceException;
import org.joshvm.j2me.dio.UnsupportedAccessModeException;
import org.joshvm.j2me.dio.spibus.InvalidWordLengthException;
import org.joshvm.j2me.dio.spibus.SPICompositeMessage;
import org.joshvm.j2me.dio.spibus.SPIDevice;
import org.joshvm.j2me.dio.spibus.SPIDeviceConfig;

/**
 *Implementation of SPISlave Interface.
 */
public class SPISlaveImpl implements SPIDevice {

    //every call checkWordLen updates these two variables
    private int byteNum;
    private int bitNum;   
	private int bitOrdering;
	private int handle;
	private int csActiveLevel;
	
    public SPISlaveImpl(SPIDeviceConfig cfg, int mode) throws
            IOException, DeviceNotFoundException, InvalidDeviceConfigException {

		handle = -1;
        if(cfg.getControllerName() != null) {
            throw new InvalidDeviceConfigException(
                ExceptionMessage.format(ExceptionMessage.DEVICE_OPEN_WITH_DEVICENAME_UNSUPPORTED)
            );
        }

        handle = openSPIDeviceByConfig0(cfg.getControllerNumber(), cfg.getAddress(),
                                        cfg.getCSActiveLevel(), cfg.getClockFrequency(),
                                        cfg.getClockMode(), cfg.getWordLength(),
                                        cfg.getBitOrdering(), mode == DeviceManager.EXCLUSIVE);

        //initPowerManagement();

		if (handle == -1) {
			throw new IOException("openSPIDeviceByConfig0 failed");
		}

        bitNum = cfg.getWordLength();
        if (bitNum > Constants.MAX_WORD_LEN) {
            throw new IOException (
                ExceptionMessage.format(ExceptionMessage.SPIBUS_SLAVE_WORD_LENGTH, bitNum)
            );
        }
        byteNum = (bitNum - 1)/8 + 1;
		csActiveLevel = cfg.getCSActiveLevel();
		bitOrdering = cfg.getBitOrdering();
    }

    public SPICompositeMessage createCompositeMessage() {
        return new SPICompositeMessageImpl(this);
    }

    /**
     * Gets the transfer word length in bits supported by this slave device.
     * <p>
     * If the length of data to be exchanged belies a slave's word length an {@link InvalidWordLengthException} will be
     * thrown.
     *
     * @return this slave's transfer word length in bits.
     * @throws IOException
     *             if some other I/O error occurs.
     * @throws UnavailableDeviceException
     *             if this peripheral is not currently available - such as it is locked by another application.
     * @throws ClosedDeviceException
     *             if the peripheral has been closed.
     */
    public synchronized int getWordLength() throws IOException,
            UnavailableDeviceException, ClosedDeviceException {
        //checkPowerState();
        return bitNum;
    }

    /**
     * Reads one data word of up to 32 bits from this slave device.
     *
     * @return the data word read.
     * @throws IOException
     *             if some other I/O error occurs.
     * @throws InvalidWordLengthException
     *             if the number of bytes to receive belies word length; that is this slave's word length is bigger than
     *             32 bits.
     * @throws UnavailableDeviceException
     *             if this peripheral is not currently available - such as it is locked by another application.
     * @throws ClosedDeviceException
     *             if the peripheral has been closed.
     */
    public int read() throws IOException, UnavailableDeviceException,
            ClosedDeviceException {
        ByteBuffer dst = ByteBuffer.allocateDirect(byteNum);        
        transferInternal(null, 0, dst);
        return byteArray2int(dst);
    }

    /**
     * Reads a sequence of bytes from this slave device into the given buffer.
     * <p />
     * Dummy data will be sent to this slave device by the platform.
     * <p />
     * {@inheritDoc}
     *
     * @param dst
     *            The buffer into which bytes are to be transferred
     *
     * @return The number of bytes read, possibly zero, or {@code -1} if the device has reached end-of-stream
     *
     * @throws NullPointerException
     *             If {@code dst} is {@code null}.
     * @throws InvalidWordLengthException
     *             if the number of bytes to receive belies word length.
     * @throws UnavailableDeviceException
     *             if this peripheral is not currently available - such as it is locked by another application.
     * @throws ClosedDeviceException
     *             if the peripheral has been closed.
     * @throws IOException
     *             If some other I/O error occurs
     */
    public int read(ByteBuffer dst) throws IOException,
            UnavailableDeviceException, ClosedDeviceException {
        return read(0, dst);
    }

    /**
     * Reads a sequence of bytes from this device into the given buffer, skipping the first {@code skip} bytes read.
     * <p />
     * Dummy data will be sent to this slave device by the platform.
     * <p />
     * Apart from skipping the first {@code skip} bytes, this method behaves identically to
     * {@link #read(org.joshvm.util.ByteBuffer)}.
     *
     * @param skip
     *            the number of read bytes that must be ignored/skipped before filling in the {@code dst} buffer.
     * @param dst
     *            The buffer into which bytes are to be transferred
     *
     * @return The number of bytes read, possibly zero, or {@code -1} if the device has reached end-of-stream
     *
     * @throws NullPointerException
     *             If {@code dst} is {@code null}.
     * @throws InvalidWordLengthException
     *             if the total number of bytes to receive belies word length.
     * @throws UnavailableDeviceException
     *             if this peripheral is not currently available - such as it is locked by another application.
     * @throws ClosedDeviceException
     *             if the peripheral has been closed.
     * @throws IOException
     *             If some other I/O error occurs
     */
    public int read(int skip, ByteBuffer dst) throws IOException,
            UnavailableDeviceException, ClosedDeviceException {
        if (0 > skip) {
            throw new IllegalArgumentException();
        }
        checkBuffer(dst);
        return transferInternal(null, skip, dst);
    }

    public int write(ByteBuffer src) throws IOException,
            UnavailableDeviceException, ClosedDeviceException {
        checkBuffer(src);
        return transferInternal(src, 0, null);
    }

    public void write(int txData) throws IOException,
            UnavailableDeviceException, ClosedDeviceException {
        writeAndRead(txData);
    }

    public int writeAndRead(ByteBuffer src, ByteBuffer dst)
            throws IOException, UnavailableDeviceException,
            ClosedDeviceException {
        return writeAndRead(src, 0, dst);
    }

    public int writeAndRead(ByteBuffer src, int skip, ByteBuffer dst)
            throws IOException, UnavailableDeviceException,
            ClosedDeviceException {
        if (0 > skip) {
            throw new IllegalArgumentException();
        }
        checkBuffer(src);
        checkBuffer(dst);
        return transferInternal(src, skip, dst);
    }

    public int writeAndRead(int txData) throws IOException, UnavailableDeviceException, ClosedDeviceException{
        ByteBuffer tx = int2byteArray(txData);
        ByteBuffer rx = tx.slice();
        transferInternal(tx, 0, rx);
        return byteArray2int(rx);
    }

    public ByteBuffer getInputBuffer() throws ClosedDeviceException,
            IOException {
        throw new java.lang.UnsupportedOperationException();
    }

    public ByteBuffer getOutputBuffer() throws ClosedDeviceException,
            IOException {
        throw new java.lang.UnsupportedOperationException();
    }

    // checkWordLen ought to be called before checkBuffer to get byteNum is up to date
    protected void checkBuffer(ByteBuffer buffer) {

        if ((buffer.remaining() % byteNum) != 0) {
            throw new InvalidWordLengthException(
                ExceptionMessage.format(ExceptionMessage.SPIBUS_BYTE_NUMBER_BELIES_WORD_LENGTH)
            );
        }
    }

    private ByteBuffer int2byteArray(int intVal) {
		// TODO: Consider bitOrdering??
        ByteBuffer retA = ByteBuffer.allocateDirect(byteNum);
        for (int i=0; i< byteNum ; i++) {
            retA.put((byte)((intVal >> (8*(byteNum-i-1))) & 0xff));
        }
        retA.flip();
        return retA;
    }

    private int byteArray2int(ByteBuffer byteA) {
        // TODO: Consider bitOrdering??
        byteA.rewind();
        int retI = 0;
        int tmp;
        for (int i = 0; i< byteNum ;i++) {
            tmp = byteA.get();
            retI |= ((tmp & 0xff ) << (8*(byteNum - i - 1)));
        }
        return retI;
    }
    
    /* Returns number of recevied bytes if dst is not NULL, or number of sent bytes otherwise */
    private int transferInternal(ByteBuffer src, int skip, ByteBuffer dst) throws IOException {
        int returnCount = dst != null ? dst.remaining() : src.remaining();
        SPICompositeMessage message = this.createCompositeMessage();
        
        if(src == null && dst != null) {
            message.appendRead(skip, dst);
        } else if(dst == null && src != null) {
            message.appendWrite(src);
        } else {
            message.appendWriteAndRead(src, skip, dst);            
        }
              
        message.transfer();
        
        return returnCount;  
    }
    
    
    /**
     * Performs a SPI transfer operation with locking the SPI peripheral
     * The both buffers should be direct and same length
     *
     * @param src 
     *            Direct byte buffer which will be sent to the slave device
     * @param dst 
     *            Direct byte buffer which will used for the saving all the 
     *            data from slave 
     *
     */    
    void transferWithLock(ByteBuffer src, ByteBuffer dst) throws IOException {
    	// TODO: Is inter-app Lock really needed???
    	byte[] src_array = src.array();
		byte[] dst_array = dst.array();
		if (src.remaining() != dst.remaining()) {
			throw new IllegalArgumentException("Input and Output buffers are not in same size");
		}

        int bytesTransferred = writeAndRead0(handle, src_array, src.position()+src.arrayOffset(), 
					  dst_array, dst.position()+dst.arrayOffset(),
					  src.remaining());
		src.position(src.position()+bytesTransferred);
		dst.position(dst.position()+bytesTransferred);
    }

    void beginTransaction() throws IOException {
        // interapp lock
        //conditionalLock();
		if (SPIDeviceConfig.CS_NOT_CONTROLLED != csActiveLevel) {
			begin0(handle, csActiveLevel);
		}        
    }

    void endTransaction(){
		if (SPIDeviceConfig.CS_NOT_CONTROLLED != csActiveLevel) {
	        end0(handle, csActiveLevel);
		}
        // interapp unlock
        //conditionalUnlock();
    }

	
	public synchronized void tryLock(int timeout) throws UnavailableDeviceException, ClosedDeviceException, java.io.IOException {
	}

	public synchronized void close() throws java.io.IOException {
		close0(handle);
		handle = -1;
	}
	
	public synchronized boolean isOpen() {
		if (handle != -1) return true;
		else return false;
	}
	
	public synchronized void unlock() throws java.io.IOException {
	}
	

    private native void begin0(int handle, int csActiveLevel) throws IOException;
    private native void end0(int handle, int csActiveLevel);

	/**
	 * If return is -1, then means error happened in open operation.
	 **/
    private native int openSPIDeviceByConfig0(int deviceNumber, int address,
                                              int csActive, int clockFrequency,
                                              int clockMode, int wordLen,
                                              int bitOrdering, boolean exclusive) throws
            IOException, DeviceNotFoundException, InvalidDeviceConfigException, UnavailableDeviceException, UnsupportedAccessModeException;

    /* PREREQUISITES: either dst.len must be equals to src.len*/
    private native int writeAndRead0(int handle, byte[] src, int src_offset, byte[] dst, int dst_offset, int length) throws IOException;
	private native void close0(int handle);
}
