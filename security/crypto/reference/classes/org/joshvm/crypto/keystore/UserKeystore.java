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
import java.util.Enumeration;
import javax.microedition.io.SecurityInfo;
import com.sun.midp.ssl.*;

import org.bouncycastle.crypto.params.*;
import org.bouncycastle.asn1.ASN1Sequence;
import org.bouncycastle.util.encoders.*;
import org.bouncycastle.asn1.pkcs.RSAPrivateKey;

class UserKeystore implements PrivateKeyStore {
    private org.bouncycastle.crypto.tls.Certificate certificate = null;
    private AsymmetricKeyParameter key = null;
        
    public void load(KeyStoreParameter param) throws IllegalArgumentException {
        if (param instanceof UserKeyStoreParam) {
            org.bouncycastle.asn1.x509.Certificate[] holder = null;
            UserKeyStoreParam p = (UserKeyStoreParam)param;
            
            Vector certlist = p.getCertificateList();
            if (certlist == null) {
                throw new IllegalArgumentException("No certificate found");
            }
            
            int size = certlist.size();
            if (size > 0) {
                holder = new org.bouncycastle.asn1.x509.Certificate[size];
            } else {
                throw new IllegalArgumentException("Empty certificate list");
            }
            
            Enumeration e = certlist.elements();
            int i = 0;
            while (e.hasMoreElements()) {
                ASN1Sequence seq;            
                Object cert = e.nextElement();
                try {
                    byte[] base64decoded;
                    if (cert instanceof byte[]) {
                        base64decoded = Base64.decode((byte[])cert);
                    } else if (cert instanceof String) {
                        base64decoded = Base64.decode((String)cert);
                    } else {
                        throw new IllegalArgumentException("Wrong parameter type of certificate");
                    }
                    seq = ASN1Sequence.getInstance(base64decoded);
                    org.bouncycastle.asn1.x509.Certificate obj = org.bouncycastle.asn1.x509.Certificate.getInstance(seq); 
                    holder[i++] = obj;
                } catch (DecoderException ex) {
                    //skip badly decoded base64 data
                }
            }

            if (i == 0) {
                throw new IllegalArgumentException("No valid certificate found");
            }
            
            certificate = new org.bouncycastle.crypto.tls.Certificate(holder);

            //Get private key
            Object privateKey = p.getPrivateKey();
            if (privateKey != null) {
                try {
                    byte[] base64decoded;
                    if (privateKey instanceof byte[]) {
                        base64decoded = Base64.decode((byte[])privateKey);
                    } else if (privateKey instanceof String) {
                        base64decoded = Base64.decode((String)privateKey);
                    } else {
                        throw new IllegalArgumentException("Wrong parameter type of key");
                    }
                    RSAPrivateKey keyStructure = RSAPrivateKey.getInstance((Object)base64decoded);
                    key = new RSAPrivateCrtKeyParameters(keyStructure.getModulus(),
                                keyStructure.getPublicExponent(), keyStructure.getPrivateExponent(),
                                keyStructure.getPrime1(), keyStructure.getPrime2(), keyStructure.getExponent1(),
                                keyStructure.getExponent2(), keyStructure.getCoefficient());
                    
                } catch (DecoderException ex) {
                    throw new IllegalArgumentException("Invalid private key");
                }
                return;
            }
        }

        throw new IllegalArgumentException("Not a UserKeystoreParam type");
    }

    public org.bouncycastle.crypto.tls.Certificate getCertificate() {
        return certificate;
    }
    
    public AsymmetricKeyParameter getPrivateKey() {
        return key;
    }

}

