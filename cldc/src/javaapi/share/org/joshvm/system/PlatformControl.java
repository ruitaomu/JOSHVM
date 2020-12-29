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

package org.joshvm.system;

import org.joshvm.security.internal.*;

public final class PlatformControl {
	private SecurityToken securityToken;

	private PlatformControl() {
	};

	public PlatformControl(SecurityToken token) {
		securityToken = token;
	}

	public void reset() {
		securityToken.checkIfPermissionAllowed(Permissions.AMS);
		reset0();
	}

	public boolean setCurrentTimeMillis(long ms) {
		securityToken.checkIfPermissionAllowed(Permissions.AMS);
		if (ms < 0) {
			return false;
		}
		return setSystemTime(ms);
	}

	private native void reset0();
	private native boolean setSystemTime(long ms);
}
