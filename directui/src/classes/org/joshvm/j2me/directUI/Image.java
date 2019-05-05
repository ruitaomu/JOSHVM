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

public class Image {
    public static final int IMAGE_TYPE_PNG = 0;
    public static final int IMAGE_TYPE_JPG = 1;
    public static final int IMAGE_TYPE_BMP = 2;

    private int imageType;
    private byte imageData[];

    public Image(int type) {
        if (type == IMAGE_TYPE_PNG ||
            type == IMAGE_TYPE_JPG ||
            type == IMAGE_TYPE_BMP) {
            imageType = type;
            return;
        }
        throw new IllegalArgumentException();
    }

    public int getImageType() {
        return imageType;
    }

    public void setImageData(byte[] imageData) {
        this.imageData = imageData;
    }

    public byte[] getImageData() {
        return imageData;
    }

    public int getWidth() {
        if (imageData == null || imageData.length == 0) {
            return 0;
        }
        return getWidth0(imageData, 0, imageData.length, imageType);
    }

    public int getHeight() {
        if (imageData == null || imageData.length == 0) {
            return 0;
        }
        return getHeight0(imageData, 0, imageData.length, imageType);
    }

    private native int getWidth0(byte[] data, int offset, int len, int type);
    private native int getHeight0(byte[] data, int offset, int len, int type);
}
