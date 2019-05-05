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

public class Text {
    public static final int FONT_DEFAULT = 0;
    public static final int FONT_SMALL = 1;
    public static final int FONT_MEDIUM = 2;
    public static final int FONT_LARGE = 3;

    private String data;
    private int rgb;
    private int font;

    public Text() {
        this(null);
    }

    public Text(String str) {
        setString(str);
    }

    public void setString(String str) {
        data = str;
    }

    public String getString() {
        return data;
    }

    public int getWidth() {
        return getWidth(data);
    }

    public int getWidth(String str) {
        if (str == null || str.length() == 0) {
            return 0;
        }
        return getWidth0(str, font);
    }

    public int getHeight() {
        if (data == null || data.length() == 0) {
            return 0;
        }
        return getHeight0(data, font);
    }

    public void setColor(int rgb) {
        this.rgb = rgb;
    }

    public int getColor() {
        return rgb;
    }

    public void setFont(int font) {
        this.font = font;
    }

    public int getFont() {
        return font;
    }

    private native int getWidth0(String str, int font);
    private native int getHeight0(String str, int font);
}
