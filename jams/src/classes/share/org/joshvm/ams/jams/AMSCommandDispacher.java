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

import java.io.IOException;
import com.sun.midp.log.*;

public class AMSCommandDispacher implements Runnable {
	AppManager appmanager;
		
	AMSCommandDispacher(AppManager appman) {
		appmanager = appman;
	}

	public void run() {
		try {
			while (!appmanager.isStopped()) {
			  try {
				AMSMessage server_cmd = appmanager.receiveMessage();
				AppManagerCommandListener cmdListener = appmanager.getCommandListener();
				if ((server_cmd != null) && (cmdListener != null)) {
					Logging.report(Logging.INFORMATION, LogChannels.LC_AMS, "Received message id:"+server_cmd.typeID());
					switch (server_cmd.typeID()) {
					case AMSMessage.MESSAGE_TYPE_ID_HEARTBEAT:
						cmdListener.commandHeartbeat(server_cmd.uniqueID());
						break;
					case AMSMessage.MESSAGE_TYPE_ID_DOWNLOAD_APP:
						if (appmanager.isConnected()) {
							String appname = parseAsString(server_cmd.body(), "APPNAME=");
							String installSource = parseAsString(server_cmd.body(), "APPURL=");
							String mainclass = parseAsString(server_cmd.body(), "MAINCLASS=");
							boolean autostart = parseAsBoolean(server_cmd.body(), "AUTOSTART=");
							boolean startAfterInstall = parseAsBoolean(server_cmd.body(), "STARTNOW=");
							int length = parseAsInt(server_cmd.body(), "APPLEN=");
							Logging.report(Logging.INFORMATION, LogChannels.LC_AMS, "installSource="+installSource+
							"mainclass="+mainclass+";appname="+appname+";autostart="+autostart+";startAfterInstall="+startAfterInstall+";length="+length);
							cmdListener.commandInstallApp(appmanager.isForceInstall(), server_cmd.uniqueID(), appname, mainclass, installSource, length, autostart, startAfterInstall);
						}
						break;
					case AMSMessage.MESSAGE_TYPE_ID_START_APP:
						if (appmanager.isConnected()) {
							cmdListener.commandStartApp(server_cmd.uniqueID(),
														parseAsString(server_cmd.body(), "APPNAME="),
														parseAsString(server_cmd.body(), "MAINCLASS="));
						}
						break;
					case AMSMessage.MESSAGE_TYPE_ID_LIST_APP:
						if (appmanager.isConnected()) {
							cmdListener.commandListInstalledApp(server_cmd.uniqueID());
						}
						break;
					case AMSMessage.MESSAGE_TYPE_ID_STOP_APP:
						if (appmanager.isConnected()) {
							cmdListener.commandStopApp(server_cmd.uniqueID());
						}
						break;
					case AMSMessage.MESSAGE_TYPE_ID_DELETE_APP:
						if (appmanager.isConnected()) {
							cmdListener.commandRemoveApp(server_cmd.uniqueID(), 
								                         parseAsString(server_cmd.body(), "APPNAME="));
						}
						break;
					case AMSMessage.MESSAGE_TYPE_ID_RUNNING_APP:
						if (appmanager.isConnected()) {
							cmdListener.commandListRunningApp(server_cmd.uniqueID());
						}
						break;
					case AMSMessage.MESSAGE_TYPE_ID_RESET_JVM:
						if (appmanager.isConnected()) {
							cmdListener.commandResetJVM(server_cmd.uniqueID());
						}
						break;
					case AMSMessage.MESSAGE_TYPE_ID_DOWNLOAD_REG:
						if (appmanager.isConnected()) {
							String installSource = parseAsString(server_cmd.body(), "REGURL=");
							int length = parseAsInt(server_cmd.body(), "REGLEN=");
							cmdListener.commandInstallReg(appmanager.isForceInstall(), server_cmd.uniqueID(), installSource, length);
						}
						break;
					case AMSMessage.MESSAGE_TYPE_ID_DELETE_REG:
						if (appmanager.isConnected()) {
							cmdListener.commandRemoveReg(server_cmd.uniqueID());
						}
						break;
					case AMSMessage.MESSAGE_TYPE_ID_SET_SYSTIME:
						if (appmanager.isConnected()) {
							int newTime = parseAsInt(server_cmd.body(), "UTCSEC=");
							cmdListener.commandSetSysTime(server_cmd.uniqueID(), (long)newTime*1000);
						}
						break;
					default:
						Logging.report(Logging.WARNING, LogChannels.LC_AMS, "AMS received unknown message: "+server_cmd.type());
					}
				}
			  } catch (WrongMessageFormatException wmfe) {
			  	wmfe.printStackTrace();
			  }
			}
		} catch (Exception e) {
			//e.printStackTrace();
			Logging.report(Logging.INFORMATION, LogChannels.LC_AMS, "Exception caught in AMS Message Dispatcher Loop: "+e.toString());
		}

		try {
			appmanager.disconnect();
		} catch (IOException ioe) {
			ioe.printStackTrace();
		}
	}

	private String parseAsString(String messageBody, String key) throws WrongMessageFormatException {
		int idx = messageBody.indexOf(key);
		if (idx != -1) {
			int begin = idx + key.length();
			int end = messageBody.indexOf(",", begin);
			if (end != -1) {
				return messageBody.substring(begin,end);
			} else {
				return messageBody.substring(begin);
			}
		} else {
			throw new WrongMessageFormatException("No key " + key + "found");
		}
	}

	private boolean parseAsBoolean(String messageBody, String key) throws WrongMessageFormatException {
		String val = parseAsString(messageBody, key);
		if (val.charAt(0) == (char)'0') {
			return false;
		} else if (val.charAt(0) == (char)'1') {
			return true;
		} else {
			throw new WrongMessageFormatException("Invalid boolean value " + val);
		}
	}

	private int parseAsInt(String messageBody, String key) throws WrongMessageFormatException {
		String val = parseAsString(messageBody, key);
		try {
			return Integer.parseInt(val);
		} catch (NumberFormatException e) {
			throw new WrongMessageFormatException("Invalid int value " + val);
		}
	}
}

