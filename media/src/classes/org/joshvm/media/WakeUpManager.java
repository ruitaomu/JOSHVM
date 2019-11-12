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
import java.util.Hashtable;

import com.joshvm.media.EventListenerProxy;

/**
 * WakeUpManager provides access to wakeup word detect
 */
public class WakeUpManager {
	private static final WakeUpManager INSTANCE = new WakeUpManager();

	private WakeUpManager() {
	}

	/**
	 * Wakeup word info include index, word, description.
	 *
	 */
	public class WakeUpWord {
		/**
		 * index
		 */
		public int index = -1;
		/**
		 * word
		 */
		public String word;
		/**
		 * description
		 */
		public String description;
	}

	/**
	 * Returns WakeUpManager instance
	 */
	public static WakeUpManager getInstance() {
		return INSTANCE;
	}

	/**
	 * Enable detect wakeup word.
	 * 
	 * @throws IOException if failed to start wakeup word detect
	 */
	public void enable() throws IOException {
		enable0(true);
	}

	/**
	 * Disable detect wakeup word.
	 * 
	 * @throws IOException if error occurs.
	 */
	public void disable() throws IOException {
		enable0(false);
	}

	/**
	 * Register a callback to be invoked when a wakeup word has been detected.
	 * 
	 * @param listener the callback that will be run.
	 * @see WakeUpListener
	 */
	public void setWakeUpListener(WakeUpListener listener) {
		EventListenerProxy.setWakeUpListener(listener);
	}

	/**
	 * Returns all the supported wakeup words.
	 * 
	 * @return Hashtable of wakeup words. Key of the Hashtable is index of
	 *         wakeup word, and value is {@link WakeUpManager.WakeUpWord
	 *         WakeUpWord} object.
	 */
	public Hashtable getWakeUpWords() {
		Hashtable table = new Hashtable();
		int count = getWordCount();
		if (count > 0) {
			WakeUpWord[] words = new WakeUpWord[count];
			for (int i = 0; i < count; i++) {
				words[i] = new WakeUpWord();
			}
			getWords(words);
			for (int j = 0; j < count; j++) {
				if (words[j].index >= 0) {
					table.put(new Integer(words[j].index), words[j]);
				}
			}
		}
		return table;
	}

	private native void enable0(boolean enable);

	private native int getWordCount();

	private native void getWords(WakeUpWord[] words);
}
