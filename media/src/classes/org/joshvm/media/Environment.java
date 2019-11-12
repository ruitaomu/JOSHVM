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

/**
 * Provides access to environment variables for media.
 */
public class Environment {
	/**
	 * Returns root path of internal storage file system used by media
	 * 
	 * @return root path which can be accessed by FileConnection interface
	 */
	public static String getRootDirectory() {
		return getRootDir0();
	}

	/**
	 * Returns root path of external storage file system
	 * 
	 * @return root path which can be accessed by FileConnection interface
	 */
	public static String getExternalStorageDirectory() {
		return getExtRootDir0();
	}

	/**
	 * Returns external storage state
	 * 
	 * @return true if ready, false otherwise.
	 */
	public static boolean getExternalStorageState() {
		return getExtStorageState0();
	}

	private static native String getRootDir0();

	private static native String getExtRootDir0();

	private static native boolean getExtStorageState0();
}
