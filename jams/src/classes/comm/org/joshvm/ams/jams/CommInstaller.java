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
package org.joshvm.ams.jams;

import java.io.*;
import javax.microedition.io.*;
import org.joshvm.security.internal.SecurityToken;
import org.joshvm.ams.consoleams.ConsoleConnection;

class CommInstaller extends Installer {
	protected InputStream server_is;

	public CommInstaller() {
		server_is = null;
	}

	protected  InputStream getSourceStream() {
		return server_is;
	}

	public void install(int type, String appName, String mainClass, int length, boolean autoStart) throws IOException, InstallVerifyErrorException {
		StreamConnection sc = null;
		final ConsoleConnection console = new ConsoleConnection();;

		try {
			/***
			 * TODO: BUG! Comm port will be open twice here! Fix required
			 *
			 */
			sc = (StreamConnection)Connector.open(installSource+";baudrate=115200;blocking=off");
			OutputStream os = sc.openOutputStream();
			InputStream is = sc.openInputStream();

			console.open(os, is);
			console.sendRequest(org.joshvm.ams.consoleams.ams.REQUEST_START_DOWNLOAD);

			server_is = is;

			super.install(type, appName, mainClass, length, autoStart, new AMSCallback() {
				void callback (int leftBytes) throws AMSException {
					try {
						console.sendRequest(org.joshvm.ams.consoleams.ams.REQUEST_DOWNLOAD_APP_NEXT_BLOCK);
					} catch (Exception e) {
						throw new AMSException("Send request error when downloading file");
					}
				}
			});
		} finally {
			if (console != null) {
				console.close();
			}

			if (server_is != null) {
				server_is.close();
			}
			if (sc != null) {
				sc.close();
			}
		}
	}
}
