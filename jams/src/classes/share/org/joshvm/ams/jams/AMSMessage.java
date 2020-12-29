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

public class AMSMessage {
	public static final int MESSAGE_TYPE_ID_HEARTBEAT = AppManager.APPMAN_COMMAND_HEARTBEAT;
	public static final int MESSAGE_TYPE_ID_REGISTER = AppManager.APPMAN_RESPCODE_REGISTER;
	public static final int MESSAGE_TYPE_ID_DOWNLOAD_APP = AppManager.APPMAN_COMMAND_DOWNLOAD_JAR;
	public static final int MESSAGE_TYPE_ID_START_APP = AppManager.APPMAN_COMMAND_RUN_APP;
	public static final int MESSAGE_TYPE_ID_LIST_APP = AppManager.APPMAN_COMMAND_LIST_APP;
	public static final int MESSAGE_TYPE_ID_STOP_APP = AppManager.APPMAN_COMMAND_STOP_APP;
	public static final int MESSAGE_TYPE_ID_DELETE_APP = AppManager.APPMAN_COMMAND_DELETE_APP;
	public static final int MESSAGE_TYPE_ID_RUNNING_APP = AppManager.APPMAN_COMMAND_RUNNING_APP;
	public static final int MESSAGE_TYPE_ID_RESET_JVM = AppManager.APPMAN_COMMAND_RESET_JVM;
	public static final int MESSAGE_TYPE_ID_DOWNLOAD_REG = AppManager.APPMAN_COMMAND_DOWNLOAD_REG;
	public static final int MESSAGE_TYPE_ID_DELETE_REG = AppManager.APPMAN_COMMAND_DELETE_REG;
	public static final int MESSAGE_TYPE_ID_SET_SYSTIME = AppManager.APPMAN_COMMAND_SET_SYSTIME;

	public static final byte[] MESSAGE_TYPE_HEARTBEAT;
	public static final byte[] MESSAGE_TYPE_REPORT_REGISTER;
	public static final byte[] MESSAGE_TYPE_DOWNLOAD_APP;
	public static final byte[] MESSAGE_TYPE_START_APP;
	public static final byte[] MESSAGE_TYPE_LIST_APP;
	public static final byte[] MESSAGE_TYPE_STOP_APP;
	public static final byte[] MESSAGE_TYPE_DELETE_APP;
	public static final byte[] MESSAGE_TYPE_RUNNING_APP;
	public static final byte[] MESSAGE_TYPE_RESET_JVM;
	public static final byte[] MESSAGE_TYPE_DOWNLOAD_REG;
	public static final byte[] MESSAGE_TYPE_DELETE_REG;
	public static final byte[] MESSAGE_TYPE_SET_SYSTIME;
	public static final byte[] LINE_END;
	public static final byte[] SEPARATOR;

	private String _body;
	private byte[] _type;
	private int _typeID;
	private String _uniqueID;

	static {
		MESSAGE_TYPE_REPORT_REGISTER = new String("[REGISTER]").getBytes();
		MESSAGE_TYPE_HEARTBEAT =       new String("[CHECKCON]").getBytes();
		MESSAGE_TYPE_DOWNLOAD_APP =    new String("[DOWNLAPP]").getBytes();
		MESSAGE_TYPE_START_APP =       new String("[STARTAPP]").getBytes();
		MESSAGE_TYPE_LIST_APP =        new String("[RLISTAPP]").getBytes();
		MESSAGE_TYPE_STOP_APP =        new String("[RSTOPAPP]").getBytes();
		MESSAGE_TYPE_DELETE_APP = 	   new String("[RDELEAPP]").getBytes();
		MESSAGE_TYPE_RUNNING_APP =     new String("[RUNNGAPP]").getBytes();
		MESSAGE_TYPE_RESET_JVM =       new String("[RESETJVM]").getBytes();
		MESSAGE_TYPE_DOWNLOAD_REG =    new String("[DOWNLREG]").getBytes();
		MESSAGE_TYPE_DELETE_REG =      new String("[RDELEREG]").getBytes();
		MESSAGE_TYPE_SET_SYSTIME =     new String("[SETSYSTM]").getBytes();
		LINE_END = new String("\n").getBytes();
		SEPARATOR = new String(",").getBytes();
	}

	public String body() { return _body;}
	public byte[] type() { return _type;}
	public int typeID() {return _typeID;}
	public String uniqueID() {return _uniqueID;}

	public AMSMessage(int typeID, byte[] type, String body) {
		_typeID = typeID;
		_type = type;
		_body = body;
		_uniqueID = null;
	}

	public AMSMessage(String strMessage) throws WrongMessageFormatException {
	  try {
		String type = strMessage.substring(0,10);
		if (type.equals("[STARTAPP]")) {
			_typeID = MESSAGE_TYPE_ID_START_APP;
			_type = MESSAGE_TYPE_START_APP;
		} else if (type.equals("[CHECKCON]")) {
			_typeID = MESSAGE_TYPE_ID_HEARTBEAT;
			_type = MESSAGE_TYPE_HEARTBEAT;
		} else if (type.equals("[DOWNLAPP]")) {
			_typeID = MESSAGE_TYPE_ID_DOWNLOAD_APP;
			_type = MESSAGE_TYPE_DOWNLOAD_APP;
		} else if (type.equals("[RLISTAPP]")) {
			_typeID = MESSAGE_TYPE_ID_LIST_APP;
			_type = MESSAGE_TYPE_LIST_APP;
		} else if (type.equals("[RSTOPAPP]")) {
			_typeID = MESSAGE_TYPE_ID_STOP_APP;
			_type = MESSAGE_TYPE_STOP_APP;
		} else if (type.equals("[RDELEAPP]")) {
			_typeID = MESSAGE_TYPE_ID_DELETE_APP;
			_type = MESSAGE_TYPE_DELETE_APP;
		} else if (type.equals("[RUNNGAPP]")) {
			_typeID = MESSAGE_TYPE_ID_RUNNING_APP;
			_type = MESSAGE_TYPE_RUNNING_APP;
		} else if (type.equals("[RESETJVM]")) {
			_typeID = MESSAGE_TYPE_ID_RESET_JVM;
			_type = MESSAGE_TYPE_RESET_JVM;
		}  else if (type.equals("[DOWNLREG]")) {
			_typeID = MESSAGE_TYPE_ID_DOWNLOAD_REG;
			_type = MESSAGE_TYPE_DOWNLOAD_REG;
		} else if (type.equals("[RDELEREG]")) {
			_typeID = MESSAGE_TYPE_ID_DELETE_REG;
			_type = MESSAGE_TYPE_DELETE_REG;
		} else if (type.equals("[SETSYSTM]")) {
			_typeID = MESSAGE_TYPE_ID_SET_SYSTIME;
			_type = MESSAGE_TYPE_SET_SYSTIME;
		} else {
			throw new WrongMessageFormatException();
		}
		if (!strMessage.substring(10, 19).equals("UNIQUEID=")) {
			throw new WrongMessageFormatException();
		}
		_uniqueID = strMessage.substring(19, 35);
	  } catch (IndexOutOfBoundsException e) {
			throw new WrongMessageFormatException();
	  }

	    if (strMessage.length() < 37) {
			_body = null;
		} else {
			_body = strMessage.substring(36);
		}
	}
}

