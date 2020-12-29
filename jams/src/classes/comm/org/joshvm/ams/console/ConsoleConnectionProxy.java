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
import javax.microedition.io.StreamConnection;
import javax.microedition.io.Connector;
import org.joshvm.ams.console.ConsoleConnection;
import java.util.Vector;

public class ConsoleConnectionProxy {
	private static ConsoleConnectionProxy proxy = null;
	private boolean listeningThreadStopped;
	private int connections;
	private boolean isListeningCommand;
	private Vector handlers;
	private static ConsoleConnection consoleConnection;
	private StreamConnection sc;
	private static final byte[] DISCONNECT_PACKET = {(byte)0xfe, (byte)0xaa, (byte)0xbc, (byte)0xfe};

	protected ConsoleConnectionProxy() {
		listeningThreadStopped = false;
		connections = 0;
		isListeningCommand = false;
		handlers = new Vector();
		consoleConnection = new ConsoleConnection();
	}
	
	public synchronized void registerConsoleCommandHandler(ConsoleCommandHandler handler) throws IllegalArgumentException {
		
		if (handler == null) {
			throw new IllegalArgumentException();
		}

		if (-1 == handlers.indexOf(handler)) {
			handlers.addElement(handler);
		}
		
		startListenCommand();
	}

	public synchronized void unregisterConsoleCommandHandler(ConsoleCommandHandler handler) {
		
		if (handlers.removeElement(handler) && handlers.isEmpty()) {
			stopListenCommand();
		}
	}

	public synchronized void connect() throws IOException {
		if (connections == 0) {
			sc = (StreamConnection)Connector.open("comm://COM0");
			consoleConnection.open(sc.openOutputStream(), sc.openInputStream());
			if (!handlers.isEmpty()) {
				startListenCommand();
			}
		}
		connections++;
	}

	public synchronized void close() throws IOException {
		if (connections > 0) {
			connections--;
			if (connections == 0) {
				try {
					write(DISCONNECT_PACKET);
				} catch (IOException ioe) {
				}
				stopListenCommand();
				stop();
			}
		}
	}

	public static synchronized ConsoleConnectionProxy start() throws IOException {
		if (proxy == null) {
		
			proxy = new ConsoleConnectionProxy();
			new Thread(new Runnable() {
					public void run() {
						proxy.commandListenerLoop();
					}
				}).start();
		}
		return proxy;
	}

	private void stop() {
		listeningThreadStopped = true;
		synchronized (this) {
			notifyAll();
		}
	}

	private static ConsoleConnectionProxy getProxy() {
		return proxy;
	}

	private void commandListenerLoop() {
		try {
			while(!listeningThreadStopped) {
				if (connections > 0 && isListeningCommand) {
					ConsoleCommand cmd = getCommand();
					dispatchCommand(cmd);
				} else {
					synchronized (proxy) {
						try {
							proxy.wait();
						} catch (InterruptedException ie) {
						}
					}
				}
			}
		} catch (Exception e) {
			e.printStackTrace();
			dispatchCommand(ConsoleCommand.getRemoteDisconnectedCommand());
			connections = 0;
			stopListenCommand();
			stop();
		} finally {
			proxy = null;
			if (sc != null) {
				try {
					consoleConnection.close();
					sc.close();
				} catch (IOException ioe) {
				}
			}
		}
	}

	private synchronized void startListenCommand() {
		isListeningCommand = true;
		notifyAll();
	}

	private synchronized void stopListenCommand() {
		isListeningCommand = false;
	}

	protected ConsoleCommand getCommand() throws IOException {
		consoleConnection.sendRTS();
		return waitCommand();
	}

	private void dispatchCommand(ConsoleCommand cmd) {
		if (cmd == null) return;
		
		for (int i = 0; i < handlers.size(); i++) {
			ConsoleCommandHandler handler = (ConsoleCommandHandler)proxy.handlers.elementAt(i);
			if (handler != null) {
				handler.onConsoleCommand(cmd);
			}
		}
	}

	protected void write(byte[] buf) throws IOException {
		write(buf, 0, buf.length);
	}

	protected void write(byte[] buf, int offset, int length) throws IOException {
		consoleConnection.sendBuf(buf, offset, length);
	}

	public void writeBytes(byte[] buf, int offset, int length) throws IOException {
		write(buf, offset, length);
	}

	protected static ConsoleCommand waitCommand() throws IOException {
		String message;
		int head = consoleConnection.receiveInt();
		int type = head & 0xff;
		if ((head & 0x00000100) != 0) {
			message = consoleConnection.receiveString();
		} else {
			message = null;
		}
		return new ConsoleCommand(type, message);
	}
}

