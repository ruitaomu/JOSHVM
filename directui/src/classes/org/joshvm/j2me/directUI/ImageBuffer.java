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

public class ImageBuffer {
	public static final int TYPE_RGB565 = 1;
	private static final int bytes_per_pixel = 2;

	private int width;
	private int height;

	private byte imageData[];

	private void createImageBuffer(int w, int h, int type, byte[] imageData) throws IllegalArgumentException {
		if ((w <= 0) || (h <= 0)) {
			throw new IllegalArgumentException("Wrong width or height of ImageBuffer");
		}

		if (type != TYPE_RGB565) {
			throw new IllegalArgumentException("Unsupported type: "+type);
		}

		width = w;
		height = h;

		if (imageData != null) {
			setImageData(imageData);
		} else {
			setImageData(new byte[w*h*bytes_per_pixel]);
		}
	}

	public ImageBuffer(int w, int h, int type) throws IllegalArgumentException {
		createImageBuffer(w, h, type, null);
	}

	public ImageBuffer(int w, int h, int type, byte[] imageData) throws IllegalArgumentException {
		createImageBuffer(w, h, type, imageData);
	}

	public void setImageData(byte[] imageData)  throws IllegalArgumentException {
		if (width*height*bytes_per_pixel != imageData.length) {
			throw new IllegalArgumentException("imageData length is mismatched with width and height");
		}

		this.imageData = imageData;
	}

	public void setRGB(int rgb) throws IllegalArgumentException {
		setRGB(0, width - 1, 0, height - 1, rgb);
	}

	public void setRGB(int col, int row, int rgb) throws IllegalArgumentException {
		setRGB(col, col, row, row, rgb);
	}

	public void setRGB(int begincol, int endcol,
			int beginrow, int endrow, int rgb) throws IllegalArgumentException {
		if ((begincol < 0 || begincol >= width) ||
			(endcol < 0 || endcol >= width) ||
			(beginrow < 0 || beginrow >= height) ||
			(endrow < 0 || endrow >= height)) {
			throw new IllegalArgumentException("col/row is out of range:("
				+ begincol + "," + beginrow + "->" + endcol + "," + endrow + ")");
		}

		int b = (rgb & 0xff);
		int g = (rgb & 0xff00) >> 8;
		int r = (rgb & 0xff0000) >> 16;
		byte colorL = (byte)(((b >> 3) | ((g & 0xfc) << 3)) & 0xff);
		byte colorH = (byte)((g >> 5) | (r & 0xf8) & 0xff);
		for (int col =  begincol; col <= endcol; col++) {
			for (int row = beginrow; row <= endrow; row++) {
				imageData[(row*width+col)*bytes_per_pixel] = colorL;
				imageData[(row*width+col)*bytes_per_pixel+1] = colorH;
			}
		}
	}

	public byte[] getImageData() {
		return imageData;
	}

	public int getWidth() {
		return width;
	}

	public int getHeight() {
		return height;
	}
}
