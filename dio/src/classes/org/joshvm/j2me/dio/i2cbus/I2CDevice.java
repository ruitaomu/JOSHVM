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
 * Copyright (c) 2013, 2015, Oracle and/or its affiliates. All rights reserved.
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
package org.joshvm.j2me.dio.i2cbus;

import java.io.IOException;

import org.joshvm.j2me.dio.ClosedDeviceException;
import org.joshvm.j2me.dio.Device;
import org.joshvm.j2me.dio.DeviceManager;
import org.joshvm.j2me.dio.UnavailableDeviceException;

import org.joshvm.util.ByteBuffer;

/**
 * The {@code I2CDevice} interface provides methods for sending and receiving data to/from an I2C
 * slave device.
 * <p>
 * An I2C slave device may be identified by the numeric ID and by the name (if any defined) that
 * correspond to its registered configuration. An {@code I2CDevice} instance can be opened by a call
 * to one of the {@link DeviceManager#open(int) DeviceManager.open(id,...)} methods using
 * its ID or by a call to one of the
 * {@link DeviceManager#open(java.lang.String, java.lang.Class, java.lang.String[])
 * DeviceManager.open(name,...)} methods using its name. When an {@code I2CDevice} instance is
 * opened with an ad-hoc {@link I2CDeviceConfig} configuration (which includes its hardware
 * addressing information) using one of the
 * {@link DeviceManager#open(org.joshvm.j2me.dio.DeviceConfig)
 * DeviceManager.open(config,...)} methods it is not assigned any ID nor name.
 * </p><p>
 * On an I2C bus, data is transferred between the I2C master device and an I2C slave device through
 * single or combined messages:</p>
 * <blockquote>
 * <dl>
 * <dt><b>Single Messages</b></dt>
 * <dd>The I2C master reads data from an I2C slave device. An application can read from an I2C slave
 * device using one of the {@link #read read} methods. <br>
 * The I2C master writes data to an I2C slave device. An application can write to an I2C slave
 * device using one of the {@link #write write} methods.</dd>
 * <dt><b>Combined Messages Messages</b></dt>
 * <dd>The I2C master issues at least two reads and/or writes to one or more slaves.
 * <blockquote>
 * <dl>
 * <dt><b>To a single slave</b></dt>
 * <dd> An application can use the methods {@link #read(int, int, ByteBuffer) read(subaddress, subaddressSize,...)} and
 * {@link #write(int, int, ByteBuffer) write(subaddress, subaddressSize,...)} that rely on combined messages to respectively read
 * and write from slave device subaddresses or register addresses. </dd>
 * <dt><b>To several slaves</b></dt>
 * <dd>An application can issue several reads and/or writes to several slaves using a
 * {@link I2CCombinedMessage} object.</dd>
 * </dl>
 * </blockquote>
 * </dd>
 * </dl>
 * </blockquote>
 * <p>
 * When the data exchange is over, an application should call the {@link #close close} method to
 * close the I2C slave device. Any further attempt to write to or read from an I2C slave device
 * which has been closed will result in a {@link ClosedDeviceException} been thrown.
 * </p><p>
 * Opening an {@link I2CDevice} instance is subject to permission checks (see {@link I2CPermission}
 * ).</p>
 *
 * @see I2CCombinedMessage
 * @see I2CPermission
 * @see ClosedDeviceException
 * @since 1.0
 */
public interface I2CDevice extends Device {

    /**
     * The {@code Bus} interface is a simplified abstraction of an I2C bus
     * providing methods for creating combined messages.
     * @since 1.0
     */
    public interface Bus {

        /**
         * Creates a new {@code I2CCombinedMessage} instance.
         *
         * @return a new {@code I2CCombinedMessage} instance.
         */
        I2CCombinedMessage createCombinedMessage();
    }

    /**
     * Retrieves the {@code Bus} instance representing the I2C bus this device is
     * connected to.
     * <br>
     * Even if this device is closed, a {@code Bus} instance for
     * this device can still be retrieved.
     *
     * @return the {@code Bus} instance for this I2C device.
     *
     * @throws IOException
     *             if some other I/O error occurs.
     */
    Bus getBus() throws IOException;

    /**
     * Reads one byte of data from this slave device. The byte is returned as an {@code int} in the
     * range {@code 0} to {@code 255}.
     * <p>
     * This method may be invoked at any time. If another thread has already initiated a read or write
     * operation upon this slave device, however, then an invocation of this method will block until
     * the first operation is complete.
     * </p>
     *
     * @return the unsigned 8-bit value read.
     * @throws IOException
     *             if some other I/O error occurs (e.g. a <em>NACK</em> from the slave device was received).
     * @throws UnavailableDeviceException
     *             if this device is not currently available - such as it is locked by another
     *             application.
     * @throws ClosedDeviceException
     *             if the device has been closed.
     */
    int read() throws IOException, UnavailableDeviceException, ClosedDeviceException;

    /**
     * Reads a sequence of bytes from this slave device into the given buffer.
     * <p>
     * An attempt is made to read up to <i>r</i> bytes from the device, where <i>r</i> is the number
     * of bytes remaining in the buffer, that is, {@code dst.remaining()}, at the moment this method
     * is invoked.
     * </p><p>
     * Suppose that a byte sequence of length <i>n</i> is read, where <i>{@code 0 <= n <= r}</i>.
     * This byte sequence will be transferred into the buffer so that the first byte in the sequence
     * is at index <i>p</i> and the last byte is at index <i>{@code p + n - 1}</i>, where <i>p</i>
     * is the buffer's position at the moment this method is invoked. Upon return the buffer's
     * position will be equal to <i>{@code p + n}</i>; its limit will not have changed.
     * </p><p>
     * A read operation will block until the requested <i>r</i> bytes are read.
     * </p><p>
     * This method may be invoked at any time. If another thread has already initiated a read or write
     * operation upon this slave device, however, then an invocation of this method will block until
     * the first operation is complete.
     * </p><p>
     * This method may be invoked at any time. If another thread has already
     * initiated a read upon this device, however, then an invocation
     * of this method will block until the first operation is complete.
     * </p>
     *
     * @param dst
     *            the buffer into which bytes are to be transferred.
     * @return the number of bytes read into {@code dst}, possibly zero.
     * @throws NullPointerException
     *             if {@code dst} is {@code null}.
     * @throws UnavailableDeviceException
     *             if this device is not currently available - such as it is locked by another
     *             application.
     * @throws ClosedDeviceException
     *             if the device has been closed.
     * @throws IOException
     *             if some other I/O error occurs (e.g. a <em>NACK</em> from the slave device was received).
     */
    int read(ByteBuffer dst) throws IOException, UnavailableDeviceException, ClosedDeviceException;

    /**
     * Reads a sequence of bytes from this device into the given buffer, skipping the first
     * {@code skip} bytes read.
     * <p>
     * Apart from skipping the first {@code skip} bytes, this method behaves identically to
     * {@link #read(java.nio.ByteBuffer)}.
     * </p>
     *
     * @param skip
     *            the number of read bytes that must be ignored/skipped before filling in the
     *            {@code dst} buffer.
     * @param dst
     *            the buffer into which bytes are to be transferred.
     * @return the number of bytes read into {@code dst}, possibly zero.
     * @throws NullPointerException
     *             if {@code dst} is {@code null}.
     * @throws IllegalArgumentException
     *              if {@code skip} is negative.
     * @throws UnavailableDeviceException
     *             if this device is not currently available - such as it is locked by another
     *             application.
     * @throws ClosedDeviceException
     *             if the device has been closed.
     * @throws IOException
     *             if some other I/O error occurs (e.g. a <em>NACK</em> from the slave device was received).
     */
    int read(int skip, ByteBuffer dst) throws IOException, UnavailableDeviceException, ClosedDeviceException;

    /**
     * Reads a sequence of bytes from a subaddress or register address of this slave device into the
     * given buffer. The most significant bytes (MSB) of the subaddress or register address are
     * transferred first.
     * <p>
     * An attempt is made to read up to <i>r</i> bytes from the device, where <i>r</i> is the number
     * of bytes remaining in the buffer, that is, {@code dst.remaining()}, at the moment this method
     * is invoked.
     * </p><p>
     * Suppose that a byte sequence of length <i>n</i> is read, where <i>{@code 0 <= n <= r}</i>.
     * This byte sequence will be transferred into the buffer so that the first byte in the sequence
     * is at index <i>p</i> and the last byte is at index <i>{@code p + n - 1}</i>, where <i>p</i>
     * is the buffer's position at the moment this method is invoked. Upon return the buffer's
     * position will be equal to <i>{@code p + n}</i>; its limit will not have changed.
     * </p><p>
     * A read operation will block until the requested <i>r</i> bytes are read.
     * </p><p>
     * This method may be invoked at any time. If another thread has already initiated a read or write
     * operation upon this slave device, however, then an invocation of this method will block until
     * the first operation is complete.
     * </p>
     *
     * @param subaddress
     *            the slave device's subaddress or register address from where to start reading.
     * @param subaddressSize
     *            the slave device's subaddress or register address size (1-4 bytes).
     * @param dst
     *            the buffer into which bytes are to be transferred.
     * @return the number of bytes read into {@code dst}, possibly zero.
     * @throws NullPointerException
     *             if {@code dst} is {@code null}.
     * @throws UnavailableDeviceException
     *             if this device is not currently available - such as it is locked by another
     *             application.
     * @throws ClosedDeviceException
     *             if the device has been closed.
     * @throws IOException
     *             if some other I/O error occurs (e.g. a <em>NACK</em> from the slave device was received).
     * @throws IllegalArgumentException
     *             if {@code subaddress} is negative or {@code subaddressSize} is not between
     *             {@code 1} and {@code 4}.
     */
    int read(int subaddress, int subaddressSize, ByteBuffer dst) throws IOException, UnavailableDeviceException,
            ClosedDeviceException;

    /**
     * Reads a sequence of bytes from a subaddress or register address of this device into the given
     * buffer, skipping the first {@code skip} bytes read. The most significant bytes (MSB) of the
     * subaddress or register address are transferred first.
     * <p>
     * An attempt is made to read up to <i>r</i> bytes from the device, where <i>r</i> is the number
     * of bytes remaining in the buffer, that is, {@code dst.remaining()}, at the moment this method
     * is invoked.
     * </p><p>
     * Suppose that a byte sequence of length <i>n</i> is read, where <i>{@code 0 <= n <= r}</i>.
     * This byte sequence will be transferred into the buffer so that the first byte in the sequence
     * is at index <i>p</i> and the last byte is at index <i>{@code p + n - 1}</i>, where <i>p</i>
     * is the buffer's position at the moment this method is invoked. Upon return the buffer's
     * position will be equal to <i>{@code p + n}</i>; its limit will not have changed.
     * </p><p>
     * A read operation will block until the requested <i>r</i> bytes are read.
     * </p><p>
     * This method may be invoked at any time. If another thread has already initiated a read or write
     * operation upon this slave device, however, then an invocation of this method will block until
     * the first operation is complete.
     * </p>
     *
     * @param subaddress
     *            the slave device's subaddress or register address from where to start reading.
     * @param subaddressSize
     *            the slave device's subaddress or register address size (1-4 bytes).
     * @param skip
     *            the number of read bytes that must be ignored/skipped before filling in the
     *            {@code dst} buffer.
     * @param dst
     *            the buffer into which bytes are to be transferred.
     * @return the number of bytes read into {@code dst}, possibly zero.
     * @throws NullPointerException
     *             if {@code dst} is {@code null}.
     * @throws UnavailableDeviceException
     *             if this device is not currently available - such as it is locked by another
     *             application.
     * @throws ClosedDeviceException
     *             if the device has been closed.
     * @throws IOException
     *             if some other I/O error occurs (e.g. a <em>NACK</em> from the slave device was received).
     * @throws IllegalArgumentException
     *             if {@code skip} or {@code subaddress} is negative or {@code subaddressSize} is not between
     *             {@code 1} and {@code 4}.
     */
    int read(int subaddress, int subaddressSize, int skip, ByteBuffer dst) throws IOException,
            UnavailableDeviceException, ClosedDeviceException;

    /**
     * Writes a sequence of bytes to this slave device from the given buffer.
     * <p>
     * An attempt is made to write up to <i>r</i> bytes to the device, where <i>r</i> is the number
     * of bytes remaining in the buffer, that is, {@code src.remaining()}, at the moment this method
     * is invoked.
     * </p><p>
     * Suppose that a byte sequence of length <i>n</i> is written, where <i>{@code 0 <= n <= r}</i>.
     * This byte sequence will be transferred from the buffer starting at index <i>p</i>, where
     * <i>p</i> is the buffer's position at the moment this method is invoked; the index of the last
     * byte written will be <i>{@code p + n - 1}</i>. Upon return the buffer's position will be
     * equal to <i>{@code p + n}</i>; its limit will not have changed.
     * </p><p>
     * A write operation will return only after writing all of the <i>r</i> requested bytes.
     * </p><p>
     * This method may be invoked at any time. If another thread has already initiated a read or write
     * operation upon this slave device, however, then an invocation of this method will block until
     * the first operation is complete.
     * </p>
     *
     * @param src
     *            the buffer from which bytes are to be retrieved.
     * @return the number of bytes written from {@code src}, possibly zero.
     * @throws NullPointerException
     *             if {@code src} is {@code null}.
     * @throws UnavailableDeviceException
     *             if this device is not currently available - such as it is locked by another
     *             application.
     * @throws ClosedDeviceException
     *             if the device has been closed.
     * @throws IOException
     *             if some other I/O error occurs (e.g. a <em>NACK</em> from the slave device was received).
     */
    int write(ByteBuffer src) throws IOException, UnavailableDeviceException, ClosedDeviceException;

    /**
     * Writes one byte to this slave device. The eight low-order bits of the argument {@code data}
     * are written. The 24 high-order bits of {@code srcData} are ignored.
     * <p>
     * This method may be invoked at any time. If another thread has already initiated a read or write
     * operation upon this slave device, however, then an invocation of this method will block until
     * the first operation is complete.
     * </p>
     *
     * @param srcData
     *            the byte to be written.
     * @throws IOException
     *             if an I/O error occurred.
     * @throws UnavailableDeviceException
     *             if this device is not currently available - such as it is locked by another
     *             application.
     * @throws ClosedDeviceException
     *             if some other I/O error occurs (e.g. a <em>NACK</em> from the slave device was received).
     */
    void write(int srcData) throws IOException, UnavailableDeviceException, ClosedDeviceException;

    /**
     * Writes a sequence of bytes to a subaddress or register address of this slave device from the
     * given buffer. The most significant bytes (MSB) of the subaddress or register address are
     * transferred first.
     * <p>
     * An attempt is made to write up to <i>r</i> bytes to the device, where <i>r</i> is the number
     * of bytes remaining in the buffer, that is, {@code src.remaining()}, at the moment this method
     * is invoked.
     * </p><p>
     * Suppose that a byte sequence of length <i>n</i> is written, where <i>{@code 0 <= n <= r}</i>.
     * This byte sequence will be transferred from the buffer starting at index <i>p</i>, where
     * <i>p</i> is the buffer's position at the moment this method is invoked; the index of the last
     * byte written will be <i>{@code p + n - 1}</i>. Upon return the buffer's position will be
     * equal to <i>{@code p + n}</i>; its limit will not have changed.
     * </p><p>
     * A write operation will return only after writing all of the <i>r</i> requested bytes.
     * </p><p>
     * This method may be invoked at any time. If another thread has already initiated a read or write
     * operation upon this slave device, however, then an invocation of this method will block until
     * the first operation is complete.
     * </p>
     *
     * @param subaddress
     *            the slave device's subaddress or register address where to start writing.
     * @param subaddressSize
     *            the slave device's subaddress or register address size (1-4 bytes).
     * @param src
     *            the buffer from which bytes are to be retrieved.
     * @return the number of bytes written from {@code src}, possibly zero.
     * @throws NullPointerException
     *             if {@code src} is {@code null}.
     * @throws UnavailableDeviceException
     *             if this device is not currently available - such as it is locked by another
     *             application.
     * @throws ClosedDeviceException
     *             if the device has been closed.
     * @throws IOException
     *             if some other I/O error occurs (e.g. a <em>NACK</em> from the slave device was received).
     * @throws IllegalArgumentException
     *             if {@code subaddress} is negative or {@code subaddressSize} is not between
     *             {@code 1} and {@code 4}.
     */
    int write(int subaddress, int subaddressSize, ByteBuffer src) throws IOException, UnavailableDeviceException,
            ClosedDeviceException;
}
