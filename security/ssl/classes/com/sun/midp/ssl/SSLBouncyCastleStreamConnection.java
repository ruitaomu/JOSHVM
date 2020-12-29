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

package com.sun.midp.ssl;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.util.Vector;

import javax.microedition.io.Connector;
import javax.microedition.io.SecurityInfo;
import javax.microedition.io.StreamConnection;
import javax.microedition.pki.Certificate;

import com.sun.midp.pki.*;

import org.joshvm.crypto.keystore.KeyStore;

import org.bouncycastle.crypto.tls.DefaultTlsClient;
import org.bouncycastle.crypto.tls.TlsAuthentication;
import org.bouncycastle.crypto.tls.TlsCredentials;
import org.bouncycastle.crypto.tls.TlsClientProtocol;
import org.bouncycastle.crypto.tls.CertificateRequest;
import org.bouncycastle.crypto.tls.TlsContext;
import org.bouncycastle.crypto.tls.DefaultTlsSignerCredentials;
import org.bouncycastle.crypto.tls.ProtocolVersion;
import org.bouncycastle.crypto.tls.SignatureAndHashAlgorithm;
import org.bouncycastle.crypto.tls.CipherSuite;
import org.bouncycastle.crypto.tls.SignatureAlgorithm;
import org.bouncycastle.crypto.tls.HashAlgorithm;



import org.bouncycastle.asn1.pkcs.RSAPrivateKey;
import org.bouncycastle.asn1.ASN1Sequence;
import org.bouncycastle.crypto.params.AsymmetricKeyParameter;
import org.bouncycastle.util.encoders.Base64;

public class SSLBouncyCastleStreamConnection implements StreamConnection {
    /** Indicates that a is ready to be opened. */
    static final int READY = 0;
    /** Indicates that a stream is opened. */
    static final int OPEN = 1;
    /** Indicates that a stream is closed. */
    static final int CLOSED = 2;

    /** Raw encrypted input stream. */
    private InputStream sin = null;
    /** Raw encrypted output stream. */
    private OutputStream sout = null;
    /** Current host name. */
    private String host = null;
    /** Current port number. */
    private int port = 0;
    /** Flag indicating the underlying TCP connection is open. */
    private boolean copen = false;
    /** Server certificate from a successful handshake. */
    private BouncyCastleX509Certificate serverCert;
    /** Cipher suite from a successful handshake. */
    private String cipherSuite;

    /*
     * The following are visible within the package so In and Out can 
     * manipulate them directly
     */ 
    /** State of the input stream given out by getInputStream. */
    int inputStreamState;
    /** State of the output stream given out by getOutputStream. */
    int outputStreamState;

	private org.bouncycastle.crypto.tls.Certificate certificate = null;
    private TlsClientProtocol tlsproto = null;
	private CertStore certStore;
    private int cipherSuiteCode = 0;
    
    protected String protoVersion = null;
    protected String protoName = null;
	
    /**
     * Establish and SSL session over a reliable stream.
     * This connection will forward the input and output stream close methods
     * to the given connection. If the caller wants to have the given
     * connection closed with this connection, the caller can close given
     * connection after constructing this connection, but leaving the closing
     * of the streams to this connection.
     * 
     * @param host hostname of the SSL server
     * @param port port number of the SSL server
     * @param in   InputStream associated with the StreamConnection
     * @param out  OutputStream associated with the StreamConnection
     * @param cs trusted certificate store to be used for this connection
     *
     * @exception IOException if there is a problem initializing the SSL
     * data structures or the SSL handshake fails
     */ 
    public SSLBouncyCastleStreamConnection(String host, int port,
                               InputStream in, OutputStream out, CertStore cs)
            throws IOException {
        
        if (cs == null) {
            throw new IllegalArgumentException(
                "SSLStreamConnection: no trusted certificate store given");
        }

		certStore = cs;
		
        if ((in == null) || (out == null)) {
            throw new IllegalArgumentException(
                "SSLStreamConnection: stream missing");
        }

        this.host = host;
        this.port = port;

        try {
            tlsproto = new TlsClientProtocol(in, out, new com.joshvm.java.security.SecureRandom());
			DefaultTlsClient client = new DefaultTlsClient() {
                public void notifyServerVersion(ProtocolVersion serverVersion)
                    throws IOException
                {
                    super.notifyServerVersion(serverVersion);
                    if (serverVersion != null) {
                        serverVersion = serverVersion.getEquivalentTLSVersion();
                    }
                    
                    if (ProtocolVersion.SSLv3.equals(serverVersion)) {
                        protoVersion = "3.0";
                        protoName = "SSL";
                    } else if (ProtocolVersion.TLSv10.equals(serverVersion)) {
                        protoVersion = "3.1";
                        protoName = "TLS";
                    } else if (ProtocolVersion.TLSv11.equals(serverVersion)) {
                        protoVersion = "3.2";
                        protoName = "TLSv1.1";
                    } else if (ProtocolVersion.TLSv12.equals(serverVersion)) {
                        protoVersion = "3.3";
                        protoName = "TLSv1.2";
                    } else {
                        throw new IOException("Unknown SSL/TLS version");
                    }
                }

                public void notifySelectedCipherSuite(int selectedCipherSuite)
                {
                    super.notifySelectedCipherSuite(selectedCipherSuite);

                    cipherSuiteCode = selectedCipherSuite;
                }
                
                public TlsAuthentication getAuthentication() throws IOException {
                    TlsAuthentication auth = new TlsAuthentication() {
                        public void notifyServerCertificate(
                                org.bouncycastle.crypto.tls.Certificate serverCertificate)
                                throws IOException {
                                                                // Capture the server certificate information!
                                certificate = serverCertificate;
                        }

                        public TlsCredentials getClientCredentials(
                                CertificateRequest certificateRequest) throws IOException {
                            return tlsSignerCredentials(context, new SignatureAndHashAlgorithm(HashAlgorithm.sha256, SignatureAlgorithm.rsa));
                        }
                    };
                    return auth;
                }
            };
			tlsproto.connect(client);
			if (certificate != null) {
            	org.bouncycastle.asn1.x509.Certificate[] certs = certificate.getCertificateList();
				serverCert = new BouncyCastleX509Certificate(validateCertChain(certs));
                if (cipherSuiteCode == CipherSuite.TLS_RSA_WITH_AES_128_CBC_SHA) {
    				cipherSuite = "TLS_RSA_WITH_AES_128_CBC_SHA"; //Hard coded for now???
                } else if (cipherSuiteCode == CipherSuite.TLS_RSA_WITH_AES_128_CBC_SHA256) {
                    cipherSuite = "TLS_RSA_WITH_AES_128_CBC_SHA256"; //Hard coded for now???
                } else if (cipherSuiteCode == CipherSuite.TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256) {
                    cipherSuite = "TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256"; //Hard coded for now???
                } else {
                    //See DefaultTlsClient.getCipherSuites()
                    throw new IOException("Unsupported CipherSuite:"+cipherSuiteCode);
                }
			} else {
				throw new IOException("No server certificate found!");
			}
			
			this.sin = tlsproto.getInputStream();
			this.sout = tlsproto.getOutputStream();
        } catch (IOException e) {
            cleanupIfNeeded();
            throw e;
        }

        copen = true;
    }


    
    private DefaultTlsSignerCredentials tlsSignerCredentials(TlsContext tlsContext, SignatureAndHashAlgorithm algorithm) {
        try {            
            return new DefaultTlsSignerCredentials(tlsContext,
                KeyStore.getSelectedPrivateKeyStore().getCertificate(), 
                KeyStore.getSelectedPrivateKeyStore().getPrivateKey(),
                algorithm);
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }
    }

	private X509Certificate validateCertChain(org.bouncycastle.asn1.x509.Certificate[] x509CertificateList) throws IOException {
		Vector certs = new Vector();
		for (int i = 0; i < x509CertificateList.length; i++) {
			X509Certificate x509Cert = new BouncyCastleX509Certificate(x509CertificateList[i]).getJOSHX509Certificate();
			certs.addElement(x509Cert);
		}
		
		X509Certificate.verifyChain(certs, -1,
            X509Certificate.SERVER_AUTH_EXT_KEY_USAGE, certStore);

        // The first cert if specified to be the server cert.
        return (X509Certificate)certs.elementAt(0);
	}
    
    /**
     * Returns the InputStream associated with this SSLStreamConnection.
     *
     * @return InputStream object from which SSL protected bytes can
     * be read
     * @exception IOException if the connection is not open or the stream was 
     * already open
     */ 
    synchronized public InputStream openInputStream() throws IOException {
        if (!copen) {
            throw new IOException("Connection closed");
        }

        if (inputStreamState != READY) {
            throw new IOException("Input stream already opened");
        }

        inputStreamState = OPEN;
        return (sin);
    }
    
    /**
     * Returns the OutputStream associated with this SSLStreamConnection.
     * 
     * @return OutputStream object such that bytes written to this stream
     * are sent over an SSL secured channel
     * @exception IOException if the connection is not open or the stream was 
     * already open
     */
    synchronized public OutputStream openOutputStream() throws IOException {
        if (!copen) {
            throw new IOException("Connection closed");
        }

        if (outputStreamState != READY) {
            throw new IOException("Output stream already opened");
        }

        outputStreamState = OPEN;
        return (sout);
    }
    
    /**
     * Returns the DataInputStream associated with this SSLStreamConnection.
     * @exception IOException if the connection is not open or the stream was 
     * already open
     * @return a DataInputStream object
     */ 
    public DataInputStream openDataInputStream() throws IOException {
	return (new DataInputStream(openInputStream()));
    }
     
    /** 
     * Returns the DataOutputStream associated with this SSLStreamConnection.
     * @exception IOException if the connection is not open or the stream was 
     * already open
     * @return a DataOutputStream object
     */
    public DataOutputStream openDataOutputStream() throws IOException {
          return (new DataOutputStream(openOutputStream()));
    }

    /**
     * Closes the SSL connection. The underlying TCP socket, over which
     * SSL is layered, is also closed unless the latter was opened by
     * an external application and its input/output streams were passed 
     * as argument to the SSLStreamConnection constructor.
     *
     * @exception IOException if the SSL connection could not be
     *                        terminated cleanly
     */ 
    synchronized public void close() throws IOException {
        if (copen) {
            copen = false;
            cleanupIfNeeded();
        }
    }

    /**
     * Returns the security information associated with this connection.
     *
     * @return the security information associated with this open connection
     *
     * @exception IOException if the connection is closed
     */
    public SecurityInfo getSecurityInfo() throws IOException {
        if (!copen) {
            throw new IOException("Connection closed");
        }

        return new SSLBouncyCastleSecurityInfo(this);
    }

    /**
     * Returns the server certificate associated with this connection.
     *
     * @return the server certificate associated with this connection
     */
    public X509Certificate getServerCertificate() {
        return serverCert.getJOSHX509Certificate();
    }

    /**
     * Returns the cipher suite in use for the connection.
     * The value returned is one of the CipherSuite definitions
     * in Appendix C of RFC 2246.
     * The cipher suite string should be used to represent the
     * actual parameters used to establish the connection regardless
     * of whether the secure connection uses SSL V3 or TLS 1.0 or WTLS.
     *
     * @return a String containing the cipher suite in use
     */
    String getCipherSuite() {
        return cipherSuite;
    }

    /**
     * Closes the SSL connection. The underlying TCP socket, over which
     * SSL is layered, is also closed unless the latter was opened by
     * an external application and its input/output streams were passed 
     * as argument to the SSLStreamConnection constructor.
     * 
     * @exception IOException if the SSL connection could not be
     *                        terminated cleanly
     */ 
    void cleanupIfNeeded() throws IOException {
        if (copen || inputStreamState == OPEN || outputStreamState == OPEN ||
               tlsproto == null) {
            // we do not need to cleanup
            return;
        }

        tlsproto.close();
        tlsproto = null;
    }
}

/**
 * This class implements methods
 * to access information about a SSL secure network connection.
 */
class SSLBouncyCastleSecurityInfo implements SecurityInfo {

    /** Parent connection. */
    private SSLBouncyCastleStreamConnection parent;

    /**
     * Creates a <code>SecurityInfo</code> object to pass back to
     * an application.
     *
     * @param parentObj parent object
     */
    SSLBouncyCastleSecurityInfo(SSLBouncyCastleStreamConnection parentObj) {
        parent = parentObj;
    }

    /**
     * Gets the <CODE>Certificate</CODE> used to establish the
     * secure connection with the server.
     *
     * @return the <CODE>Certificate</CODE> used to establish the
     * secure connection with the server
     */
    public javax.microedition.pki.Certificate getServerCertificate() {
        return parent.getServerCertificate();
    }

    /**
     * Returns the security status of the connection.
     *
     * @return <CODE>true</CODE> if the connection has been made directly to
     * the server specified in <code>Connector.open</code> and a handshake
     * with that server has established a secure connection.
     * <CODE>false</CODE> is returned otherwise
     */
    public boolean isSecure() {
        return true;
    }

    /**
     * Returns the protocol version.
     * If appropriate, it should contain the major and minor versions
     * for the protocol separated with a "." (Unicode x2E).
     * For example,
     *     for SSL V3 it MUST return "3.0";
     *     for TLS 1.0 it MUST return "3.1".
     *
     * @return a String containing the version of the protocol
     */
    public String getProtocolVersion() {
        return parent.protoVersion;
    }

    /**
     * Returns the secure protocol name.
     *
     * @return a <code>String</code> containing the secure protocol identifier;
     * if TLS (RFC 2246) is used for the connection the return value is "TLS".
     * If SSL V3 (The SSL Protocol Version 3.0) is used for the connection
     * the return value is "SSL").
     * If WTLS (WAP 199) is used for the connection the return value is "WTLS".
     */
    public String getProtocolName() {
        return parent.protoName;
    }

    /**
     * Returns the cipher suite in use for the connection.
     * The value returned is one of the CipherSuite definitions
     * in Appendix C of RFC 2246.
     * The cipher suite string should be used to represent the
     * actual parameters used to establish the connection regardless
     * of whether the secure connection uses SSL V3 or TLS 1.0 or WTLS.
     *
     * @return a String containing the cipher suite in use
     */
    public String getCipherSuite() {
        return parent.getCipherSuite();
    }
}


