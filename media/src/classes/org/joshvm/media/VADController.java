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
package org.joshvm.media;

import java.io.IOException;

import com.joshvm.media.EventListenerProxy;
import com.joshvm.media.VADAudioRecorder;

/**
 * VADController provides access to vad control.
 */
public class VADController {
	private static final VADController INSTANCE = new VADController();
	private AudioRecorder recorder;

	private VADController() {
	}

	/**
	 * Returns VADController instance
	 */
	public static VADController getInstance() {
		return INSTANCE;
	}

	/**
	 * Set vad off time
	 * 
	 * @param timeout vad off time in milliseconds
	 */
	public void setTimeout(int timeout) {
		setTimeout0(timeout);
	}

	/**
	 * Register a callback to be invoked when native vad engine is in begin or
	 * end state.
	 * 
	 * @param listener the callback that will be run.
	 */
	public void setVADControllerListener(final VADControllerListener listener) {
		EventListenerProxy
				.setVADControllerListener(listener == null ? null : new EventListenerProxy.VADControllerListenerExt() {
					public void onVADBegin(AudioRecorder audioRecorder) {
						listener.onVADBegin(audioRecorder);
					}

					public void onVADEnd() {
						listener.onVADEnd();
					}

					public AudioRecorder getAudioRecorder() {
						return recorder;
					}
				});
	}

	/**
	 * Starts vad controller.
	 * 
	 * @throws IOException
	 */
	public void start() throws IOException {
		if (recorder == null) {
			try {
				recorder = new VADAudioRecorder();
			} catch (Exception e) {
				throw new IOException("Failed to start VADController.");
			}
		}
		start0();
	}

	/**
	 * Pauses vad controller.
	 * 
	 * @throws IOException
	 */
	public void pause() throws IOException {
		pause0();
	}

	/**
	 * Resumes vad controller.
	 * 
	 * @throws IOException
	 */
	public void resume() throws IOException {
		resume0();
	}

	/**
	 * Stops vad controller.
	 * 
	 * @throws IOException
	 */
	public void stop() throws IOException {
		stop0();
		if (recorder != null) {
			try {
				recorder.release();
			} catch (Exception e) {
			}
			recorder = null;
		}
	}

	private native void setTimeout0(int timeout);

	private native void start0();

	private native void pause0();

	private native void resume0();

	private native void stop0();

	private native void finalize();
}
