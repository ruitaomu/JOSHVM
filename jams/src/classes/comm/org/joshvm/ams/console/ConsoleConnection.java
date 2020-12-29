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
package org.joshvm.ams.console;


import java.io.IOException;
import java.io.*;


public class ConsoleConnection {
	private static final byte[] output_buffer;
	OutputStream os;
	InputStream is;
	static {
		output_buffer = new byte[105];
		output_buffer[0] = (byte)0xfe;
		output_buffer[1] = (byte)0xaa;
		output_buffer[2] = (byte)0xbc;
		output_buffer[3] = (byte)0xef;
	}

	void open (OutputStream os, InputStream is) {
		this.os = os;
		this.is = is;
	}

	void close() {
		os = null;
		is = null;
	}
	
	String receiveString() throws IOException  {
		byte[] buf = receivePacket();
		if ((buf == null) || (buf.length == 0)) {
			throw new IOException();
		}
		return new String(buf);
	}

	int receiveInt() throws IOException  {
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

	int read(byte b[], int off, int len) throws IOException {
		for (int i = 0; i < len; i++) {
			b[off+i] = readByteBlocking();
		}
		return len;
	}
	
	byte[] receivePacket() throws IOException {
		int len;
		len = readByteBlocking();
		len = len & 0xff;
		byte[] buf = new byte[len];
		for (int i = 0; i < len; i++) {
			buf[i] = readByteBlocking();
		}
		return buf;
	}

	synchronized void sendBuf(byte[] buf, int offset, int length) throws IOException {
		if (length > 100) {
			throw new IOException("Size is over the maximum limit of sendBuf");
		}
		output_buffer[4] = (byte)(length & 0xff);
		System.arraycopy(buf,offset,output_buffer,5,length);
		os.write(output_buffer, 0, length+5);
	}

	synchronized void sendRTS() throws IOException {
		output_buffer[4] = 0;
		os.write(output_buffer, 0, 5);
	}

	private byte readByteBlocking() throws IOException {
		int i;
		int retry = 0;
		while ((retry < 3) && (is.available() == 0)) {
			try {
				Thread.sleep(50);
			} catch (InterruptedException ie) {
			}
			retry++;
		}

		if (is.available() == 0) {
			throw new IOException("Can't connect to console");
		}

		i = is.read();

		return (byte)(i & 0xff);
	}
	
}

