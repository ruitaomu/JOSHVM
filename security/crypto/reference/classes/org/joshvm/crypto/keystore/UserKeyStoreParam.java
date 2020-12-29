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

import java.util.Vector;

public class UserKeyStoreParam implements KeyStoreParameter {
    private Vector certs;
    private Object key;

    private UserKeyStoreParam() {
        certs = new Vector();        
    }

    public static UserKeyStoreParam Build() throws IllegalArgumentException {
        return new UserKeyStoreParam();
    }

    /**
     * Add base64-encoded X509 certificate into the KeyStoreParam to setup KeyStore
     *
     */
    public UserKeyStoreParam addCertificate(String cert) {
        certs.addElement(cert);
        return this;
    }

    /**
     * Add base64-encoded X509 certificate into the KeyStoreParam to setup KeyStore
     *
     */
    public UserKeyStoreParam addCertificate(byte[] cert) {
        certs.addElement(cert);
        return this;
    }

    /**
     * Set base64-encoded private key of the KeyStoreParam to setup KeyStore
     *
     */
    public UserKeyStoreParam setPrivateKey(String key) {
        this.key = key;
        return this;
    }

    /**
     * Set base64-encoded private key of the KeyStoreParam to setup KeyStore
     *
     */
    public UserKeyStoreParam setPrivateKey(byte[] key) {
        this.key = key;
        return this;
    }

    /**
     * Get the base64-encoded private key for loading UserKeyStore
     *
     */
    public Object getPrivateKey() {
        return key;
    }

    /**
     * Get the list of base64 encoded certificates for loading UserKeyStore
     *
     */
    public Vector getCertificateList() {
        return certs;
    }
}

