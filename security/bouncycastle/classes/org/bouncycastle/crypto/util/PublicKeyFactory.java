package org.bouncycastle.crypto.util;

import java.io.IOException;
import java.io.InputStream;

import org.bouncycastle.asn1.ASN1Encodable;
import org.bouncycastle.asn1.ASN1InputStream;
import org.bouncycastle.asn1.ASN1Integer;
import org.bouncycastle.asn1.ASN1ObjectIdentifier;
import org.bouncycastle.asn1.ASN1OctetString;
import org.bouncycastle.asn1.ASN1Primitive;
import org.bouncycastle.asn1.DERBitString;
import org.bouncycastle.asn1.DEROctetString;
import org.bouncycastle.asn1.pkcs.PKCSObjectIdentifiers;
import org.bouncycastle.asn1.pkcs.RSAPublicKey;
import org.bouncycastle.asn1.x509.AlgorithmIdentifier;
import org.bouncycastle.asn1.x509.SubjectPublicKeyInfo;
import org.bouncycastle.asn1.x509.X509ObjectIdentifiers;
import org.bouncycastle.crypto.params.AsymmetricKeyParameter;
import org.bouncycastle.crypto.params.RSAKeyParameters;

import com.joshvm.java.math.BigInteger;
import com.joshvm.java.util.HashMap;
import com.joshvm.java.util.Map;

/**
 * Factory to create asymmetric public key parameters for asymmetric ciphers from range of
 * ASN.1 encoded SubjectPublicKeyInfo objects.
 */
public class PublicKeyFactory
{
    private static Map converters = new HashMap();

    static
    {
        converters.put(PKCSObjectIdentifiers.rsaEncryption, new RSAConverter());
        converters.put(X509ObjectIdentifiers.id_ea_rsa, new RSAConverter());
    }

    /**
     * Create a public key from a SubjectPublicKeyInfo encoding
     *
     * @param keyInfoData the SubjectPublicKeyInfo encoding
     * @return the appropriate key parameter
     * @throws IOException on an error decoding the key
     */
    public static AsymmetricKeyParameter createKey(byte[] keyInfoData)
        throws IOException
    {
        return createKey(SubjectPublicKeyInfo.getInstance(ASN1Primitive.fromByteArray(keyInfoData)));
    }

    /**
     * Create a public key from a SubjectPublicKeyInfo encoding read from a stream
     *
     * @param inStr the stream to read the SubjectPublicKeyInfo encoding from
     * @return the appropriate key parameter
     * @throws IOException on an error decoding the key
     */
    public static AsymmetricKeyParameter createKey(InputStream inStr)
        throws IOException
    {
        return createKey(SubjectPublicKeyInfo.getInstance(new ASN1InputStream(inStr).readObject()));
    }

    /**
     * Create a public key from the passed in SubjectPublicKeyInfo
     *
     * @param keyInfo the SubjectPublicKeyInfo containing the key data
     * @return the appropriate key parameter
     * @throws IOException on an error decoding the key
     */
    public static AsymmetricKeyParameter createKey(SubjectPublicKeyInfo keyInfo)
        throws IOException
    {
        return createKey(keyInfo, null);
    }

    /**
     * Create a public key from the passed in SubjectPublicKeyInfo
     *
     * @param keyInfo the SubjectPublicKeyInfo containing the key data
     * @param defaultParams default parameters that might be needed.
     * @return the appropriate key parameter
     * @throws IOException on an error decoding the key
     */
    public static AsymmetricKeyParameter createKey(SubjectPublicKeyInfo keyInfo, Object defaultParams)
        throws IOException
    {
        AlgorithmIdentifier algId = keyInfo.getAlgorithm();
        SubjectPublicKeyInfoConverter converter = (SubjectPublicKeyInfoConverter)converters.get(algId.getAlgorithm());

        if (converter != null)
        {
            return converter.getPublicKeyParameters(keyInfo, defaultParams);
        }
        else
        {
            throw new IOException("algorithm identifier in key not recognised: " + algId.getAlgorithm());
        }
    }

    private static abstract class SubjectPublicKeyInfoConverter
    {
        abstract AsymmetricKeyParameter getPublicKeyParameters(SubjectPublicKeyInfo keyInfo, Object defaultParams)
            throws IOException;
    }

    private static class RSAConverter
        extends SubjectPublicKeyInfoConverter
    {
        AsymmetricKeyParameter getPublicKeyParameters(SubjectPublicKeyInfo keyInfo, Object defaultParams)
            throws IOException
        {
            RSAPublicKey pubKey = RSAPublicKey.getInstance(keyInfo.parsePublicKey());

            return new RSAKeyParameters(false, pubKey.getModulus(), pubKey.getPublicExponent());
        }
    }

}
