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
package com.joshvm.media;

import java.util.Hashtable;

class MediaEventThread extends Thread {
	public static final int EVENT_GROUP_PLAYER = 10;
	public static final int EVENT_GROUP_WAKEUP = 11;
	public static final int EVENT_GROUP_VAD = 12;

	public static final int EVENT_INVALID = -1;
	public static final int EVENT_PREPARE_COMPLETED = 1000;
	public static final int EVENT_SEEK_COMPLETED = 1001;
	public static final int EVENT_PLAY_COMPLETED = 1002;
	public static final int EVENT_WAKEUP = 1100;
	public static final int EVENT_VAD_BEGIN = 1200;
	public static final int EVENT_VAD_END = 1201;

	private static MediaEventThread thread;
	private Hashtable listeners = new Hashtable();
	private boolean isInited;
	private boolean isStop;

	static class MediaEvent {
		public int type = EVENT_INVALID;
		public int handle;
		public int intData;
	}

	interface EventListner {
		public void onEvent(MediaEvent event);
	}

	private static class ListenerKey {
		int group;
		int handle;

		public ListenerKey(int group, int handle) {
			this.group = group;
			this.handle = handle;
		}

		public int hashCode() {
			return group * 100000 + handle;
		}

		public boolean equals(Object obj) {
			if (obj instanceof ListenerKey) {
				return (group == ((ListenerKey) obj).group) && (handle == ((ListenerKey) obj).handle);
			}
			return false;
		}
	}

	static synchronized void updateListener(int group, int handle, EventListner listener) {
		if (thread == null) {
			if (listener == null) {
				return;
			}
			thread = new MediaEventThread();
			thread.initEvent();
		}

		if (listener == null) {
			thread.listeners.remove(new ListenerKey(group, handle));
			if (thread.listeners.size() == 0) {
				thread.isStop = true;
				thread.finalizeEvent();
			}
		} else {
			thread.listeners.put(new ListenerKey(group, handle), listener);
			thread.isStop = false;
			if (!thread.isAlive()) {
				thread.start();
			}
		}
	}

	public void run() {
		while (true) {
			MediaEvent event = new MediaEvent();
			waitingForEvent(event);
			synchronized (getClass()) {
				if (isStop) {
					thread = null;
					break;
				}
				if (event.type != EVENT_INVALID) {
					int group = event.type / 100;
					EventListner listener = (EventListner) listeners.get(new ListenerKey(group, event.handle));
					if (listener != null) {
						listener.onEvent(event);
					}
				}
			}
		}
	}

	private native void waitingForEvent(MediaEvent event);

	private native void initEvent();

	private native void finalizeEvent();
}
