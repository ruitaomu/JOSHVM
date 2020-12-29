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
 * MediaRecorder is used to record audio.
 */
public class MediaRecorder {
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
	 * Creates an MediaRecorder.<br>
	 * Call {@link #release()} to release native resources after using the
	 * MediaRecorder.
	 *
	 * @throws IOException if failed to create recorder
	 */
	public MediaRecorder() throws IOException {
		player = new PlayerImpl(PlayerImpl.TYPE_MEDIA_RECORDER);
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
	 * Sets the path of the output file to be produced. Call this after
	 * setOutputFormat() but before prepare(). <br>
	 * Root path can be got from {@link Environment#getRootDirectory()} or
	 * {@link Environment#getExternalStorageDirectory()}. <br>
	 * Output file can be accessed by FileConnection interface.
	 * 
	 * @param outputFilePath the pathname to use.
	 * @throws IOException
	 */
	public void setOutputFile(String outputFilePath) throws IOException {
		player.setOutputFile(outputFilePath);
	}

	/**
	 * Sets the format of the output file produced during recording.
	 * 
	 * @param outFormat the output format to use. See
	 *            {@link MediaFormat#FORMAT_WAV},
	 *            {@link MediaFormat#FORMAT_AMR_NB} and
	 *            {@link MediaFormat#FORMAT_AMR_WB}
	 * @throws IllegalArgumentException
	 */
	public void setOutputFormat(int outFormat) throws IllegalArgumentException {
		player.setOutputFormat(outFormat);
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
	 * Prepares the recorder to begin capturing and encoding data,
	 * synchronously.
	 * 
	 * @throws IOException if prepare fails
	 */
	public void prepare() throws IOException {
		player.prepare(true);
	}

	/**
	 * Begins capturing and encoding data to the file specified with
	 * setOutputFile().
	 * 
	 * @throws IOException if start fails
	 */
	public void start() throws IOException {
		player.start();
	}

	/**
	 * Stops recording. Once recording is stopped, you will have to configure it
	 * again as if it has just been constructed.
	 * 
	 * @throws IOException
	 */
	public void stop() throws IOException {
		player.stop();
	}

	/**
	 * Set the MediaRecorder to its idle state. After calling this method, you
	 * will have to configure it again as if it had just been constructed.
	 */
	public void reset() throws IOException {
		player.reset();
	}

	/**
	 * Releases the native MediaRecorder resources. The object can no longer be
	 * used.
	 * 
	 * @throws IOException
	 */
	public void release() throws IOException {
		player.close();
	}
}
