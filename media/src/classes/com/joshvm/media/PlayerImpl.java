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
import java.io.InputStream;
import java.io.OutputStream;

import javax.microedition.io.Connector;
import javax.microedition.io.file.FileConnection;

import org.joshvm.media.AudioFormat;
import org.joshvm.media.MediaFormat;
import org.joshvm.media.MediaPlayerListener;

public class PlayerImpl {
	public static final int TYPE_MEDIA_PLAYER = 0;
	public static final int TYPE_MEDIA_RECORDER = 1;
	public static final int TYPE_AUDIO_TRACK = 2;
	public static final int TYPE_AUDIO_RECORDER = 3;
	public static final int TYPE_VAD_AUDIO_RECORDER = 4;

	public static final int STATE_STOPPED = 1;
	public static final int STATE_PAUSED = 2;
	public static final int STATE_STARTED = 3;
	public static final int STATE_UNKNOWN = 99;

	private int handle = -1; // native handle
	private boolean isOpen;
	private String oriSource;
	private String tempFile;
	private String resFile;
	private boolean isResource;

	public PlayerImpl(int type) throws IOException {
		init(type);
	}

	public PlayerImpl(int type, Class cls) throws IOException {
		if (type == TYPE_AUDIO_RECORDER && cls == VADAudioRecorder.class) {
			type = TYPE_VAD_AUDIO_RECORDER;
		}
		init(type);
	}

	private void init(int type) throws IOException {
		if (type != TYPE_MEDIA_PLAYER && type != TYPE_MEDIA_RECORDER && type != TYPE_AUDIO_TRACK
				&& type != TYPE_AUDIO_RECORDER && type != TYPE_VAD_AUDIO_RECORDER) {
			throw new IllegalArgumentException("Unsupported type: " + type);
		}

		try {
			handle = open0(type);
			isOpen = true;
		} catch (Exception e) {
			throw new IOException("Failed to create native player");
		}
	}

	public void setDataSource(String source) throws IOException {
		String path = null;
		boolean flag = false;
		String newSource = null;
		if (isValidPath(source)) {
			if (source.startsWith("file://")) {
				// file
				newSource = source.substring(7);
				if (!newSource.startsWith("/")) {
					newSource = "/" + newSource;
				}
			} else if (source.startsWith("res://")) {
				// resource
				path = source.substring(6);
				newSource = getTempFile0();
				int dotIndex = path.lastIndexOf('.');
				if (dotIndex != -1 && dotIndex != (path.length() - 1)) {
					if (path.indexOf('/', dotIndex) == -1) {
						newSource += path.substring(dotIndex);
					}
				}
				flag = true;
			} else if (source.startsWith("/") || source.indexOf("://", 1) != -1) {
				// file, http etc.
				newSource = source;
			}
		}

		if (newSource == null) {
			throw new IOException("invalid source");
		}

		setSource0(handle, newSource);

		removeTempfile();
		isResource = flag;
		if (flag) {
			tempFile = newSource;
			if (path.startsWith("/")) {
				resFile = path;
			} else {
				resFile = "/" + path;
			}
		}
		oriSource = source;
	}

	public void prepare(boolean blocking) throws IOException {
		if (isResource) {
			try {
				extractToTempfile();
			} catch (IOException ioe) {
				removeTempfile();
				throw ioe;
			}
		}
		prepare0(handle, blocking);
	}

	public void start() throws IOException {
		start0(handle);
	}

	public void pause() throws IOException {
		pause0(handle);
	}

	public void stop() throws IOException {
		stop0(handle);
	}

	public void reset() throws IOException {
		reset0(handle);
	}

	public synchronized void close() throws IOException {
		if (isOpen) {
			removeTempfile();
			close0(handle);
			setMediaPlayerListener(null);
			handle = -1;
			isOpen = false;
		}
	}

	public int getCurMediaTime() {
		return getMediaTime0(handle);
	}

	public void setCurMediaTime(int ms) throws IOException {
		setMediaTime0(handle, ms);
	}

	public int getDuration() {
		return getDuration0(handle);
	}

	public int getState() {
		int state = getState0(handle);
		if (state == STATE_STOPPED || state == STATE_PAUSED || state == STATE_STARTED) {
			return state;
		}
		return STATE_STOPPED;
	}

	public void setAudioSampleRate(int hz) throws IllegalArgumentException {
		if ((hz < 4000) || (hz > 48000)) {
			throw new IllegalArgumentException(hz + "Hz is not supported.");
		}
		setAudioSampleRate0(handle, hz);
	}

	public void setChannelConfig(int channelConfig) throws IllegalArgumentException {
		if (channelConfig != AudioFormat.CHANNEL_MONO && channelConfig != AudioFormat.CHANNEL_STEREO) {
			throw new IllegalArgumentException("Unsupported channel configuration.");
		}
		setChannelConfig0(handle, channelConfig);
	}

	public void setAudioBitRate(int rate) throws IllegalArgumentException {
		if ((rate < 8) || (rate > 32) || (rate % 8 != 0)) {
			throw new IllegalArgumentException(rate + " is not supported.");
		}
		setAudioBitRate0(handle, rate);
	}

	public void setOutputFile(String file) throws IOException {
		setOutputFile0(handle, file);
	}

	public void setOutputFormat(int format) {
		if (format != MediaFormat.FORMAT_WAV && format != MediaFormat.FORMAT_AMR_NB
				&& format != MediaFormat.FORMAT_AMR_WB) {
			throw new IllegalArgumentException("Unsupported output format.");
		}
		setOutputFormat0(handle, format);
	}

	public int getBufferSize() {
		return getBufferSize0(handle);
	}

	public int write(byte[] data, int offset, int size) throws IOException {
		int check = data[offset] + data[size - 1] + data[offset + size - 1];
		return write0(handle, data, offset, size);
	}

	public void flush() throws IOException {
		flush0(handle);
	}

	public int read(byte[] data, int offset, int size) throws IOException {
		int check = data[offset] + data[size - 1] + data[offset + size - 1];
		return read0(handle, data, offset, size);
	}

	public void setMediaPlayerListener(MediaPlayerListener listener) {
		EventListenerProxy.setMediaPlayerListener(handle, listener);
	}

	private boolean isValidPath(String path) {
		if (path == null || path.length() < 1) {
			return false;
		}
		if (path.indexOf("/../") != -1 || path.indexOf("/./") != -1 || path.endsWith("/..") || path.endsWith("/.")
				|| path.indexOf("\\") != -1) {
			return false;
		}
		return true;
	}

	private void removeTempfile() {
		if (tempFile == null || tempFile.length() == 0) {
			return;
		}
		FileConnection fc = null;

		try {
			fc = (FileConnection) Connector.open("file://" + tempFile);
			if (fc.exists()) {
				fc.delete();
				tempFile = null;
			}
		} catch (Exception e) {
		} finally {
			if (fc != null) {
				try {
					fc.close();
				} catch (IOException e) {
				}
			}
		}
	}

	private void extractToTempfile() throws IOException {
		FileConnection fc = null;
		OutputStream fileOs = null;
		InputStream resIs = null;

		try {
			resIs = getClass().getResourceAsStream(resFile);

			fc = (FileConnection) Connector.open("file://" + tempFile);
			if (!fc.exists()) {
				fc.create();
			}
			fileOs = fc.openOutputStream();

			byte[] buf = new byte[2048];
			int len;
			while ((len = resIs.read(buf)) != -1) {
				if (len > 0) {
					fileOs.write(buf, 0, len);
				}
			}
		} catch (Exception e) {
			throw new IOException("can not buffer res file.");
		} finally {
			if (resIs != null) {
				try {
					resIs.close();
				} catch (IOException e) {
				}
			}
			if (fileOs != null) {
				try {
					fileOs.close();
				} catch (IOException e) {
				}
			}
			if (fc != null) {
				try {
					fc.close();
				} catch (IOException e) {
				}
			}
		}
	}

	private native int open0(int type);

	private native void close0(int handle);

	private native void finalize();

	private native void setSource0(int handle, String source);

	private native void prepare0(int handle, boolean blocking);

	private native void start0(int handle);

	private native void pause0(int handle);

	private native void stop0(int handle);

	private native void reset0(int handle);

	private native int getState0(int handle);

	private native int getMediaTime0(int handle);

	private native void setMediaTime0(int handle, int ms);

	private native int getDuration0(int handle);

	private native void setAudioSampleRate0(int handle, int hz);

	private native void setChannelConfig0(int handle, int channel);

	private native void setAudioBitRate0(int handle, int rate);

	private native void setOutputFile0(int handle, String file);

	private native void setOutputFormat0(int handle, int format);

	private native int getBufferSize0(int handle);

	private native int write0(int handle, byte[] data, int offset, int size);

	private native void flush0(int handle);

	private native int read0(int handle, byte[] data, int offset, int size);

	private native String getTempFile0();

}
