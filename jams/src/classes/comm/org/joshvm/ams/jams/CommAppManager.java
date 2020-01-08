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
import org.joshvm.ams.consoleams.ConnectionResetException;
import com.sun.midp.log.*;

final class CommAppManager extends org.joshvm.ams.consoleams.ams implements AppManager {
	protected boolean stop;
	protected AppManagerCommandListener listener;
	private boolean appStarted;
	private String name;

	public CommAppManager() {
		super();
	}
	
	public void init(String port) {
		super.init(port);
		listener = null;
		connected = false;
		stop = false;
		appStarted = false;
		name = port;
	}

	public void connect() throws IOException {
		System.out.println("========CommAppManager.connect");
		stop = false;
		super.connect();		
		startCommandListener();
	}
	public boolean isConnected() {
		return connected;
	}
	public boolean isStopped() {
		return stop;
	}
	public boolean isForceInstall() {
		return true;
	}

	public synchronized void disconnect() throws IOException {
		notifyAll();

		System.out.println("========CommAppManager.disconnect: stop="+stop);

		if (stop == false) {
			Logging.report(Logging.INFORMATION, LogChannels.LC_AMS, "CommAppManager disconnecting...");
		
			stop = true;
			super.disconnect();
		}
	}

	public void notifyConnected() {
	}

	public String getAppdbNativeRoot() {
		String path = System.getProperty("org.joshvm.ams.appdbpath.native_root.unsecure");

		if (path == null) {
			Logging.report(Logging.INFORMATION, LogChannels.LC_AMS, "getAppdbNativeRoot use default");
			path = new String("C:\\java\\internal\\appdb\\unsecure\\");
		}
		return path;
	}

	public String getAppdbRoot() {
		String path = System.getProperty("org.joshvm.ams.appdbpath.root.unsecure");
		if (path == null) {
			Logging.report(Logging.INFORMATION, LogChannels.LC_AMS, "getAppdbRoot use default");
			path = new String("/internal/appdb/unsecure/");
		}
		return path;
	}
	public void response(String uniqueid, int code) throws IOException {
		response(uniqueid, code, null);
	}

	public void response(String uniqueid, int code, String body) throws IOException {
		switch (code) {
			case AppManager.APPMAN_RESPCODE_INSTALLOK:
				console.sendReport(REPORT_FINISH_DOWNLOAD);
				break;
			case AppManager.APPMAN_RESPCODE_INSTALLFAIL:
				console.sendReport(REPORT_FAIL_DOWNLOAD);
				break;
			case AppManager.APPMAN_RESPCODE_APPLIST:
				System.out.println(body);
				console.sendReport(REPORT_FINISH_LISTAPP);
				break;
			case AppManager.APPMAN_RESPCODE_DELETEOK:
				console.sendReport(REPORT_FINISH_ERASEAPP);
				break;
			case AppManager.APPMAN_RESPCODE_APPFINISH:				
				console.sendReport(REPORT_FINISH_RUNAPP);
				appStarted = false;
				try {
					synchronized (this) {
						notifyAll();
					}
				} catch (Exception e) {
					e.printStackTrace();
				}
				break;
			case AppManager.APPMAN_RESPCODE_APPSTARTOK:
				appStarted = true;
				break;
			default:
				console.sendReport(REPORT_GENERAL_FAIL);
		}
	}
	public AMSMessage receiveMessage() throws IOException, WrongMessageFormatException {
		AMSMessage msg = null;

		try {
			int command = waitForCommand();

			switch (command) {
				case COMMAND_DOWNLOAD_JAR:
					msg = downloadInfo();
					break;
				case COMMAND_RUN_APP:
					msg = runningInfo();
					break;
				case COMMAND_LIST_APP:
					msg = appListInfo();
					break;
				case COMMAND_ERASE_APP:
					msg = removeAppInfo();
					break;
				case COMMAND_STOP_APP:
					break;
				case COMMAND_NO_MORE_COMMAND:
					synchronized (this) {
						wait();
					}
					break;
				case COMMAND_DOWNLOAD_REG:
					msg = downloadRegInfo();
					break;
				case COMMAND_ERASE_REG:
					msg = removeRegInfo();
					break;
				case COMMAND_SET_SYSTIME:
					msg = setSysTime();
					break;
				default:
			}
		} catch (ConnectionResetException e) {
			if (!appStarted) {
				throw new IOException(e.toString());
			}
		} catch (InterruptedException ie) {
		}

		return msg;
	}

	public void setCommandListener(AppManagerCommandListener listener) {
		this.listener = listener;
	}

	public AppManagerCommandListener getCommandListener() {
		return listener;
	}

	private void startCommandListener() {
		new Thread(new AMSCommandDispacher(this)).start();
	}

	private AMSMessage downloadInfo() throws IOException, ConnectionResetException, WrongMessageFormatException {
		String appJarFile = getDownloadAppName();
		int filebytes = getDownloadAppLength();

		return new AMSMessage("[DOWNLAPP]UNIQUEID=0000000000000000,APPNAME="+appJarFile+
			",APPURL=comm:COM0,AUTOSTART=0,STARTNOW=0,MAINCLASS=dummy,APPLEN="+filebytes);
	}

	private AMSMessage appListInfo() throws IOException, ConnectionResetException, WrongMessageFormatException {
		return new AMSMessage("[RLISTAPP]UNIQUEID=0000000000000000");
	}

	private AMSMessage removeAppInfo() throws IOException, ConnectionResetException, WrongMessageFormatException {
		String appJarFile = getDownloadAppName();

		return new AMSMessage("[RDELEAPP]UNIQUEID=0000000000000000,APPNAME="+appJarFile);
	}

	private AMSMessage runningInfo() throws IOException, ConnectionResetException, WrongMessageFormatException {
		String appJarFile = getRunAppJarfile();
		String appName = getRunAppMainClass();

		return new AMSMessage("[STARTAPP]UNIQUEID=0000000000000000,APPNAME="+appJarFile+
			", MAINCLASS="+appName);
	}

	private AMSMessage downloadRegInfo() throws IOException, ConnectionResetException, WrongMessageFormatException {
		int filebytes = getDownloadAppLength();

		return new AMSMessage("[DOWNLREG]UNIQUEID=0000000000000000,REGURL=comm:COM0,REGLEN="+filebytes);
	}

	private AMSMessage removeRegInfo() throws IOException, ConnectionResetException, WrongMessageFormatException {
		return new AMSMessage("[RDELEREG]UNIQUEID=0000000000000000");
	}

	private AMSMessage setSysTime() throws IOException, ConnectionResetException, WrongMessageFormatException {
		int time = getSysTimeUTCSecond();
		return new AMSMessage("[SETSYSTM]UNIQUEID=0000000000000000,UTCSEC="+time);
	}

	public String toString() {
		return name;
	}

}
