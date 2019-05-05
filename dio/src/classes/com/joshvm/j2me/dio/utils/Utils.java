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
 * Copyright (c) 2014, 2015,  Oracle and/or its affiliates. All rights reserved.
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

package com.joshvm.j2me.dio.utils;

import org.joshvm.j2me.dio.DeviceConfig;

/**
 * Utility class for device permission class
 */
public final class Utils {

    public static final int EMPTY = 0;
    public static final int DECIMAL_DIGITS = 1;
    public static final int HEXADECIMAL_DIGITS = 2;
    public static final int HEXADECIMAL_DIGITS_INTERVAL = 3;

    /**
     * Checks if given value is zero or positive
     *
     */
    public static void checkIntZeroOrPozitive(final int val) {
        if (DeviceConfig.UNASSIGNED >= val) {
            throw new IllegalArgumentException(Integer.toString(val));
        }
    }

    /**
     * Checks if given value is UNASSIGNED or zero or positive
     *
     */
    public static void checkIntValue(final int val) {
        if (DeviceConfig.UNASSIGNED > val) {
            throw new IllegalArgumentException(Integer.toString(val));
        }
    }

    /**
     * Checks if given value is greater than 0 or UNASSIGNED
     *
     */
    public static void checkGreaterThanZero(final int val) {
        if (DeviceConfig.UNASSIGNED != val && val < 1){
            throw new IllegalArgumentException(Integer.toString(val));
        }
    }

    public static String[] parseDevicePermissionName(String name) {
        name = name.trim();
        String[] ret = new String[2];
        int idx = -1;
        while (-1 != (idx = name.indexOf(':', idx + 1))) {
            if (idx == 0 || '\\' != name.charAt(idx - 1)) {
                break;
            }
        }
        if (-1 == idx) {
            ret[0] = name;
            ret[1] = "";

        } else {
            ret[0] = name.substring(0, idx);
            ret[1] = name.substring(idx + 1);
        }
        return ret;
    }

    // type 0 - decimal digit
    // type 1 - hexadecimal digit
    // any other type - check empty string
    public static void checkDevicePermissionChannelFormat(String name, int type) {

        String channel = parseDevicePermissionName(name)[1];

        if (channel.equals("*") || channel.equals(""))
            return;

        switch(type) {
            //decimal digit
            case DECIMAL_DIGITS:
               if (!isNonNegativeNumber(channel, 10)) {
                    throw new IllegalArgumentException();
                }
                break;
            //hexadecimal digit
            case HEXADECIMAL_DIGITS:
                if (!isNonNegativeNumber(channel, 16)) {
                    throw new IllegalArgumentException();
                }
                break;
            case HEXADECIMAL_DIGITS_INTERVAL:
                if (!isNonNegativeNumberInterval(channel)) {
                    throw new IllegalArgumentException();
                }
                break;
            default:
               throw new IllegalArgumentException(name);
        }

    }

    /** Checks new requested scaled value range e.g. sampling interval range, pulse period range */
    public static void checkNewScaledRange(final double min_er,
                                        final double max_er,
                                        final double new_factor
                                        ) {
        // new scaled ones
        final double min_sr = min_er * new_factor;
        final double max_sr = max_er * new_factor;

        if (min_sr < 1.0d || max_sr > new Integer(Integer.MAX_VALUE).doubleValue())
            throw new IllegalArgumentException();
    }

    // Check all characters in the string are decimal digits
    private static boolean isNonNegativeNumber(String str, int radix) {
        try {
           return Integer.parseInt(str, radix) >= 0;
        } catch (NumberFormatException e) {
            return false;
        }
    }

    // Check the string has format {hexadecimal}-{hexadecimal}
    private static boolean isNonNegativeNumberInterval(String str) {
        int len = str.length();
        int idx = str.indexOf('-');

        if (-1 == idx || (len - 1) <= idx) {
            return false;
        }
        try {
            return Long.parseLong(str.substring(0, idx), 16) >=0
                && Integer.parseInt(str.substring(idx + 1), 16) >= 0;
        } catch (NumberFormatException e) {
            return false;
        }
    }

}
