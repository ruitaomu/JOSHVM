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
package org.joshvm.j2me.dio;

public interface Device {
	static final int	LITTLE_ENDIAN	= 0;
	static final int	BIG_ENDIAN		= 1;
	static final int	MIXED_ENDIAN	= 2;

	void tryLock(int timeout)
             throws UnavailableDeviceException,
                    ClosedDeviceException,
                    java.io.IOException;

	void close() throws java.io.IOException;
	boolean isOpen();
	void unlock() throws java.io.IOException;
}

