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
package org.joshvm.ams.consoleams;

import javax.microedition.io.*;
import javax.microedition.io.file.*;
import com.sun.cldc.isolate.*;
import java.io.*;
import com.sun.midp.log.*;

public class ams {
	/*REQUEST CODE*/
	static final public int REQUEST_SYNC = 0;
	static final public int REQUEST_WAIT_COMMAND = 1;
	static final public int REQUEST_DOWNLOAD_APP_FILENAME = 2;
	static final public int REQUEST_DOWNLOAD_APP_FILELENGTH = 3;
	static final public int REQUEST_RUN_APP_FILENAME = 4;
	static final public int REQUEST_RUN_APP_MAINCLASS = 5;
	static final public int REQUEST_START_DOWNLOAD = 6;
	static final public int REQUEST_DOWNLOAD_APP_NEXT_BLOCK = 7;



	static final public int REQUEST_SET_SYSTIME = 11;
	static final public int REQUEST_NEED_SYSTIME = 12;
	/*COMMAND CODE*/
	static final public int COMMAND_DOWNLOAD_JAR = 30;
	static final public int 	COMMAND_RUN_APP = 31;
	static final public int 	COMMAND_LIST_APP = 32;
	static final public int 	COMMAND_STOP_APP = 33;
	static final public int COMMAND_ERASE_APP = 35;
	static final public int COMMAND_NO_MORE_COMMAND = 36;


	
	static final public int COMMAND_SET_SYSTIME = 40;

	static final public int COMMAND_DOWNLOAD_REG = 50;
	static final public int COMMAND_ERASE_REG = 51;
	/*REPORT CODE*/
	static final public int REPORT_FINISH_DOWNLOAD = 60;
	static final public int REPORT_FINISH_LISTAPP = 61;
	static final public int REPORT_FINISH_RUNAPP = 62;
	static final public int REPORT_FINISH_ERASEAPP = 63;
	/*ERROR REPORT CODE*/
	static final public int REPORT_FAIL_DOWNLOAD = 100;
	static final public int REPORT_GENERAL_FAIL = 255;

	static final public int SYNC_CODE = 0x0c0ffee0;

	StreamConnection sc;
	OutputStream com_os;
	InputStream com_is;
	String commport;

	protected boolean connected;
	protected ConsoleConnection console;

	private static String appdbPath;
	private static String appdbNativePath;

	protected ams(String comm_port) {
		connected = false;
		console = new ConsoleConnection();
		commport = comm_port;
	}

	protected ams() {
		connected = false;
		console = new ConsoleConnection();
		commport = "COM0";
	}

	protected void init(String port) {
		commport = port;
	}

	protected String getRunAppJarfile() throws IOException, ConnectionResetException {
		console.sendRequest(REQUEST_RUN_APP_FILENAME);
		return console.receiveString();
	}

	protected String getRunAppMainClass() throws IOException, ConnectionResetException {
		console.sendRequest(REQUEST_RUN_APP_MAINCLASS);
		return console.receiveString();
	}

	private void runApp()  throws IOException, ConnectionResetException {
		Isolate iso;
		String appName;
		String appJarFile;
		String[] appArgs = new String[0];

		if (!connected) {
			return;
		}

		appJarFile = getRunAppJarfile();
		appName = getRunAppMainClass();

		try {
			disconnect();
			Logging.report(Logging.INFORMATION, LogChannels.LC_AMS, "New Isolate: "+appName+","+appJarFile);
			iso = new Isolate(appName, appArgs, new String[]{appdbNativePath+appJarFile});
			Logging.report(Logging.INFORMATION, LogChannels.LC_AMS, "New Isolate: "+appName+" about to start");
			iso.start();
			Logging.report(Logging.INFORMATION, LogChannels.LC_AMS, "New Isolate: "+appName+" started, waiting for exit...");
			iso.waitForExit();
			Logging.report(Logging.INFORMATION, LogChannels.LC_AMS, "New Isolate: "+appName+" exit successfully with code:"+iso.exitCode());
			if (iso.exitCode() < 0) {
			}
		} catch (Exception IsolateStartupException) {
			Logging.report(Logging.CRITICAL, LogChannels.LC_AMS, "IsolateStartupException when starting to run " + appName);
		}
	}

	protected String getDownloadAppName() throws IOException, ConnectionResetException {
		console.sendRequest(REQUEST_DOWNLOAD_APP_FILENAME);
		return console.receiveString();
	}

	protected int getDownloadAppLength() throws IOException, ConnectionResetException {
		console.sendRequest(REQUEST_DOWNLOAD_APP_FILELENGTH);
		return console.receiveInt();
	}

	protected int getSysTimeUTCSecond() throws IOException, ConnectionResetException {
		console.sendRequest(REQUEST_SET_SYSTIME);
		return console.receiveInt();
	}

	protected int waitForCommand() throws IOException, ConnectionResetException {
		console.sendRequest(REQUEST_WAIT_COMMAND);
		Logging.report(Logging.INFORMATION, LogChannels.LC_AMS, "Waiting for command");
		return console.receiveInt();
	}

	private void downloadApp() throws IOException, ConnectionResetException {
		if (!connected) {
			return;
		}

		String appJarFile = getDownloadAppName();
		int filebytes = getDownloadAppLength();

		FileConnection fconn = null;
		OutputStream file_os = null;
		Logging.report(Logging.INFORMATION, LogChannels.LC_AMS, "filename to download:"+appJarFile);
		Logging.report(Logging.INFORMATION, LogChannels.LC_AMS, "filebytes to download:"+filebytes);
		try {
			fconn = (FileConnection)Connector.open("file://"+appdbPath+appJarFile);
			if (fconn.exists()) {
				fconn.delete();
			}
			fconn.create();
			file_os = fconn.openOutputStream();
	        byte[] buf = new byte[filebytes];
			console.sendRequest(REQUEST_START_DOWNLOAD);
			int count = console.read(buf, 0, filebytes);
			if (filebytes != count) {
				Logging.report(Logging.ERROR, LogChannels.LC_AMS, "Download fail for wrong size");
				console.sendReport(REPORT_FAIL_DOWNLOAD);
			} else {
				file_os.write(buf, 0, count);
				Logging.report(Logging.INFORMATION, LogChannels.LC_AMS, "Download finish");
				console.sendReport(REPORT_FINISH_DOWNLOAD);
			}

		}
		finally {
			if (file_os != null)
				file_os.close();

			if (fconn != null)
				fconn.close();
		}


	}

	void sync() throws IOException {
		int code;
		do {
			console.sendRequest(REQUEST_SYNC);
			code = console.receiveInt();
		} while (code != SYNC_CODE);

		if (System.currentTimeMillis() < 630720000000L) {
			//Tell Josh Console the device need to set SYSTIME, but not require response, in case JOSH Console doesn't support this request
			console.sendRequest(REQUEST_NEED_SYSTIME);
		}
	}

	protected void connect() throws IOException {
		if (connected)
			return;

		sc = (StreamConnection)Connector.open("comm:"+commport+";baudrate=115200;blocking=off");
		com_os = sc.openOutputStream();
		com_is = sc.openInputStream();
		connected = true;
		console.open(com_os, com_is);
		Logging.report(Logging.INFORMATION, LogChannels.LC_AMS, "Proxy port open, now sync...");
		try {
			sync();
		} catch (IOException e) {
			Logging.report(Logging.ERROR, LogChannels.LC_AMS, e.getMessage());
			disconnect();
			throw e;
		}
		Logging.report(Logging.INFORMATION, LogChannels.LC_AMS, "Proxy sync ok");
	}

	protected void disconnect() throws IOException {
		if (!connected)
			return;

        com_is.close();
		com_os.close();
		sc.close();

		com_is = null;
		com_os = null;
		sc = null;
		connected = false;
		console.close();
	}
	
	public static int main(String argv[]) {
		if (argv.length < 3) {
			/*
			 * appdbPath and appdbNativePath must be a pair of same File System path, which is represented in Java and native.
			 * For example: cldc_vm org.joshvm.ams.consoleams.ams COM0 /javauserroot /home/javauser/joshuserroot
			 * Whereas file:///javauserroot should be mapped to native /home/javauser/joshuserroot
			 */
			Logging.report(Logging.CRITICAL, LogChannels.LC_AMS, "SimpleAMS must be used with comm port name, appdb Java path and native path as input parameters.");
			return -1;
		}

		String commport = argv[0];
		appdbPath = argv[1];
		appdbNativePath = argv[2];
		
		ams appman = new ams(commport);

		do {
			try {
				appman.connect();

				int command = appman.waitForCommand();
				switch (command) {
					case COMMAND_DOWNLOAD_JAR:
						appman.downloadApp();
						break;
					case COMMAND_RUN_APP:
						appman.runApp();
						break;
					case COMMAND_LIST_APP:
						break;
					case COMMAND_STOP_APP:
						break;
					default:
				}
			} catch (Exception e) {
				e.printStackTrace();
			}

		} while (true);
	}
}
