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

import java.io.IOException;
import java.io.*;


public class ConsoleConnection {
	/**
	 * Version: MAGIC[3] is the protocol version number. 0xef is Version 0.
	 * Version 1 is 0xee, Version 2 is 0xed ...
	 * Version number is (byte)(0xef - MAGIC[3])
	 *
	 **/
	private static final byte[] MAGIC = {(byte)0xfe, (byte)0xaa, (byte)0xbc, (byte)0xee};
	OutputStream os;
	InputStream is;

	public void open (OutputStream os, InputStream is) {
		this.os = os;
		this.is = is;
	}

	public void close() {
		os = null;
		is = null;
	}
	
	public String receiveString() throws IOException  {
		byte[] buf = receivePacket();
		if ((buf == null) || (buf.length == 0)) {
			throw new IOException();
		}
		return new String(buf);
	}

	public int receiveInt() throws IOException  {
		byte[] buf = receivePacket();
		if (buf.length != 4) {
			throw new IOException();
		}
		int n = 0;
		for (int i = 0; i < 4; i++) {
			n = n << 8;
			n |= (int)buf[i] & 0xff;			
		} 
		return n;
	}

	public int read(byte b[], int off, int len) throws IOException {
		for (int i = 0; i < len; i++) {
			b[off+i] = readByteBlocking();
		}
		return len;
	}

	public void sendRequest(int request_id) throws IOException {
		sendMagic();
		os.write(request_id);		
	}
	
	public void sendReport(int report_id) throws IOException {
		sendMagic();
		os.write(report_id);
	}
	
	public byte[] receivePacket() throws IOException {
		int len;
		len = readByteBlocking();
		len = len & 0xff;
		byte[] buf = new byte[len];
		for (int i = 0; i < len; i++) {
			buf[i] = readByteBlocking();
		}
		return buf;
	}



	private void sendMagic() throws IOException {
		os.write(MAGIC);
	}

	private byte readByteBlocking() throws IOException {
		int i;
		int retry = 10;
		while (retry -- > 0 && is.available() == 0) {
			try {
				Thread.sleep(100);
			} catch (InterruptedException e) {
			}
		}
		if (retry <= 0) {
			throw new IOException("Serial console connection timeout. Not connected to any serial console.");
		}

		i = is.read();

		return (byte)(i & 0xff);
	}
	
}

