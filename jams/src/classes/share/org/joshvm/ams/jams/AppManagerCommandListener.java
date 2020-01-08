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

public interface AppManagerCommandListener {
	public void commandInstallApp(boolean forceInstall, String uniqueID, String appName, String mainClass, String installSource, int length, boolean autoStart, boolean startAfterInstall);
	public void commandRemoveApp(String uniqueID, String appName);
	public void commandStopApp(String uniqueID);
	public void commandStartApp(String uniqueID, String appName, String mainClass);
	public void commandRestartApp(String uniqueID);
	public void commandListInstalledApp(String uniqueID);
	public void commandListRunningApp(String uniqueID);
	public void commandGetAppInfo(String uniqueID, String appName);
	public void commandHeartbeat(String uniqueID);
	public void commandResetJVM(String uniqueID);
	public void commandInstallReg(boolean forceInstall, String uniqueID, String installSource, int length);
	public void commandRemoveReg(String uniqueID);
	public void commandSetSysTime(String uniqueID, long milliSecondsToSet);
}
