package org.bouncycastle.crypto.tls;

import java.io.IOException;

public abstract class DefaultTlsClient
    extends AbstractTlsClient
{
    public DefaultTlsClient()
    {
        super();
    }

    public DefaultTlsClient(TlsCipherFactory cipherFactory)
    {
        super(cipherFactory);
    }

    public int[] getCipherSuites()
    {
        return new int[]
        {
        		/*
            CipherSuite.TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256,
            CipherSuite.TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256,
            CipherSuite.TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA,
            CipherSuite.TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256,
            CipherSuite.TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256,
            CipherSuite.TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA,
            CipherSuite.TLS_DHE_DSS_WITH_AES_128_GCM_SHA256,
            CipherSuite.TLS_DHE_DSS_WITH_AES_128_CBC_SHA256,
            CipherSuite.TLS_DHE_DSS_WITH_AES_128_CBC_SHA,
            CipherSuite.TLS_DHE_RSA_WITH_AES_128_GCM_SHA256,
            CipherSuite.TLS_DHE_RSA_WITH_AES_128_CBC_SHA256,
            CipherSuite.TLS_DHE_RSA_WITH_AES_128_CBC_SHA,
            CipherSuite.TLS_RSA_WITH_AES_128_GCM_SHA256,
			*/
            CipherSuite.TLS_RSA_WITH_AES_128_CBC_SHA256,            
            CipherSuite.TLS_RSA_WITH_AES_128_CBC_SHA,
        };
    }

    public TlsKeyExchange getKeyExchange()
        throws IOException
    {
        int keyExchangeAlgorithm = TlsUtils.getKeyExchangeAlgorithm(selectedCipherSuite);

        switch (keyExchangeAlgorithm)
        {
        case KeyExchangeAlgorithm.RSA:
            return createRSAKeyExchange();

        default:
            /*
             * Note: internal error here; the TlsProtocol implementation verifies that the
             * server-selected cipher suite was in the list of client-offered cipher suites, so if
             * we now can't produce an implementation, we shouldn't have offered it!
             */
            throw new TlsFatalAlert(AlertDescription.internal_error);
        }
    }

    protected TlsKeyExchange createRSAKeyExchange()
    {
        return new TlsRSAKeyExchange(supportedSignatureAlgorithms);
    }
}
