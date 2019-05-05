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

package com.sun.midp.pki;

import com.sun.midp.pki.X509Certificate;
import java.io.IOException;

public class BouncyCastleX509Certificate {
	private X509Certificate certificate;
	
	public BouncyCastleX509Certificate(org.bouncycastle.asn1.x509.Certificate cert) throws IOException {
		byte[] buf = cert.getEncoded();
		this.certificate = X509Certificate.generateCertificate(buf, 0, buf.length);
	}

	public BouncyCastleX509Certificate(X509Certificate cert) {
		this.certificate = cert;
	}
	
	public X509Certificate getJOSHX509Certificate() {
		return certificate;
	}
}

