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

package com.sun.midp.crypto;

import com.joshvm.java.math.BigInteger;
import java.io.IOException;

public class BouncyCastleRSAPublicKey 
	extends org.bouncycastle.asn1.pkcs.RSAPublicKey implements PublicKey {

    public BouncyCastleRSAPublicKey(
        BigInteger modulus,
        BigInteger publicExponent)
    {
        super(modulus, publicExponent);
    }
	
	//
	//Overrive interface Key.java
	//

	/**
     * Gets the type of the current key.
     * @return the type of the key
     */    
    public String getAlgorithm() {
	    return "RSA";
    }

    /** 
     * Returns the name of the encoding format for this key.
     *
     * @return the string "RAW".
     */
    public String getFormat() {	
        return "RAW"; 
    }

    /**
     * Returns the encoding of key.
     *
     * @return if OCSP is enabled, returns DER encoding of this key,
     *         otherwise returns null
     */
    public byte[] getEncoded() {
    	byte[] res = null;
    	try {
    		res = toASN1Primitive().getEncoded();
    	} catch (IOException e) {
    	}

		return res;
    }
}

