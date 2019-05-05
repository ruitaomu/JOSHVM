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
package org.joshvm.crypto.keystore;

import com.sun.midp.publickeystore.WebPublicKeyStore;

/**
 * The utility class to initialize WebPublicKeyStore with user-specified keystore file
 *
 */

public class KeyStore {

	/**
	 * Initialized WebPublicKeyStore with the specified keystore file from resource
	 *
	 * @param classToResource the class of the loading resource file
	 * @param pathToResource path of the resource file of keystore
	 *
	 * @throws NullPointerException {@code classToResource} or {@code pathToResource} is null.
	 *
	 */
	public static void initWebPublicKeystoreLocation(Class classToResource, String pathToResource) {
		if (classToResource == null) {
			throw new NullPointerException("Class can not be null");
		}

		if (pathToResource == null) {
			throw new NullPointerException("Resource path can not be null");
		}
		
		WebPublicKeyStore.initKeystoreLocation(classToResource, pathToResource);
	}
}
