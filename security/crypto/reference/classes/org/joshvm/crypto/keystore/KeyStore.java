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
import java.util.Hashtable;

/**
 * The utility class to initialize WebPublicKeyStore and get specified PrivateKeyStore.
 *
 */

public class KeyStore {
    private final static Hashtable keyStores = new Hashtable();
    private static PrivateKeyStore selectedKeyStore = null;
    
    private final static String DEFAULT_PRIVATE_STORE_PACKAGE_NAME = "org.joshvm.crypto.keystore";
    private final static String DEFAULT_PRIVATE_STORE_CLASS_NAME = "UserKeystore";    

	/**
	 * Initialized WebPublicKeyStore with the user-specified keystore file from resource
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

    /**
     * Select a private key store to use
     *
     * @param privateKeyStoreType Private key store provider, null means that default key store type is used.
     * 
     * @return The key store for storage of private keys and certificates. Return null if the specified key store type not available.
     *
     */
    public static PrivateKeyStore selectPrivateKeyStore(String privateKeyStoreType) {
        if (privateKeyStoreType == null) {
            privateKeyStoreType = DEFAULT_PRIVATE_STORE_CLASS_NAME;
        }

        try {
            Class clazz = Class.forName(DEFAULT_PRIVATE_STORE_PACKAGE_NAME+"."+privateKeyStoreType);
            selectedKeyStore = (PrivateKeyStore)clazz.newInstance();
            return selectedKeyStore;
        } catch (Exception e) {
            return null;
        }
    }

    public static PrivateKeyStore getSelectedPrivateKeyStore() {
        return selectedKeyStore;
    }
}
