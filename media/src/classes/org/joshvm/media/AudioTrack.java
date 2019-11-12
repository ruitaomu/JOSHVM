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
 * AudioTrack is used to play a single audio resource.
 */
public class AudioTrack {
	/**
	 * indicates AudioTrack state is stopped
	 */
	public static final int PLAYSTATE_STOPPED = 1;
	/**
	 * indicates AudioTrack state is paused
	 */
	public static final int PLAYSTATE_PAUSED = 2;
	/**
	 * indicates AudioTrack state is playing
	 */
	public static final int PLAYSTATE_PLAYING = 3;

	private PlayerImpl player;

	/**
	 * Creates an AudioTrack.<br>
	 * Call {@link #release()} to release native resources after using it.
	 * 
	 * @throws IOException if failed to create AudioTrack
	 */
	public AudioTrack() throws IOException {
		player = new PlayerImpl(PlayerImpl.TYPE_AUDIO_TRACK);
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
	 * Returns native buffer size.
	 */
	public int getBufferSize() {
		return player.getBufferSize();
	}

	/**
	 * Returns play state.
	 * 
	 * @see #PLAYSTATE_STOPPED
	 * @see #PLAYSTATE_PAUSED
	 * @see #PLAYSTATE_PLAYING
	 */
	public int getPlayState() {
		return player.getState();
	}

	/**
	 * Writes the audio data to the audio sink for playback, block until all
	 * data has been written to the audio sink.
	 * 
	 * @param audioData the array that holds the data to play.
	 * @param offset the start offset in the <code>audioData</code>
	 * @param size the number of bytes to write
	 * @return number of bytes written
	 * @throws IOException if an I/O error occurs or player has been stopped.
	 */
	public int write(byte[] audioData, int offset, int size) throws IOException {
		int total = 0;
		while (total < size) {
			int count = player.write(audioData, offset, size);
			total += count;
		}
		return total;
	}

	/**
	 * Start playing
	 * 
	 * @throws IOException if an I/O error occurs
	 */
	public void play() throws IOException {
		player.start();
	}

	/**
	 * Pauses the playback of the audio data. Data that has not been played back
	 * will not be discarded. Subsequent calls to {@link #play()} will play this
	 * data back. See {@link #flush()} to discard this data.
	 * 
	 * @throws IOException if an I/O error occurs
	 */
	public void pause() throws IOException {
		player.pause();
	}

	/**
	 * Flushes the audio data currently queued for playback. Any data that has
	 * not been played back will be discarded. No-op if not stopped or paused.
	 * 
	 * @throws IOException if an I/O error occurs
	 */
	public void flush() throws IOException {
		player.flush();
	}

	/**
	 * Stops playing the audio data. Audio will stop playing after the last
	 * buffer that was written has been played. For an immediate stop, use
	 * {@link #pause()}, followed by {@link #flush()} to discard audio data that
	 * hasn't been played back yet.
	 * 
	 * @throws IOException if an I/O error occurs
	 */
	public void stop() throws IOException {
		player.stop();
	}

	/**
	 * Releases the native AudioTrack resources.
	 * 
	 * @throws IOException if an I/O error occurs
	 */
	public void release() throws IOException {
		player.close();
	}
}
