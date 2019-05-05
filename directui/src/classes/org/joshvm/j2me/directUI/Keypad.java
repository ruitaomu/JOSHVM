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
package org.joshvm.j2me.directUI;

import java.io.IOException;

/**
 * The {@code Keypad} class encapsulates the hardware keypad of the device. Usually 
 * it's not used for soft keypad. To operate the Keypad, use {@link org.joshvm.j2me.directUI.Keypad#getKeypad() getKeypad()}
 * to get the Keypad object.
 *
 */

public class Keypad {
    public static final char KEY_NUM0 = '0';
    public static final char KEY_NUM1 = '1';
    public static final char KEY_NUM2 = '2';
    public static final char KEY_NUM3 = '3';
    public static final char KEY_NUM4 = '4';
    public static final char KEY_NUM5 = '5';
    public static final char KEY_NUM6 = '6';
    public static final char KEY_NUM7 = '7';
    public static final char KEY_NUM8 = '8';
    public static final char KEY_NUM9 = '9';
    public static final char KEY_ASTERISK = '*';
    public static final char KEY_HASH = '#';
    public static final char KEY_POUND = '#';
    public static final char KEY_DOT = '.';
    public static final char KEY_BACK = 0xA0;
    public static final char KEY_OK = 0xA1;
    public static final char KEY_POWER = 0xA2;
    public static final char KEY_USER1 = 0xC1;
    public static final char KEY_USER2 = 0xC2;
    public static final char KEY_USER3 = 0xC3;
    public static final char KEY_USER4 = 0xC4;
    public static final char KEY_USER5 = 0xC5;
    public static final char KEY_USER6 = 0xC6;
    public static final char KEY_FUNC1 = 0xF1;
    public static final char KEY_FUNC2 = 0xF2;
    public static final char KEY_FUNC3 = 0xF3;
    public static final char KEY_FUNC4 = 0xF4;
    public static final char KEY_FUNC5 = 0xF5;
    public static final char KEY_FUNC6 = 0xF6;
    public static final char KEY_INVALID = 0xFF;

    public static final int KEY_EVENT_INVALID = -1;
    public static final int KEY_EVENT_PRESSED = 1;
    public static final int KEY_EVENT_RELEASED = 2;

    private static Keypad theKeypad;
    private KeypadEventListener eventListener = null;

    private Keypad() throws IOException {
		//TODO:
		// Check if keypad device exists?
    }

	/**
     * Sets the current Keypad Listener
     *
     * @param the event listener of the Keypad. Set to null to stop listening Keypad event.
     * NOTE: If set to non-null listener to start listening events, it's good to stop listening
     * before quit app or don't want to get any Keypad events anymore, by set it to null.
     */
    public synchronized void setKeyListener(KeypadEventListener listener) {
        eventListener = listener;
        if (eventListener != null) {
            startReadKey();
        } else {
            stopReadKey();
        }
    }

	/**
     * Gets the current Keypad Listener
     *
     * @return the current {@code KeypadEventListener}
     */
    public KeypadEventListener getKeyListener() {
        return eventListener;
    }

	/**
     * Gets the Keypad of current device
     *
     * @return the {@code Keypad} instance.
     * @throws IOException if no Keypad installed
     */
    public synchronized static Keypad getKeypad() throws IOException {
        if (theKeypad != null) {
            return theKeypad;
        }

        theKeypad = new Keypad();
        return theKeypad;
    }

    private void startReadKey() {
        KeypadThread.startReadKey(this);
    }

    private void stopReadKey() {
        KeypadThread.stopReadKey();
    }
}

class KeypadThread extends Thread {
    private static KeypadThread thread;
    private Keypad keypad;
    private boolean isStop;
    private boolean isValid;
    private boolean isInited;

    public static synchronized void startReadKey(Keypad keypad) {
        if (thread == null) {
            thread = new KeypadThread();
        }
        thread.keypad = keypad;
        thread.isStop = false;
        if (!thread.isAlive()) {
            thread.start();
        }
        if (!thread.isInited) {
            thread.initEvent();
            thread.isInited = true;
        }
    }

    public static synchronized void stopReadKey() {
        if (thread != null) {
            thread.keypad = null;
            thread.isStop = true;
            if (thread.isInited) {
                thread.finalizeEvent();
                thread.isInited = false;
            }
        }
    }

    public void run() {
        while (true) {
            KeyEvent event = new KeyEvent();
            waitingForKeyEvent(event);
            synchronized(getClass()) {
                if (isStop) {
                    thread = null;
                    break;
                }
                if (event.eventType != Keypad.KEY_EVENT_INVALID &&
                    event.keyValue != Keypad.KEY_INVALID) {
                    KeypadEventListener listener = keypad.getKeyListener();
                    if (listener != null) {
                        if (event.eventType == Keypad.KEY_EVENT_PRESSED) {
                            listener.keyPressed(event.keyValue);
                        } else if (event.eventType == Keypad.KEY_EVENT_RELEASED) {
                            listener.keyReleased(event.keyValue);
                        }
                    }
                }
            }
        }
    }

    private class KeyEvent {
        public int eventType = Keypad.KEY_EVENT_INVALID;
        public char keyValue = Keypad.KEY_INVALID;
    }

    private native void initEvent();
    private native void finalizeEvent();
    private native void waitingForKeyEvent(KeyEvent event);
}
