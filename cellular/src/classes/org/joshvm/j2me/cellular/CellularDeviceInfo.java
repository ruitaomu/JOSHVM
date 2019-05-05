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

import java.io.IOException;

/**
 * CellularDeviceInfo is the main class to get celluar network related information of the device,
 * including APN, IMEI, IMSI, ICCID, simcard and cell station information, etc.
 *
 * @author      Max Mu
 */
public class CellularDeviceInfo {
	
	private static CellularDeviceInfo[] devices;

	private static AccessPoint default_apn;

	private int device_id;

	public static final int SIMCARD_STATUS_UNKNOWN = 0;
	public static final int SIMCARD_STATUS_INSERTED = 1;
	public static final int SIMCARD_STATUS_REMOVED = 2;
	
	
	static {
		default_apn = new AccessPoint(getCurrentAPNName0(), getCurrentAPNUsername0(), getCurrentAPNPassword0());
	}
		
	protected CellularDeviceInfo(int id)  throws IOException {
		device_id = id;
	}

	/** 
     * Set APN for current cellular device
     *
     * @param apn       the APN to be set for next time attaching to celluar network
     * @see             AccessPoint
     */	
	public void setAPN(AccessPoint apn) throws IOException {
		String apnname  = apn.getName();
		String username = apn.getUserName();
		String password = apn.getPassword();

		if (apnname == null) apnname = "";
		if (username == null) username = "";
		if (password == null) password = "";
		
		setAPN0(apnname, username, password);
	}

	/** 
     * Reset dfault APN for current cellular device
     *
     * @see             AccessPoint
     */	
	public void resetDefaultAPNSetting() throws IOException {
		setAPN(default_apn);
	}

	/** 
     * Get current APN setting
     *
     * @return	The current APN setting
     * @see     AccessPoint
     */	
     public AccessPoint getCurrentAPNSetting()  throws IOException {
		return new AccessPoint(getCurrentAPNName0(), getCurrentAPNUsername0(), getCurrentAPNPassword0());
	}

	public String getIMSI()  throws IOException {
		return getIMSI0();
	}

	public String getIMEI()  throws IOException {
		return getIMEI0();
	}

	public String getICCID()  throws IOException {
		return getICCID0();
	}

	/**
	 * Get cellular networking signal level
	 *
	 * @return 0-31, signal strength;
	 *         99,   not known or not detectable
	 */
	public native int getNetworkSignalLevel()  throws IOException ;

	/**
	 * Get current celluar network information
	 *
	 * @return Current network information, including MNC and MCC, Networking Type, etc.
	 * @see  NetworkInfo
	 */
	public NetworkInfo getNetworkInfo()  throws IOException {
		int mnc = getMNC0();
		int mcc = getMCC0();

		return new NetworkInfo(mnc, mcc, NetworkInfo.NETWORK_TYPE_3GPP, "DUMMY");
	}

	/**
	 * Get current celluar station information
	 *
	 * @return Current celluar station information
	 * @see  CellInfo
	 */
	public CellInfo getCellInfo()  throws IOException {
		int lac = getLAC0();
		return new CellInfo(lac);
	}

	/**
	 * Get sim card status
	 *
	 * @return CellularDeviceInfo.SIMCARD_STATUS_UNKNOWN  Unknow status<p>
	 *         CellularDeviceInfo.SIMCARD_STATUS_INSERTED Simcard exists<p>
	 *         CellularDeviceInfo.SIMCARD_STATUS_REMOVED Simcard is not inserted<p>
	 *
	 */
	public int getSIMCardStatus()  throws IOException {
		return SIMCARD_STATUS_UNKNOWN;
	}

	/**
	 * List all installed cellular devices in the system. For example, if the device has 2 modules installed,
	 * this API will returns an array which contains 2 elements.
	 *
	 * @return All the installed cellular devices
	 *
	 */
	public synchronized static CellularDeviceInfo[] listCellularDevices() {
		if (devices == null) {
			try {
				CellularDeviceInfo info = new CellularDeviceInfo(0);
				devices = new CellularDeviceInfo[1];
				devices[0] = info;
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
		return devices;
	}

	private native void setAPN0(String name, String username, String password);
	private static native String getCurrentAPNName0();
	private static native String getCurrentAPNUsername0();		
	private static native String getCurrentAPNPassword0();
	private native int getLAC0();
	private native int getMNC0();
	private native int getMCC0();
	private native String getIMSI0();
	private native String getIMEI0();
	private native String getICCID0();

}

