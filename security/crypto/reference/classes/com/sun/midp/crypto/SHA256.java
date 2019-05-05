/*
 *   
 *
 * Copyright  1990-2009 Sun Microsystems, Inc. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version
 * 2 only, as published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License version 2 for more details (a copy is
 * included at /legal/license.txt).
 * 
 * You should have received a copy of the GNU General Public License
 * version 2 along with this work; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 * 
 * Please contact Sun Microsystems, Inc., 4150 Network Circle, Santa
 * Clara, CA 95054 or visit www.sun.com if you need additional
 * information or have any questions.
 */

package com.sun.midp.crypto;

import org.bouncycastle.crypto.digests.SHA256Digest;

/**
 * Implements the SHA-256 message digest algorithm.
 */ 
final class SHA256 extends MessageDigest {

	private SHA256Digest impl;

    /** Create SHA digest object. */
    SHA256() {
        impl = new SHA256Digest();
    }

	SHA256(SHA256Digest another_digest) {
		impl = another_digest;
	}

    /** 
     * Gets the message digest algorithm.
     * @return algorithm implemented by this MessageDigest object
     */
    public String getAlgorithm() {
        return "SHA-256";
    }

    /** 
     * Gets the length (in bytes) of the hash.
     * @return byte-length of the hash produced by this object
     */
    public int getDigestLength() {
        return 32;
    }

    /** 
     * Resets the MessageDigest to the initial state for further use.
     */
    public void reset() {
    	impl.reset();
    }

    /**
     * Accumulates a hash of the input data. This method is useful when
     * the input data to be hashed is not available in one byte array. 
     * @param inBuf input buffer of data to be hashed
     * @param inOff offset within inBuf where input data begins
     * @param inLen length (in bytes) of data to be hashed
     * @see #doFinal(byte[], int, int, byte[], int)
     */
    public void update(byte[] inBuf, int inOff, int inLen) {
		impl.update(inBuf, inOff, inLen);
    }

    /**
     * Completes the hash computation by performing final operations
     * such as padding. The digest is reset after this call is made.
     *
     * @param buf output buffer for the computed digest
     *
     * @param offset offset into the output buffer to begin storing the digest
     *
     * @param len number of bytes within buf allotted for the digest
     *
     * @return the number of bytes placed into <code>buf</code>
     * 
     * @exception DigestException if an error occurs.
     */
    public int digest(byte[] buf, int offset, int len) throws DigestException {
    	return impl.doFinal(buf, offset);
    }

    /** 
     * Clones the MessageDigest object.
     * @return a clone of this object
     */
    public Object clone() {
		return new SHA256(new SHA256Digest(impl));
    }
}
