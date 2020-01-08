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
import javax.microedition.io.file.*;
import org.joshvm.security.internal.*;
import org.joshvm.system.PlatformControl;
import com.sun.cldc.io.j2me.file.Protocol;
import com.sun.cldc.isolate.*;
import com.sun.midp.log.*;

public final class Jams implements AppManagerCommandListener {
	private static final String APPMANAGER_SERVER_ADDRESS = "appman.joshvm.com";
	private static final String APPMANAGER_SERVER_PORT = "8765";
	private static final String APPMANAGER_SERVER_PATH = "";
	private static final String APPMANAGER_COMM_PORT = "COM0";
	private static final int APPMANAGER_TYPE_COMM = 1;
	private static final int APPMANAGER_TYPE_NETWORK = 2;
	private static final boolean ifConnectAtStart = false;

	private static final int REGFILE_MAX_SIZE = 1024;

	private static Isolate runningIsolate;
	private static String stoppingUniqueID;
	
	private static AppManager appman = null;
	private PlatformControl platformControl;

	private static Object startSyncObj = new Object();

    /**
     * Inner class to request security token from SecurityInitializer.
     * SecurityInitializer should be able to check this inner class name.
     */
    static private class SecurityTrusted
        implements ImplicitlyTrustedClass {}

    /** This class has a different security domain than the MIDlet suite */
    private static SecurityToken securityToken =
        SecurityInitializer.requestToken(new SecurityTrusted());
	
	private Jams (int type) throws UnsupportedAppManagerException {
		try {
			if (type == APPMANAGER_TYPE_COMM) {
				appman = (AppManager)Class.forName("org.joshvm.ams.jams.CommAppManager").newInstance();
				appman.init(APPMANAGER_COMM_PORT);
			}
			
			if (type == APPMANAGER_TYPE_NETWORK) {
				appman = (AppManager)Class.forName("org.joshvm.ams.jams.NetworkAppManager").newInstance();
				appman.init(APPMANAGER_SERVER_ADDRESS+":"+APPMANAGER_SERVER_PORT);
			}
			stoppingUniqueID = null;
			platformControl = new PlatformControl(securityToken);
		} catch (ClassNotFoundException cne) {
			throw new UnsupportedAppManagerException(cne.getMessage());
		} catch (InstantiationException ie) {
			throw new UnsupportedAppManagerException(ie.getMessage());
		} catch (IllegalAccessException iae) {
			throw new UnsupportedAppManagerException(iae.getMessage());
		}
	}

	private Installer getInstaller(String installSourceURL) {
		return Installer.getInstaller(securityToken, installSourceURL);
	}

	
	public static void main(String argv[]) {
		Jams ams;
		AppManager appman;

		try {
			//Try Comm App Manager
			ams = new Jams(APPMANAGER_TYPE_COMM);
			appman = ams.appman;
			appman.setCommandListener(ams);
			try {
				appman.connect();
			} catch (IOException e) {
				//Logging.report(Logging.WARNING, LogChannels.LC_AMS, "App Manager cannot connect to COMM port");
			}
		} catch (UnsupportedAppManagerException uame) {
			ams = null;
			appman = null;
			uame.printStackTrace();
		}

		if ((appman == null) || !appman.isConnected()) {
			if (appman != null) {
				appman.setCommandListener(null);			
			}

			try {
				ams = new Jams(APPMANAGER_TYPE_NETWORK);
				appman = ams.appman;
				
				//If Comm App Manager connected , auto-start application will not be auto-started
				//If Network App Manager is not supported, auto-start application will not be auto-started too.
				autoStartAll();
			} catch (UnsupportedAppManagerException uame) {
				uame.printStackTrace();
			}

			if ((ams != null) && (appman != null)) {
				appman.setCommandListener(ams);
				try {
					appman.connect();
				} catch (IOException e) {
					Logging.report(Logging.WARNING, LogChannels.LC_AMS, "AMS failed to connected to: "+appman.toString()+", for reason: "+e.toString());
				}
			}
		}

		while (appman != null) {
			if (appman.isConnected()) {
				try {
					Logging.report(Logging.INFORMATION, LogChannels.LC_AMS, "AMS connected to: "+appman.toString());
					synchronized (appman) {
						appman.wait();
					}
				} catch (InterruptedException e) {
				} finally {				
					try {
						appman.disconnect();	
					} catch (IOException ioe) {
					}
				}
			}
			
			Logging.report(Logging.WARNING, LogChannels.LC_AMS, "AMS reconnecting in 10 seconds...");

			try {
				appman.disconnect();
				Thread.sleep(10000);
				
				appman.connect();		
			} catch (IOException e) {
				Logging.report(Logging.WARNING, LogChannels.LC_AMS, "AMS failed to connected to: "+appman.toString()+", for reason: "+e.toString());
			} catch (InterruptedException ite) {
			}
		}
	}

	synchronized public void commandStartApp(String uniqueID, String appName, String mainClass) {
		startApp(uniqueID, appName, mainClass);
	}
	synchronized public void commandRemoveApp(String uniqueID, String appName) {
		if (runningIsolate != null) {
			try {
				appman.response(uniqueID, AppManager.APPMAN_RESPCODE_APPNOTFINISH);
			} catch (IOException e) {
				e.printStackTrace();
			}
			return;
		}
		
		if (!isInstalled(appName)) {
			try {
				appman.response(uniqueID, AppManager.APPMAN_RESPCODE_APPNOTEXIST);
			} catch (IOException e) {
				e.printStackTrace();
			}
			return;
		}
		
		Protocol fconn = new Protocol();
		String filename = "//"+Jams.getAppdbRoot()+appName;
		try {
			fconn.openPrim(securityToken, filename+".jar", Connector.READ_WRITE, false);
			fconn.delete();
			appman.response(uniqueID, AppManager.APPMAN_RESPCODE_DELETEOK);
		} catch (IOException ioe) {
			try {
				appman.response(uniqueID, AppManager.APPMAN_RESPCODE_DELETEFAIL);
			}  catch (IOException e) {
				e.printStackTrace();
			}
		} finally {
			if (fconn != null) {
				try {
					fconn.close();
				} catch (IOException ex) {
					ex.printStackTrace();
				}
			}
		}

		fconn = null;
		
		try {
			fconn = new Protocol();
			fconn.openPrim(securityToken, filename+".aut", Connector.READ_WRITE, false);
			if (fconn.exists()) {
				fconn.delete();
			}	
			Logging.report(Logging.CRITICAL, LogChannels.LC_AMS, "Application uninstalled: "+appName);
		} catch (IOException ex) {
		} finally {
			if (fconn != null) {
				try {
					fconn.close();
				} catch (IOException ex) {
				}
			}
		}
	}
	synchronized public void commandStopApp(String uniqueID) {
		if (runningIsolate != null) {
			stoppingUniqueID = uniqueID;
			runningIsolate.exit(0);
		} else {
			try {
				appman.response(uniqueID, AppManager.APPMAN_RESPCODE_APPFINISH, String.valueOf(0));
			} catch (IOException ioe) {
				ioe.printStackTrace();
			}
		}
	}
	synchronized public void commandRestartApp(String uniqueID) {
	}
	
	synchronized public void commandHeartbeat(String uniqueID) {
		try {
			appman.notifyConnected();
			appman.response(uniqueID, AppManager.APPMAN_RESPCODE_HEARTBEAT);
		} catch (IOException ioe) {
			ioe.printStackTrace();
		}
	}
	
	synchronized public void commandInstallApp(boolean forceInstall, String uniqueID, String appName, String mainClass, String installSource, int length, boolean autoStart, boolean startAfterInstall) {
		try {
			if (!forceInstall && isInstalled(appName)) {
				appman.response(uniqueID, AppManager.APPMAN_RESPCODE_APPEXIST);
				if (startAfterInstall) {
					startApp(uniqueID, appName, mainClass);
				}
			} else {
				Installer inst = getInstaller(installSource);
				
				try {
					inst.install(appName, mainClass, length, autoStart);
					Logging.report(Logging.CRITICAL, LogChannels.LC_AMS, "Application installed: "+appName);
					appman.response(uniqueID, AppManager.APPMAN_RESPCODE_INSTALLOK);

					if (startAfterInstall) {
						startApp(uniqueID, appName, mainClass);
					}
				} catch (Exception e) {
					e.printStackTrace();
					appman.response(uniqueID, AppManager.APPMAN_RESPCODE_INSTALLFAIL);
				}
			}
		} catch (IOException ioe) {
			ioe.printStackTrace();
		}
	}

	synchronized public void commandListInstalledApp(String uniqueID) {
		String filepath = "//"+Jams.getAppdbRoot();
		StringBuffer applist = new StringBuffer();
		try {
			Protocol fconn = new Protocol();
			fconn.openPrim(securityToken, filepath, Connector.READ_WRITE, false);
			java.util.Enumeration em = fconn.list();
			
			while (em.hasMoreElements()) {
				String filename = (String)em.nextElement();
				appendAppNameToList(applist, filename);
			}

			int length = applist.length();
			if (length > 0) {
				applist.setLength(length - 1); //Remove the last ";"
			} else {
				applist.append("<No applications found>");
			}

			appman.response(uniqueID, AppManager.APPMAN_RESPCODE_APPLIST, applist.toString());

			fconn.close();
		} catch (IOException ioe) {
			ioe.printStackTrace();
		}
	}

	synchronized public void commandListRunningApp(String uniqueID) {
		String applist;
		
		if (runningIsolate != null) {
			String cp = runningIsolate.getClassPath()[0];			
			
			int sep = cp.lastIndexOf('/');
			if (sep == -1) {
				sep = cp.lastIndexOf('\\');
			}
			sep++; //if sep == -1, then sep = 0
			
			if (cp.endsWith(".jar")) {
				applist = cp.substring(sep, cp.length()-4);
			} else {
				applist = cp.substring(sep);
			}
		} else {
			applist = new String("EMPTY_LIST");
		}

		try {
			appman.response(uniqueID, AppManager.APPMAN_RESPCODE_RUNNINGAPPLIST, applist.toString());
		} catch (IOException ioe) {
			ioe.printStackTrace();
		}
	}

	synchronized public void commandGetAppInfo(String uniqueID, String appName) {
	}

	synchronized public void commandResetJVM(String uniqueID) {
		Logging.report(Logging.INFORMATION, LogChannels.LC_AMS, "=============Reset now by server command=============");
		platformControl.reset();
	}

	synchronized public void commandSetSysTime(String uniqueID, long milliSecondsToSet) {
		Logging.report(Logging.INFORMATION, LogChannels.LC_AMS, "System time has been adjusted");
		platformControl.setCurrentTimeMillis(milliSecondsToSet);
	}

	private static void startApp(final String uniqueID, final String appName, final String mainClass) {
		if (runningIsolate != null) {
			Logging.report(Logging.ERROR, LogChannels.LC_AMS, "[startApp]There's another application running. Failed to start: "+appName);
			try {
				appman.response(uniqueID, AppManager.APPMAN_RESPCODE_APPNOTFINISH);
			} catch (IOException e) {
				e.printStackTrace();
			}
			return;
		}
		
		new Thread( new Runnable() {
			public void run() {
				boolean flag = false;
				try {
					Logging.report(Logging.CRITICAL, LogChannels.LC_AMS, "New Isolate: "+mainClass+","+appName+".jar");
					if (!isInstalled(appName)) {
						throw new ClassNotFoundException();
					}
					Isolate iso = null;
					synchronized (startSyncObj) {
						if (runningIsolate != null) {
							Logging.report(Logging.WARNING, LogChannels.LC_AMS, "[startApp]There's another application running.");
							return;
						}
						iso = new Isolate(mainClass, new String[0], new String[]{getAppdbNativeRoot()+appName+".jar"});
						Logging.report(Logging.INFORMATION, LogChannels.LC_AMS, "New Isolate: "+appName+" about to start");
						iso.start();
						Logging.report(Logging.INFORMATION, LogChannels.LC_AMS, "New Isolate: "+appName+" started, waiting for exit...");
						try {
							appman.response(uniqueID, AppManager.APPMAN_RESPCODE_APPSTARTOK);
						} catch (IOException ioe) {
							ioe.printStackTrace();
						}
						runningIsolate = iso;
						flag = true;
					}
					stoppingUniqueID = null;
					iso.waitForExit();
					int exitcode = iso.exitCode();
					Logging.report(Logging.CRITICAL, LogChannels.LC_AMS, "Isolate: "+appName+" exit with code:"+exitcode);
					try {
						appman.response(stoppingUniqueID != null?stoppingUniqueID:uniqueID, AppManager.APPMAN_RESPCODE_APPFINISH, String.valueOf(exitcode));
					} catch (IOException ioe) {
						ioe.printStackTrace();
					}
				} catch (IsolateStartupException ise) {
					ise.printStackTrace();
					try {
						appman.response(uniqueID, AppManager.APPMAN_RESPCODE_APPSTARTERROR);
					} catch (IOException ioe) {
						ioe.printStackTrace();
					}
				} catch (ClassNotFoundException cnfe) {

					cnfe.printStackTrace();
					try {
						appman.response(uniqueID, AppManager.APPMAN_RESPCODE_APPNOTEXIST);
					} catch (IOException ioe) {
						ioe.printStackTrace();
					}
				} finally {
					if (flag) {
						runningIsolate = null;
					}
				}
			}
		}).start();
	}

	synchronized public void commandInstallReg(boolean forceInstall, String uniqueID, String installSource, int length) {
		try {
			if (!forceInstall && isRegistered()) {
				appman.response(uniqueID, AppManager.APPMAN_RESPCODE_APPEXIST);
			} else {
				if (length <= 0 || length > REGFILE_MAX_SIZE) {
					Logging.report(Logging.CRITICAL, LogChannels.LC_AMS, "Invalid reg file.");
					appman.response(uniqueID, AppManager.APPMAN_RESPCODE_INSTALLFAIL);
					return;
				}

				Installer inst = getInstaller(installSource);
				try {
					inst.installReg(length);
					Logging.report(Logging.CRITICAL, LogChannels.LC_AMS, "Registered.");
					appman.response(uniqueID, AppManager.APPMAN_RESPCODE_INSTALLOK);
				} catch (Exception e) {
					e.printStackTrace();
					appman.response(uniqueID, AppManager.APPMAN_RESPCODE_INSTALLFAIL);
				}
			}
		} catch (IOException ioe) {
			ioe.printStackTrace();
		}
	}

	synchronized public void commandRemoveReg(String uniqueID) {
		if (!isRegistered()) {
			try {
				appman.response(uniqueID, AppManager.APPMAN_RESPCODE_APPNOTEXIST);
			} catch (IOException e) {
				e.printStackTrace();
			}
			return;
		}

		Protocol fconn = new Protocol();
		String filename = "//"+Jams.getRegFilePath();
		try {
			fconn.openPrim(securityToken, filename, Connector.READ_WRITE, false);
			fconn.delete();
			Logging.report(Logging.CRITICAL, LogChannels.LC_AMS, "Unregistered.");
			appman.response(uniqueID, AppManager.APPMAN_RESPCODE_DELETEOK);
		} catch (IOException ioe) {
			try {
				appman.response(uniqueID, AppManager.APPMAN_RESPCODE_DELETEFAIL);
			}  catch (IOException e) {
				e.printStackTrace();
			}
		} finally {
			if (fconn != null) {
				try {
					fconn.close();
				} catch (IOException ex) {
					ex.printStackTrace();
				}
			}
		}
	}

	public static String getAppdbNativeRoot() {
		String path = appman.getAppdbNativeRoot();
		Logging.report(Logging.INFORMATION, LogChannels.LC_AMS, "appdb_native_root="+path);
		return path;
	}

	public static String getAppdbRoot() {
		String path = appman.getAppdbRoot();
		Logging.report(Logging.INFORMATION, LogChannels.LC_AMS, "appdb_root="+path);
		return path;
	}

	private StreamConnection getCommConnection() {
		return null;
	}

	
	private StreamConnection getLocalHostConnection() {
		return null;
	}	

	private static void autoStartAll() {
		String filepath = "//"+Jams.getAppdbRoot();
		Logging.report(Logging.INFORMATION, LogChannels.LC_AMS, "Try to find auto-start application...");
		try {
			Protocol fconn = new Protocol();
			fconn.openPrim(securityToken, filepath, Connector.READ_WRITE, false);
			java.util.Enumeration em = fconn.list();
			while (em.hasMoreElements()) {
				String filename = (String)em.nextElement();
				if (filename.endsWith(".aut")) {
					Protocol asfile = new Protocol();
					asfile.openPrim(securityToken, filepath+filename, Connector.READ_WRITE, false);
					
					String appname = filename.substring(0, filename.length()-4);
					if (!isInstalled(appname)) {						
						asfile.delete();						
					} else {
						
						InputStream in = asfile.openInputStream();
						byte[] buf = new byte[64];
						int len = in.read(buf);
						in.close();
						
						String mainclass = new String(buf, 0, len);
						Logging.report(Logging.INFORMATION, LogChannels.LC_AMS, "Find auto start application: "+appname);
						startApp(null, appname, mainclass);
					}
					asfile.close();
				}
			}
			fconn.close();
		} catch (IOException ioe) {
			ioe.printStackTrace();
		}
	}

	private static boolean isInstalled(String appname) {
		boolean result;
		String filepath = "//"+Jams.getAppdbRoot()+appname+".jar";
		try {
			Protocol fconn = new Protocol();
			fconn.openPrim(securityToken, filepath, Connector.READ_WRITE, false);
			if (fconn.exists()) {
				result = true;
			} else {
				result = false;
			}
			fconn.close();
		} catch (IOException ioe) {
			ioe.printStackTrace();
			result = false;
		}
		return result;
	}

	private static boolean isAutostart(String appname) {
		boolean result;
		String filepath = "//"+Jams.getAppdbRoot()+appname+".aut";
		try {
			Protocol fconn = new Protocol();
			fconn.openPrim(securityToken, filepath, Connector.READ_WRITE, false);
			if (fconn.exists()) {
				result = true;
			} else {
				result = false;
			}

			fconn.close();
		} catch (IOException ioe) {
			ioe.printStackTrace();
			result = false;
		}
		return result;
	}

	static String getRegFilePath() {
		String path = System.getProperty("org.joshvm.ams.appdbpath.root.secure");
		if (path == null) {
			Logging.report(Logging.INFORMATION, LogChannels.LC_AMS, "getRegFilePath use default");
			path = new String("/internal/appdb/secure/");
		}
		return path + "reg.dat";
	}

	private static boolean isRegistered() {
		boolean result;
		String filepath = "//"+Jams.getRegFilePath();
		try {
			Protocol fconn = new Protocol();
			fconn.openPrim(securityToken, filepath, Connector.READ_WRITE, false);
			if (fconn.exists()) {
				result = true;
			} else {
				result = false;
			}

			fconn.close();
		} catch (IOException ioe) {
			ioe.printStackTrace();
			result = false;
		}
		return result;
	}

	static byte[] getRegFileData() {
		byte[] buf = new byte[REGFILE_MAX_SIZE];
		String filepath = "//"+Jams.getRegFilePath();
		Protocol fconn = null;
		int total = 0;
		try {
			fconn = new Protocol();
			fconn.openPrim(securityToken, filepath, Connector.READ_WRITE, false);
			InputStream is = fconn.openInputStream();
			while (total < REGFILE_MAX_SIZE) {
				int count = is.read(buf, total, REGFILE_MAX_SIZE - total);
				if (count < 0) {
					break;
				} else {
					total += count;
				}
			}
			is.close();
		} catch (IOException ioe) {
			ioe.printStackTrace();
			return null;
		} finally {
			if (fconn != null) {
				try {
					fconn.close();
				} catch (IOException e) {
				}
			}
		}

		if (total == REGFILE_MAX_SIZE) {
			return buf;
		}

		byte[] data = new byte[total];
		System.arraycopy(buf, 0, data, 0, total);
		return data;
	}

	private static void appendAppNameToList(StringBuffer applist, String filename) {
		if (filename.endsWith(".jar") && !filename.endsWith(".ini.jar")) {
			String appname = filename.substring(0, filename.length()-4);
			applist.append(appname);
			if (isAutostart(appname)) {
				applist.append("[AUTOSTART]");
			}
			applist.append(";");
		}
	}

	public void event(int event_code, int arg) {
		return;
	}

	private class UnsupportedAppManagerException extends Exception {
		UnsupportedAppManagerException() {
        	super();
    	}

	    UnsupportedAppManagerException(String s) {
    	    super(s);
    	}
	}
}

