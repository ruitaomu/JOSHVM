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
package org.joshvm.j2me.dio.spibus;

import org.joshvm.util.ByteBuffer;

import org.joshvm.j2me.dio.ClosedDeviceException;
import org.joshvm.j2me.dio.UnavailableDeviceException;
import java.io.IOException;

/**
 * The {@code SPICompositeMessage} interface provides methods for constructing a
 * composite message. A composite message may be constituted of a sequence of read
 * and/or write operations to a single SPI slave device that will be performed as
 * a single transaction.
 * <p>
 * Here is an example of a composite message to a single slave:</p>
 * <blockquote>
 * <pre>
 * try (SPIDevice slave = DeviceManager.open("SPI1", SPIDevice.class, null)) {
 *    ByteBuffer sndBuf1 = ByteBuffer.wrap(new byte[] {0x01});
 *    ByteBuffer sndBuf2 = ByteBuffer.wrap(new byte[] {0x02});
 *    ByteBuffer rcvBuf = ByteBuffer.wrap(new byte[3]);
 *    int bytesRead = slave.createCompositeMessage()
 *             .appendwrite(sndBuf1)
 *             .appendDelay(100)
 *             .appendwriteAndRead(sndBuf2, rcvBuf)
 *             .transfer()[0];
 * } catch (IOException ioe) {
 *     <i>// Handle exception</i>
 * }
 * </pre>
 * </blockquote>
 * <p>
 * The preceding example is using a
 * <em>try-with-resources</em> statement; the
 * {@link SPIDevice#close SPIDevice.close} method is automatically invoked by
 * the platform at the end of the statement.</p>
 *
 * @since 1.0.1
 */
public interface SPICompositeMessage {

    /**
     * Appends a read message/operation from the targeted SPI slave device.
     * Reads up to {@code rxBuf.remaining()} bytes of data from the targeted slave
     * device into the buffer {@code rxBuf}.
     * <p>
     * Upon the invocation of the {@link #transfer transfer} method
     * the appended operation will have a behavior equivalent to that of the invocation of the
     * {@link SPIDevice#read(org.joshvm.util.ByteBuffer)} method on the targeted {@code SPIDevice}.
     * </p><p>
     * Buffers are not safe for use by multiple concurrent threads so care should
     * be taken to not access the provided buffer until the operation has completed - that
     * is: until the {@code transfer} method has been invoked and has returned.
     * </p>
     *
     * @param rxBuf the buffer into which the data is read.
     * @return a reference to this {@code SPICompositeMessage} object.
     * @throws NullPointerException if {@code rxBuf} is {@code null}.
     * @throws IllegalStateException if this message has already been assembled
     * and transferred once.
     * @throws ClosedDeviceException if the device has been closed.
     * @throws IOException if some other I/O error occurs.
     */
    SPICompositeMessage appendRead(ByteBuffer rxBuf) throws IOException, ClosedDeviceException;

    /**
     * Appends a read message/operation from the targeted SPI slave device.
     * Reads up to {@code rxBuf.remaining()} bytes of data from the targeted slave
     * device into the buffer skipping {@code rxBuf} the first {@code rxSkip}
     * bytes read.
     * <p>
     * Upon the invocation of the {@link #transfer transfer} method
     * the appended operation will have a behavior equivalent to that of the invocation of the
     * {@link SPIDevice#read(int, org.joshvm.util.ByteBuffer)} method on the targeted {@code SPIDevice}.
     * </p><p>
     * Buffers are not safe for use by multiple concurrent threads so care should
     * be taken to not access the provided buffer until the operation has completed - that
     * is: until the {@code transfer} method has been invoked and has returned.
     * </p>
     *
     * @param rxSkip the number of read bytes that must be ignored/skipped
     * before filling in the {@code rxBuf} buffer.
     * @param rxBuf the buffer into which the data is read.
     * @return a reference to this {@code SPICompositeMessage} object.
     * @throws NullPointerException if {@code rxBuf} is {@code null}.
     * @throws IllegalStateException if this message has already been assembled
     * and transferred once.
     * @throws ClosedDeviceException if the device has been closed.
     * @throws IllegalArgumentException if {@code rxSkip} is negative.
     * @throws IOException if some other I/O error occurs.
     */
    SPICompositeMessage appendRead(int rxSkip, ByteBuffer rxBuf) throws IOException, ClosedDeviceException;

    /**
     * Appends a write message/operation from the targeted SPI slave device.
     * Writes to the targeted slave device {@code txBuf.remaining()} bytes from the
     * buffer {@code txBuf}.
     * <p>
     * Upon the invocation of the {@link #transfer transfer} method
     * the appended operation will have a behavior equivalent to that of the invocation of the
     * {@link SPIDevice#write(org.joshvm.util.ByteBuffer)} method on the targeted {@code SPIDevice}.
     * </p><p>
     * Buffers are not safe for use by multiple concurrent threads so care should
     * be taken to not access the provided buffer until the operation has completed - that
     * is: until the {@code transfer} method has been invoked and has returned.
     * </p>
     *
     * @param txBuf the buffer containing the bytes to write.
     * @return a reference to this {@code SPICompositeMessage} object.
     * @throws NullPointerException if {@code txBuf} is {@code null}.
     * @throws IllegalStateException if this message has already been assembled
     * and transferred once.
     * @throws ClosedDeviceException if the device has been closed.
     * @throws IOException if some other I/O error occurs.
     */
    SPICompositeMessage appendWrite(ByteBuffer txBuf) throws IOException, ClosedDeviceException;

    /**
     * Appends an exchange message/operation from the targeted SPI slave device.
     * Exchanges (transmits and receives) data with the targeted slave device.
     * <p>
     * Upon the invocation of the {@link #transfer transfer} method
     * the appended operation will have a behavior equivalent to that of the invocation of the
     * {@link SPIDevice#writeAndRead(org.joshvm.util.ByteBuffer, com.joshvm.util.ByteBuffer)}
     * method on the targeted {@code SPIDevice}.
     * </p><p>
     * The designated portions of the sending and receiving byte buffers may not have the same length. When sending more
     * than is being received the extra received bytes are ignored/discarded. Conversely, when sending less than is
     * being received extra dummy data will be sent.
     * </p><p>
     * Buffers are not safe for use by multiple concurrent threads so care should
     * be taken to not access the provided buffer until the operation has completed - that
     * is: until the {@code transfer} method has been invoked and has returned.
     * </p>
     *
     * @param src
     *            The buffer from which bytes are to be retrieved.
     * @param dst
     *            The buffer into which bytes are to be transferred.
     * @return a reference to this {@code SPICompositeMessage} object.
     * @throws NullPointerException
     *             if {@code src} or {@code dst} is {@code null}.
     * @throws IllegalStateException if this message has already been assembled
     * and transferred once.
     * @throws ClosedDeviceException if the device has been closed.
     * @throws IOException if some other I/O error occurs
     */
    SPICompositeMessage appendWriteAndRead(ByteBuffer src, ByteBuffer dst) throws IOException, ClosedDeviceException;

    /**
     * Appends an exchange message/operation with the targeted SPI slave device.
     * Exchanges (transmits and receives) data with the targeted slave device skipping the specified number of bytes received.
     * <p>
     * Upon the invocation of the {@link #transfer transfer} method
     * the appended operation will have a behavior equivalent to that of the invocation of the
     * {@link SPIDevice#writeAndRead(org.joshvm.util.ByteBuffer, int, com.joshvm.util.ByteBuffer)}
     * method on the targeted {@code SPIDevice}.
     * </p><p>
     * The designated portions of the sending and receiving byte buffers may not have the same length. When sending more
     * than is being received the extra received bytes are ignored/discarded. Conversely, when sending less than is
     * being received extra dummy data will be sent.
     * </p><p>
     * Buffers are not safe for use by multiple concurrent threads so care should
     * be taken to not access the provided buffer until the operation has completed - that
     * is: until the {@code transfer} method has been invoked and has returned.
     * </p>
     *
     * @param src
     *            The buffer from which bytes are to be retrieved.
     * @param skip
     *            the number of received bytes that must be ignored/skipped before filling in the {@code dst} buffer.
     * @param dst
     *            The buffer into which bytes are to be transferred.
     * @return a reference to this {@code SPICompositeMessage} object.
     * @throws NullPointerException
     *             if {@code src} or {@code dst} is {@code null}.
     * @throws IllegalArgumentException
     *              if {@code skip} is negative.
     * @throws IllegalStateException if this message has already been assembled
     * and transferred once.
     * @throws ClosedDeviceException if the device has been closed.
     * @throws IOException
     *             if some other I/O error occurs
     */
    SPICompositeMessage appendWriteAndRead(ByteBuffer src, int skip, ByteBuffer dst) throws IOException, ClosedDeviceException;

    /**
     * Appends a delay (in microseconds).
     * Delays the next appended operation by the specified amount of microseconds.
     * <p>
     * Upon the invocation of the {@link #transfer transfer} method
     * the next operation (or the completion of this composite message transfer if this
     * is the last operation) will be delayed by the specified amount of microseconds.
     * </p><p>
     * If the underlying platform or driver does not support a microsecond timer resolution
     * or does not support the requested delay value then {@code delay} will be <em>rounded up</em>
     * to accommodate the supported timer resolution or the closest greater supported discrete delay value, if any.
     * </p>
     *
     * @param delay
     *            the amount (in microseconds) to delay the next operation.
     * @return a reference to this {@code SPICompositeMessage} object.
     * @throws IllegalArgumentException
     *              if {@code delay} is negative or cannot be accommodated for by rounding it up to
     *              a supported value.
     * @throws IllegalStateException if this message has already been assembled
     * and transferred once.
     * @throws ClosedDeviceException if the device has been closed.
     * @throws IOException
     *             if some other I/O error occurs
     */
    SPICompositeMessage appendDelay(int delay)  throws IOException, ClosedDeviceException;

    /**
     * Transfers this composite message to the targeted {@code SPIDevice}. This will result in each of the
     * contained messages/operations to be sent/executed in the same order they
     * have been appended to this composite message.
     * <p>
     * This method may be invoked at any time. If another thread has already initiated a transaction
     * (see {@link jdk.dio.Transactional}) or, a read or write operation upon the targeted slave device,
     * however, then an invocation of this method will block until the first operation is complete.
     * </p><p>
     * Once transferred no additional operation can be appended anymore to this
     * composite message. Any such attempt will result in a
     * {@link IllegalStateException} to be thrown.
     * Nevertheless, this {@code SPICompositeMessage} object can be reused several times to perform
     * the same sequence of operations. The data transferred
     * to or from each of the provided {@code ByteBuffer}s is determined by their respective current {@code position}
     * and {@code remaining} attributes at the time this method is call.
     * <br>
     * Buffers are not safe for use by multiple concurrent threads so care should
     * be taken to not access the provided buffers until the transfer has completed.
     * </p>
     *
     * @return an array (possibly empty) containing the number of bytes read for each of the read
     * operations of this composite message; the results of each read operations
     * appear in the very same order the read operations have been appended to
     * this composite message.
     * @throws InvalidWordLengthException if the total number of bytes to receive
     * or send of any of the appended operations belies word length.
     * @throws UnavailableDeviceException if the targeted devices is not
     * currently available - such as it is locked by another application.
     * @throws ClosedDeviceException if the targeted device is has been closed.
     * @throws IOException if some other I/O error occurred
     */
    int[] transfer() throws IOException, UnavailableDeviceException, ClosedDeviceException;

    /**
     * Gets the {@code SPIDevice} this composite message is targeting.
     * @return the targeted SPI device slave.
     */
    SPIDevice getTargetedDevice();
}
