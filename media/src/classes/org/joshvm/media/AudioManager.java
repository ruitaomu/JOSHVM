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
package org.joshvm.media;

import java.io.IOException;

/**
 * AudioManager provides access to volume control.
 */
public class AudioManager {
	/**
	 * Returns the maximum volume. (normally is 100)
	 */
	public static int getMaxVolume() {
		return getMaxVolume0();
	}

	/**
	 * Returns the current volume between 0 and maximum volume.
	 *
	 * @see #getMaxVolume
	 */
	public static int getCurrentVolume() {
		return getCurrentVolume0();
	}

	/**
	 * Set the current volume with value between 0 and maximum volume.
	 *
	 * @param volume the new volume
	 * @throws IOException if an I/O error occurs.
	 */
	public static void setVolume(int volume) throws IOException {
		setVolume0(volume);
	}

	private static native int getMaxVolume0();

	private static native int getCurrentVolume0();

	private static native void setVolume0(int volume);
}
