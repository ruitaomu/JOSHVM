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
package org.joshvm.j2me.directUI;

import java.io.IOException;

/**
 * <code>Display</code> represents the primitive display device of the
 * system. It includes methods for showing {@link org.joshvm.j2me.directUI.Image Image} ,
 * {@link org.joshvm.j2me.directUI.ImageBuffer ImageBuffer} ,  {@link org.joshvm.j2me.directUI.Text Text} ,
 * and other primitive operations of display device, such as <code>flush</code> and <code>clear</code>. <p>
 *
 * There is exactly one default Display bound to one device; Or, user could get different Display devices by 
 * specifing user defined display adaptors. Usually, user defined display is the display device mounted on SPI 
 * or other kind of external I/O port. <p>
 *
 * Use {@link #getDisplay() getDisplay()} to get it. <p>
 *
 */

public class Display {
	public static final int COLOR_MODE_RGB565 = 1;
	public static final int COLOR_MODE_RGB666 = 2;
	public static final int COLOR_MODE_RGB888 = 3;
	public static final int COLOR_MODE_ARGB8888 = 4;

	private static Display theDisplay = null;
	private DisplayDeviceAdaptor device;

	/**
	 *
	 * Get the default display device.
	 *
	 */
	public static synchronized Display getDisplay() throws IOException {
		if (theDisplay != null) {
			return theDisplay;
		}

		DisplayDeviceAdaptor device;
		String drivername = System.getProperty("org.joshvm.directui.display.drivername");
		if (drivername != null) {
			Class clazz;
    	    try {
        	    clazz = Class.forName(drivername);
        	} catch (ClassNotFoundException exc) {
        		exc.printStackTrace();
				throw new IOException("Display device driver not found: "+drivername);
			}

			try {
				device = (DisplayDeviceAdaptor)clazz.newInstance();
			} catch (Exception e) {
				e.printStackTrace();
				throw new IOException("Can't open display device");
			}
		} else {
			device = new DefaultDisplayDevice();
		}

		return getDisplay(device);
	}

	/**
	 *
	 * Get the user defined display device.
	 *
	 * @see {@link org.joshvm.j2me.directUI.DisplayDeviceAdaptor DisplayDeviceAdaptor}
	 */
	public static synchronized Display getDisplay(DisplayDeviceAdaptor device) throws IOException {
		if (device == null) {
			throw new IllegalArgumentException();
		}
		theDisplay = new Display(device);
		return theDisplay;
	}

	private Display(DisplayDeviceAdaptor device) {
		this.device = device;
	}

	public int getDisplayWidth() {
		return device.getDisplayWidth();
	}

	public int getDisplayHeight() {
		return device.getDisplayHeight();
	}

	public int getColorMode() {
		return device.getColorMode();
	}

	public void clear() {
		clear(0xFFFFFF);
	}

	public void clear(int rgb) {
		device.clear(rgb);
	}

	public void showImageBuffer(int top_left_x, int top_left_y, ImageBuffer image) {
		showImageBuffer(top_left_x, top_left_y, image, false);
	}

	public void showImageBuffer(int top_left_x, int top_left_y, ImageBuffer image, boolean delayed) {
		device.update(top_left_x, top_left_y, image, delayed);
	}

	public void showImage(int top_left_x, int top_left_y, Image image) {
		showImage(top_left_x, top_left_y, image, false);
	}

	public void showImage(int top_left_x, int top_left_y, Image image, boolean delayed) {
		device.showImage(top_left_x, top_left_y, image, delayed);
	}

	public void showText(int top_left_x, int top_left_y, Text text) {
		showText(top_left_x, top_left_y, text, false);
	}

	public void showText(int top_left_x, int top_left_y, Text text, boolean delayed) {
		device.showText(top_left_x, top_left_y, text, delayed);
	}

	public void flush() {
		device.flush();
	}

	public void turnOnBacklight() {
		device.turnOnBacklight();
	}

	public void turnOffBacklight() {
		device.turnOffBacklight();
	}
}
