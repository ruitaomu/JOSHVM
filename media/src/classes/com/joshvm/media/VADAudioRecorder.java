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

import java.io.IOException;

import org.joshvm.media.AudioRecorder;

public class VADAudioRecorder extends AudioRecorder {

	public VADAudioRecorder() throws IOException {
		super();
	}

	public void setAudioSampleRate(int sampleRateInHz) throws IllegalArgumentException {
		// ignore
	}

	public void setChannelConfig(int channelConfig) throws IllegalArgumentException {
		// ignore
	}

	public void setAudioBitRate(int audioBitRate) throws IllegalArgumentException {
		// ignore
	}

	public void startRecording() throws IOException {
		// ignore
	}

	public void stop() throws IOException {
		// ignore
	}
}
