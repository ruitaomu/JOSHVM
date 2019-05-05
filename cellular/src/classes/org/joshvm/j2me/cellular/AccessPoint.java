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
package org.joshvm.j2me.cellular;

public class AccessPoint {
	private final String name;
	private final String username;
	private final String password;

	public AccessPoint(String apn_name) {
		this.name = apn_name;
		this.username = null;
		this.password = null;
	}

	public AccessPoint(String apn_name, String username, String password) {
		this.name = apn_name;
		this.username = username;
		this.password = password;
	}

	public String getName() {
		return name;
	}

	public String getUserName() {
		return username;
	}

	public String getPassword() {
		return password;
	}
}

