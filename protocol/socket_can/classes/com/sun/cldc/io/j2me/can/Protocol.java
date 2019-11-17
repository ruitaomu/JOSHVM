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

package com.sun.cldc.io.j2me.can;

import java.io.*;

import javax.microedition.io.*;

import com.sun.cldc.io.BufferedConnectionAdapter;
//import com.sun.j2me.security.*;
import java.security.*;  //We currently use dummy implementation of AccessControl


/**
 * This implements the comm port protocol.
 */
public class Protocol extends com.sun.cldc.io.j2me.socket.Protocol {

    /** Comm permission name. */
    private static final String CANBUS_PERMISSION_NAME =
        "javax.microedition.io.Connector.can";

    /** Baud rate. */
    private int baud     = 20000;
    /** Blocking. */
    private boolean blocking = true;

    /**
     * Parse the next parameter out of a string.
     *
     * @param parm a string containing one or more parameters
     * @param start where in the string to start parsing
     * @param end where in the string to stop parsing
     *
     * @exception IllegalArgumentException if the next parameter is wrong
     */
    private void parseParameter(String parm, int start, int end) {
        parm = parm.substring(start, end);

        if (parm.startsWith("baudrate=")) {
			try {
            	baud = Integer.parseInt(parm.substring(9));
			} catch (NumberFormatException nfe) {
				throw new IllegalArgumentException(nfe.toString());
			}
        } else if (parm.equals("blocking=off")) {
            blocking = false;
        } else if (parm.equals("blocking=on")) {
            blocking = true;
        } else {
            throw new IllegalArgumentException("Bad parameter");
        }
    }

    /**
     * Open a CAN bus connection.
     * 
     * @param name A URI with the type and parameters for the connection.
     * <pre>
     * The scheme must be: can
     *
     * The first parameter must be a port ID: A device name or
     * a logical port number from 0 to 9.
     *
     * Any additional parameters must be separated by a ";" and
     * spaces are not allowed.
     *
     * The optional parameters are:
     *
     * baudrate:    The speed of the port, defaults to 20000.
     * blocking:    If "on" wait for a full buffer when reading.
     *              Defaults to "on".
     * </pre>
     * @param mode       A flag that is <code>true</code> if the caller expects
     *                   to write to the connection. This is ignored
     *                   in all connections that are read-write.
     * @param timeouts   A flag to indicate that the called wants
     *                   timeout exceptions. This is ignored.
     *
     *
     * @return reference to this connection
     *
     * @exception  IOException  if an I/O error occurs, or
     *             IllegalArgumentException
     *             if the name string is has an error.
     */
    public Connection openPrim(String name, int mode, boolean timeouts)
            throws IOException {

        int portNumber = 0;
        String deviceName = null;
        int start = 0;
        int pos = 0;

        checkForPermission(name);

        if (name.length() == 0) {
             throw new IllegalArgumentException("Missing port ID");
        }

        if (Character.isDigit(name.charAt(0))) {
            portNumber = Integer.parseInt(name.substring(0, 1));
            pos++;
        } else {
            pos = name.indexOf(";");
            if (pos < 0) {
                deviceName = name;
                pos = name.length();
            } else {
                deviceName = name.substring(0, pos);
            }
        }

        while (name.length() > pos) {
            if (name.charAt(pos) != ';') {
                throw new IllegalArgumentException(
                    "missing parameter delimiter");
            }

            pos++;
            start = pos;
            while (true) {
                if (pos == name.length()) {
                    parseParameter(name, start, pos);
                    break;
                }

                if (name.charAt(pos) == ';') {
                    parseParameter(name, start, pos);
                    break;
                }
                pos++;
            }
        }

		// cstring is always NUL terminated (note the extra byte allocated).
		// This avoids awkward char array manipulation in C code.
		byte cstring[] = new byte[deviceName.length() + 1];
		for (int n=0; n<deviceName.length(); n++) {
			cstring[n] = (byte)(deviceName.charAt(n));
		}

        handle = open0(cstring, mode);
        
        opens++;
        copen = true;
        this.mode = mode;
        return this;
    }

    /**
     * Check for the required permission.
     *
     * @param name name of resource to insert into the permission question
     *
     * @exception IOInterruptedException if another thread interrupts the
     *   calling thread while this method is waiting to preempt the
     *   display.
     */
	private void checkForPermission(String name)
				throws IOException {
		name = "can" + ":" + name;

		try {
			AccessController.checkPermission(CANBUS_PERMISSION_NAME, name);
		} catch (SecurityException ise) {
			throw new IOException("No permission to access comm port");
		}
	}


    /** 
     * Gets the baudrate for the serial port connection.
     * @return the baudrate of the connection
     * @see #setBaudRate
     */
    public int getBaudRate() {
		return baud;
    }

	protected static native int open0(byte[] name, int mode) throws IOException;
      
}



