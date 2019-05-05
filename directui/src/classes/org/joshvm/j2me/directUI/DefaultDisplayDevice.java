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

public class DefaultDisplayDevice implements DisplayDeviceAdaptor {

    public int getDisplayWidth() {
        return getDisplayWidth0();
    }

    public int getDisplayHeight() {
        return getDisplayHeight0();
    }

    public int getColorMode() {
        return COLOR_MODE_RGB565;
    }

    public void clear(int rgb) {
        clear0(rgb);
    }

    public void update(int top_left_x, int top_left_y, ImageBuffer imageBuffer, boolean delayshow) {
        if (imageBuffer == null) {
            throw new IllegalArgumentException();
        }
        update0(top_left_x, top_left_y, imageBuffer.getImageData(),
                imageBuffer.getWidth(), imageBuffer.getHeight(), delayshow);
    }

    public void showImage(int top_left_x, int top_left_y, Image image, boolean delayshow) {
        if (image == null || image.getImageData() == null) {
            return;
        }
        showImage0(top_left_x, top_left_y, image.getImageData(),
                image.getImageType(), delayshow);
    }

    private void showImage0(int x, int y, byte[] data, int type, boolean delayshow) {
        showImage0(x, y, data, 0, data.length, type, delayshow);
    }

    public void showText(int top_left_x, int top_left_y, Text text, boolean delayshow) {
        if (text == null || text.getString() == null) {
            return;
        }
        showText0(top_left_x, top_left_y, text.getString(), text.getColor(), text.getFont(), delayshow);
    }

    public void flush() {
        flush0();
    }

    public void turnOnBacklight() {
        setBacklight0(true);
    }

    public void turnOffBacklight() {
        setBacklight0(false);
    }

    private native int getDisplayWidth0();
    private native int getDisplayHeight0();
    private native void clear0(int rgb);
    private native void update0(int x, int y, byte[] data, int w, int h, boolean delayed);
    private native void showImage0(int x, int y, byte[] data, int offset, int len, int type, boolean delayed);
    private native void showText0(int x, int y, String str, int rgb, int font, boolean delayed);
    private native void flush0();
    private native void setBacklight0(boolean on);
}
