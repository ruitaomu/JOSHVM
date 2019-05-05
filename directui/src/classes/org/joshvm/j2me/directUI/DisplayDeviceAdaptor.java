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

public interface DisplayDeviceAdaptor {
	public static final int COLOR_MODE_RGB565 = 1;
	public static final int COLOR_MODE_RGB666 = 2;
	public static final int COLOR_MODE_RGB888 = 3;
	public static final int COLOR_MODE_ARGB8888 = 4;

	public int getDisplayWidth();

	public int getDisplayHeight();

	public int getColorMode();

	public void clear(int rgb);

	public void update(int top_left_x, int top_left_y, ImageBuffer framebuffer, boolean delayshow);

	public void showImage(int top_left_x, int top_left_y, Image image, boolean delayshow);

	public void showText(int top_left_x, int top_left_y, Text text, boolean delayshow);

	public void flush();

	public void turnOnBacklight();

	public void turnOffBacklight();
}
