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
 * MediaPlayer is used to control playback of audio files.
 */
public class MediaPlayer {
	private PlayerImpl player;
	private String source;

	/**
	 * Creates an MediaPlayer.<br>
	 * Call {@link #release()} to release native resources after using the
	 * MediaPlayer.
	 *
	 * @throws IOException if failed to create player
	 */
	public MediaPlayer() throws IOException {
		player = new PlayerImpl(PlayerImpl.TYPE_MEDIA_PLAYER);
	}

	/**
	 * Sets the data source. <br>
	 * <br>
	 * Data source format examples:<br>
	 * 
	 * <pre>
	 * <b>[local file]</b>      file:///sdcard/1.wav   or   /sdcard/1.wav
	 * <b>[file in jar]</b>     res://1.wav
	 * <b>[http URL]</b>        http://xx.com/1.wav
	 * </pre>
	 * 
	 * @param source the path of the file or URL of the data
	 * @throws IOException
	 */
	public void setDataSource(String source) throws IOException {
		player.setDataSource(source);
		this.source = source;
	}

	/**
	 * Prepares the player for playback, asynchronously.<br>
	 * When the media source is ready for playback,
	 * {@link MediaPlayerListener#onPrepared()} will be called.
	 * 
	 * @throws IOException if data source has not set or an I/O error occurs
	 * @see #setMediaPlayerListener
	 */
	public void prepare() throws IOException {
		if (source == null) {
			throw new IOException("no data source set.");
		}
		player.prepare(false);
	}

	/**
	 * Starts or resumes playback. When the end of a media source is reached,
	 * {@link MediaPlayerListener#onCompletion()} will be called.
	 * 
	 * @throws IOException
	 */
	public void start() throws IOException {
		player.start();
	}

	/**
	 * Pauses playback. Call start() to resume.
	 * 
	 * @throws IOException
	 */
	public void pause() throws IOException {
		player.pause();
	}

	/**
	 * Stops playback.<br>
	 * Need to call prepare before start again.
	 * 
	 * @throws IOException
	 */
	public void stop() throws IOException {
		player.stop();
	}

	/**
	 * Resets the MediaPlayer to its uninitialized state. After calling this
	 * method, you will have to initialize it again by setting the data source
	 * and calling prepare().
	 * 
	 * @throws IOException
	 */
	public void reset() throws IOException {
		player.reset();
	}

	/**
	 * Releases the native MediaPlayer resources. The object can no longer be
	 * used.
	 * 
	 * @throws IOException
	 */
	public void release() throws IOException {
		player.close();
	}

	/**
	 * Gets the current playback position.
	 *
	 * @return the current position in milliseconds, if no position is
	 *         available, -1 is returned.
	 */
	public int getCurrentPosition() {
		return player.getCurMediaTime();
	}

	/**
	 * Gets the total duration of the media source.
	 *
	 * @return the duration in milliseconds, if no duration is available, -1 is
	 *         returned.
	 */
	public int getDuration() {
		return player.getDuration();
	}

	/**
	 * Checks whether the MediaPlayer is playing.
	 *
	 * @return true if currently playing, false otherwise
	 */
	public boolean isPlaying() {
		return player.getState() == PlayerImpl.STATE_STARTED;
	}

	/**
	 * Seeks to specified time position. When the seek operation is completed,
	 * {@link MediaPlayerListener#onSeekComplete()} will be called.
	 * 
	 * @param ms the offset in milliseconds from the start to seek to
	 * @throws IOException
	 */
	public void seekTo(int ms) throws IOException {
		player.setCurMediaTime(ms);
	}

	/**
	 * Register a callback to be invoked when the media source is ready, or the
	 * seek operation is completed, or the end of a media source is reached.
	 * 
	 * @param listener the callback that will be run.
	 */
	public void setMediaPlayerListener(MediaPlayerListener listener) {
		player.setMediaPlayerListener(listener);
	}
}
