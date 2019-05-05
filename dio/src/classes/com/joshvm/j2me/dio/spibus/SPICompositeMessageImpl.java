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
 * Copyright (c) 2015, Oracle and/or its affiliates. All rights reserved.
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

import org.joshvm.util.ByteBuffer;
import org.joshvm.util.ArrayList;
import com.joshvm.j2me.dio.utils.ExceptionMessage;

import java.io.InterruptedIOException;

import org.joshvm.j2me.dio.ClosedDeviceException;
import org.joshvm.j2me.dio.UnavailableDeviceException;
import org.joshvm.j2me.dio.spibus.SPICompositeMessage;
import org.joshvm.j2me.dio.spibus.SPIDevice;



final class SPICompositeMessageImpl implements SPICompositeMessage {

    private final ArrayList messageList = new ArrayList();

    private boolean isAlreadyTransferedOnce;

    /* Owner of the message */
    private final SPISlaveImpl device;

    // delay between operations
    private int delay;

    // number of messages to read
    private int rxMsgs;

    private class Message {
        ByteBuffer tx, rx, newTx, newRx;
        int skip, delay;

        public Message(ByteBuffer tx, int skip, ByteBuffer rx, int delay) {
            this.rx = rx;
            this.skip = skip;
            this.tx = tx;
            this.delay = delay;
        }
    }

    private void checkStatus() throws ClosedDeviceException {
        if (isAlreadyTransferedOnce) {
            throw new IllegalStateException(
                    ExceptionMessage.format(ExceptionMessage.I2CBUS_ALREADY_TRANSFERRED_MESSAGE)
            );
        }
        if (!device.isOpen()) {
            throw new ClosedDeviceException();
        }
    }

    private void check(Message message) throws ClosedDeviceException {

        checkStatus();

        if (0 > message.skip) {
            throw new IllegalArgumentException(
                    ExceptionMessage.format(ExceptionMessage.I2CBUS_NEGATIVE_SKIP_ARG)
            );
        }

        for (int i = 0; i < messageList.size(); i++) {
            ByteBuffer tx = ((Message)messageList.get(i)).tx;
            ByteBuffer rx = ((Message)messageList.get(i)).rx;
            if ((null != tx && (tx == message.tx ||
                    tx == message.rx))
                    || (null != rx && (rx == message.tx ||
                    rx == message.rx))) {
                throw new IllegalArgumentException(
                        ExceptionMessage.format(ExceptionMessage.I2CBUS_BUFFER_GIVEN_TWICE)
                );
            }
        }

        if (null != message.rx) {
            rxMsgs++;
        }
    }

    /**
     * Creates a new {@code SPICompositeMessageImpl} instance.
     */
    SPICompositeMessageImpl(SPISlaveImpl device) {
        this.device = device;
    }

    public SPICompositeMessage appendRead(ByteBuffer rxBuf) throws IOException, ClosedDeviceException {
        return appendRead(0, rxBuf);
    }

    public SPICompositeMessage appendRead(int rxSkip, ByteBuffer rxBuf) throws IOException, ClosedDeviceException {
        //null check
        rxBuf.limit();
        return append(null, rxSkip, rxBuf);
    }

    public SPICompositeMessage appendWrite(ByteBuffer txBuf) throws IOException,
            ClosedDeviceException {
        //null check
        txBuf.limit();
        return append(txBuf, 0, null);
    }

    public SPICompositeMessage appendWriteAndRead(ByteBuffer src, ByteBuffer dst) throws IOException, ClosedDeviceException {
        return appendWriteAndRead(src, 0, dst);
    }

    public SPICompositeMessage appendWriteAndRead(ByteBuffer src, int skip, ByteBuffer dst) throws IOException, ClosedDeviceException {
        //null check
        src.limit();
        dst.limit();
        return append(src, skip, dst);
    }

    private synchronized SPICompositeMessage append(ByteBuffer src, int skip, ByteBuffer dst) throws IOException, ClosedDeviceException {
        Message message = new Message(src, skip, dst, delay);
        check(message);
        messageList.add(message);
        return this;
    }

    public synchronized SPICompositeMessage appendDelay(int delay) throws ClosedDeviceException {

        checkStatus();

        this.delay = delay;
        return this;
    }

    public SPIDevice getTargetedDevice() {
        return device;
    }

    /**
     * Returns buffers are suitable for low level SPI operations
     * New src buffer is located at index 0, dst buffer is at index 1
     * The both buffers are direct to avoid native resource allocations in low levels
     * The both buffers are the same length
     *
     * @param originalSrc original array to be sent
     * @param originalDst The buffer into which bytes are to be transferred
     * @return New buffers in the array. newSrcBuf = array[0], newDstBuf = array[1]
     */
    private ByteBuffer[] getBuffersForTransfer(ByteBuffer originalSrc, int skip, ByteBuffer originalDst) {
        int bytesInSrc = originalSrc == null ? 0 : originalSrc.remaining();
        int bytesInDst = originalDst == null ? 0 : originalDst.remaining();

        int newRequiredSizeOfBuffers = bytesInSrc < (skip + bytesInDst) ?
                skip + bytesInDst :
                bytesInSrc;

        ByteBuffer[] array = new ByteBuffer[2];

        if (originalSrc == null || !originalSrc.isDirect() || originalSrc.remaining() < newRequiredSizeOfBuffers) {
            array[0] = (ByteBuffer) ByteBuffer.allocateDirect(newRequiredSizeOfBuffers);
            if (originalSrc != null) {
                array[0].put(originalSrc);
            }
            array[0].rewind();
        } else {
            //Can not use originalSrc as is, because caller code can change position and limit
            //after calling read/write/append operations
            array[0] = originalSrc.slice();
        }

        if (originalDst == null || !originalDst.isDirect() || originalDst.remaining() < newRequiredSizeOfBuffers) {
            array[1] = ByteBuffer.allocateDirect(newRequiredSizeOfBuffers);
        } else {
            //Can not use originalDst as is, because caller code can change position and limit
            //after calling read/write/append operations
            array[1] = originalDst.slice();
        }

        return array;
    }

    public int[] transfer() throws IOException, UnavailableDeviceException, ClosedDeviceException {
        // global handle lock to prevent access from other threads.
        synchronized (device) {
            /* Forbid adding more messages to this combined message */
            isAlreadyTransferedOnce = true;
            if (0 == messageList.size()) {
                return new int[0];
            }

            device.beginTransaction();

            try {
                final int size = messageList.size();
                for (int i = 0; i < size; i++) {
                    Message message = (Message)messageList.get(i);
                    ByteBuffer[] newBuffers = getBuffersForTransfer(message.tx, message.skip, message.rx);
                    message.newTx = newBuffers[0];
                    message.newRx = newBuffers[1];

                    //New buffers have the same length and to avoid a dummy native call
                    //just make tranfers of non-empty buffers
                    if (message.newRx.remaining() > 0) {
                        device.transferWithLock(message.newTx, message.newRx);
                    }

					if (message.delay > 0) {
                    	Thread.currentThread().sleep(message.delay / 1000);
					}
                }

            } catch (InterruptedException ex) {
                throw new InterruptedIOException(ExceptionMessage.format(
                        ExceptionMessage.SPIBUS_TRANSFER_INTERRUPTED));
            } finally {
                device.endTransaction();
            }

            int i = 0;
			int j = 0;
            int[] bytesRead = new int[rxMsgs];

			Message message;			
            for (i = 0; i < messageList.size(); i++) {
				message = (Message)messageList.get(i);
                if (message.tx != null) {
                    message.tx.position(message.tx.position()+message.newTx.position());
                }

                if (message.rx != null) {
					message.newRx.flip();
                    message.newRx.position(message.newRx.position()+message.skip);
                    bytesRead[j++] = message.newRx.remaining();
                    message.rx.put(message.newRx);
                }

                message.newRx = null;
                message.newTx = null;
            }

            return bytesRead;
        }
    }
}
