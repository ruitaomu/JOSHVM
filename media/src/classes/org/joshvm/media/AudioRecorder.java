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

import com.joshvm.media.PlayerImpl;

/**
 * AudioRecorder is used to record audio from the audio input hardware of the
 * platform.
 */
public class AudioRecorder {
	/**
	 * The state indicating that the recorder is not recording
	 */
	public static final int RECORDSTATE_STOPPED = 1;
	/**
	 * The state indicating that the recorder is recording
	 */
	public static final int RECORDSTATE_RECORDING = 3;

	private PlayerImpl player;

	/**
	 * Creates an AudioRecorder.<br>
	 * Call {@link #release()} to release native resources after using the
	 * recorder.
	 *
	 * @throws IOException if failed to create recorder
	 */
	public AudioRecorder() throws IOException {
		player = new PlayerImpl(PlayerImpl.TYPE_AUDIO_RECORDER, getClass());
	}

	/**
	 * Set audio sample rate in Hertz.
	 * 
	 * @param sampleRateInHz sample rate. Normally use <code>8000</code>,
	 *            <code>16000</code>, <code>22050</code>, <code>44100</code>,
	 *            <code>48000</code>. Default is <code>16000</code>.
	 * @throws IllegalArgumentException
	 */
	public void setAudioSampleRate(int sampleRateInHz) throws IllegalArgumentException {
		player.setAudioSampleRate(sampleRateInHz);
	}

	/**
	 * Set the configuration of the audio channels.
	 * 
	 * @param channelConfig audio channel configuration. See
	 *            {@link AudioFormat#CHANNEL_MONO} and
	 *            {@link AudioFormat#CHANNEL_STEREO}. Default is
	 *            <code>CHANNEL_MONO</code>
	 * @throws IllegalArgumentException
	 */
	public void setChannelConfig(int channelConfig) throws IllegalArgumentException {
		player.setChannelConfig(channelConfig);
	}

	/**
	 * Set bit rate.
	 * 
	 * @param audioBitRate bit per sample. Normally use <code>8</code>,
	 *            <code>16</code>. Default is <code>16</code>
	 * @throws IllegalArgumentException
	 */
	public void setAudioBitRate(int audioBitRate) throws IllegalArgumentException {
		player.setAudioBitRate(audioBitRate);
	}

	/**
	 * Returns the recording state.
	 * 
	 * @see #RECORDSTATE_STOPPED
	 * @see #RECORDSTATE_RECORDING
	 */
	public int getRecordingState() {
		return player.getState();
	}

	/**
	 * Reads up to <code>size</code> bytes of audio data from the audio hardware
	 * for recording into a buffer, blocks until at least one byte is read or no
	 * more data because recorder has been stopped.
	 * 
	 * @param audioData the array to which the recorded audio data is written.
	 * @param offset the start offset in array <code>audioData</code> at which
	 *            the data is written
	 * @param size the maximum number of bytes to read
	 * @return the total number of bytes read into the buffer, or
	 *         <code>-1</code> for end
	 * @throws IOException if an I/O error occurs
	 */
	public int read(byte[] audioData, int offset, int size) throws IOException {
		return player.read(audioData, offset, size);
	}

	/**
	 * Start recording
	 * 
	 * @throws IOException if an I/O error occurs
	 */
	public void startRecording() throws IOException {
		player.start();
	}

	/**
	 * Stop recording
	 * 
	 * @throws IOException if an I/O error occurs
	 */
	public void stop() throws IOException {
		player.stop();
	}

	/**
	 * Releases the native resources. The object can no longer be used and the
	 * reference should be set to null after a call to release().
	 * 
	 * @throws IOException if an I/O error occurs
	 */
	public void release() throws IOException {
		player.close();
	}
}
