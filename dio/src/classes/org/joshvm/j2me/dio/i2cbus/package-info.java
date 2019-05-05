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
/**
 * Interfaces and classes for I2C (Inter-Integrated Circuit Bus) device access.
 * <p>
 * The functionalities supported by this API are those of an I2C master.
 * </p><p>
 * In order to communicate with a specific slave device, an application should first open and obtain
 * an {@link org.joshvm.j2me.dio.i2cbus.I2CDevice} instance for the I2C slave device the
 * application wants to exchange data with, using its numeric ID, name, type (interface) and/or
 * properties:
 * <blockquote>
 * <dl>
 * <dt>Using its ID</dt>
 * <dd><blockquote>
 * <pre>
 * I2CDevice slave = (I2CDevice) DeviceManager.open(3);
 * </pre>
 * </blockquote></dd>
 * <dt>Using its name and interface</dt>
 * <dd><blockquote>
 * <pre>
 * I2CDevice slave = DeviceManager.open(&quot;ADC1&quot;, I2CDevice.class, null);
 * </pre>
 * </blockquote></dd>
 * </dl>
 * </blockquote>
 * Once the device opened, the application can exchange data with the I2C slave device using
 * methods of the {@link org.joshvm.j2me.dio.i2cbus.I2CDevice} interface such as the
 * {@link org.joshvm.j2me.dio.i2cbus.I2CDevice#write(ByteBuffer) write} method. <blockquote>
 * <pre>
 * slave.write(sndBuf, 0, 1);
 * </pre>
 * </blockquote> When the data exchange is over, the application should call the
 * {@link org.joshvm.j2me.dio.i2cbus.I2CDevice#close I2CDevice.close} method to close the I2C
 * slave device. <blockquote>
 * <pre>
 * slave.close();
 * </pre>
 * </blockquote> The following sample code gives an example of using the I2C API to communicate with
 * an I2C slave device: <blockquote>
 * <pre>
 * try (I2CDevice slave = DeviceManager.open(&quot;LED_CONTROLLER&quot;, I2CDevice.class, null)) {
 *     ByteBuffer stopCmd = ByteBuffer.wrap(LED_STOP_COMMAND);
 *     ByteBuffer offCmd = ByteBuffer.wrap(LED_OFF_COMMAND);
 *     ByteBuffer onCmd = ByteBuffer.wrap(LED_ON_COMMAND);
 *     <i>// Clear all status of the 'LED' slave device</i>
 *     slave.write(ByteBuffer.wrap(stopCmd));
 *     slave.write(ByteBuffer.wrap(offCmd));
 *
 *     for (int i = 0; i &lt; LED_LOOP_COUNT; i++) {
 *         <i>// turning 'LED' on and keeping it on for 1500ms</i>
 *         slave.write(ByteBuffer.wrap(onCmd));
 *         try {
 *             Thread.sleep(LED_BLINK_TIME);
 *         } catch (InterruptedException ex) {
 *         }
 *
 *         <i>// turning 'LED' off keeping it off for 1500ms</i>
 *         slave.write(ByteBuffer.wrap(offCmd));
 *         try {
 *             Thread.sleep(LED_BLINK_TIME);
 *         } catch (InterruptedException ex) {
 *         }
 *     }
 * } catch (IOException ioe) {
 *     <i>// handle exception</i>
 * }
 * </pre>
 * </blockquote>
 * <p>
 * The preceding example is using a <em>try-with-resources</em> statement;
 * the {@link org.joshvm.j2me.dio.i2cbus.I2CDevice#close I2CDevice.close} method is
 * automatically invoked by the platform at the end of the statement.
 * </p><p>
 * Information about the I2C-bus specification can be found at <a
 * href="http://www.nxp.com/documents/user_manual/UM10204.pdf"
 * >http://www.nxp.com/documents/user_manual/UM10204.pdf</a>.
 * </p><p>
 * Unless otherwise noted, permission and security checks that may cause
 * a {@link java.lang.SecurityException SecurityException} to be thrown must be performed
 * in priority to any other checks or operations once performed the checking of the input parameters
 * from which the permission target names and action lists are retrieved and assembled.
 * </p><p>
 * Unless otherwise noted, passing a {@code null} argument to a constructor or method in any class
 * or interface in this package will cause a {@link java.lang.NullPointerException
 * NullPointerException} to be thrown.
 * </p>
 *
 * @since 1.0
 */
package org.joshvm.j2me.dio.i2cbus;

