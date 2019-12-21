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

import org.joshvm.media.AudioRecorder;
import org.joshvm.media.MediaPlayerListener;
import org.joshvm.media.VADControllerListener;
import org.joshvm.media.WakeUpListener;

import com.joshvm.media.MediaEventThread.MediaEvent;

public class EventListenerProxy {

	public static void setMediaPlayerListener(int id, final MediaPlayerListener listener) {
		MediaEventThread.updateListener(MediaEventThread.EVENT_GROUP_PLAYER, id,
				listener == null ? null : new MediaEventThread.EventListner() {
					public void onEvent(MediaEvent event) {
						int eventType = event.type;
						if (eventType == MediaEventThread.EVENT_PREPARE_COMPLETED) {
							listener.onPrepared();
						} else if (eventType == MediaEventThread.EVENT_SEEK_COMPLETED) {
							listener.onSeekComplete();
						} else if (eventType == MediaEventThread.EVENT_PLAY_COMPLETED) {
							listener.onCompletion();
						}
					}
				});
	}

	public static void setWakeUpListener(int id, final WakeUpListener listener) {
		MediaEventThread.updateListener(MediaEventThread.EVENT_GROUP_WAKEUP, id,
				listener == null ? null : new MediaEventThread.EventListner() {
					public void onEvent(MediaEvent event) {
						int eventType = event.type;
						int index = event.intData;
						if (eventType == MediaEventThread.EVENT_WAKEUP) {
							listener.onWakeUp(index);
						}
					}
				});
	}

	public static void setWakeUpListener(WakeUpListener listener) {
		setWakeUpListener(0, listener);
	}

	public interface VADControllerListenerExt extends VADControllerListener {
		public AudioRecorder getAudioRecorder();
	}

	public static void setVADControllerListener(int id, final VADControllerListenerExt listener) {
		MediaEventThread.updateListener(MediaEventThread.EVENT_GROUP_VAD, id,
				listener == null ? null : new MediaEventThread.EventListner() {
					public void onEvent(MediaEvent event) {
						int eventType = event.type;
						if (eventType == MediaEventThread.EVENT_VAD_BEGIN) {
							listener.onVADBegin(listener.getAudioRecorder());
						} else if (eventType == MediaEventThread.EVENT_VAD_END) {
							listener.onVADEnd();
						}
					}
				});
	}

	public static void setVADControllerListener(VADControllerListenerExt listener) {
		setVADControllerListener(0, listener);
	}
}
