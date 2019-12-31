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
package org.joshvm.ams.console;

public final class ConsoleCommand {
	private int type;
	private String message;
	public static final int NOTIFY_REMOTE_DISCONNECTED = 1;
	public static final int NOTIFY_REMOTE_CONNECTED = 2;
	
	public ConsoleCommand(int type, String message) {
		this.type = type;
		this.message = message;
	}
	
	public int getType() {
		return type;
	}
	
	public String getMessage() {
		return message;
	}

	public static ConsoleCommand getRemoteDisconnectedCommand() {
		return new ConsoleCommand(NOTIFY_REMOTE_DISCONNECTED, null);
	}

}
