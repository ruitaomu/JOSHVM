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

import org.bouncycastle.crypto.digests.SHA1Digest;

/**
 * Implements the SHA-1 message digest algorithm.
 */ 
final class SHA extends MessageDigest {

	private SHA1Digest impl;

    /** Create SHA digest object. */
    SHA() {
        impl = new SHA1Digest();
    }

	SHA(SHA1Digest another_digest) {
		impl = another_digest;
	}

    /** 
     * Gets the message digest algorithm.
     * @return algorithm implemented by this MessageDigest object
     */
    public String getAlgorithm() {
        return "SHA-1";
    }

    /** 
     * Gets the length (in bytes) of the hash.
     * @return byte-length of the hash produced by this object
     */
    public int getDigestLength() {
        return 20;
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
		return new SHA(new SHA1Digest(impl));
    }
}

