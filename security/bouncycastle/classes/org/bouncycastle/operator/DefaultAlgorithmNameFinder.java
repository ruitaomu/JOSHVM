package org.bouncycastle.operator;

import org.bouncycastle.asn1.ASN1ObjectIdentifier;
import org.bouncycastle.asn1.nist.NISTObjectIdentifiers;
import org.bouncycastle.asn1.oiw.OIWObjectIdentifiers;
import org.bouncycastle.asn1.pkcs.PKCSObjectIdentifiers;
import org.bouncycastle.asn1.x509.AlgorithmIdentifier;

import com.joshvm.java.util.HashMap;
import com.joshvm.java.util.Map;

public class DefaultAlgorithmNameFinder
    implements AlgorithmNameFinder
{
    private final static Map algorithms = new HashMap();

    static
    {
        algorithms.put(NISTObjectIdentifiers.id_sha224, "SHA224");
        algorithms.put(NISTObjectIdentifiers.id_sha256, "SHA256");
        algorithms.put(NISTObjectIdentifiers.id_sha384, "SHA384");
        algorithms.put(NISTObjectIdentifiers.id_sha512, "SHA512");
        algorithms.put(NISTObjectIdentifiers.id_sha3_224, "SHA3-224");
        algorithms.put(NISTObjectIdentifiers.id_sha3_256, "SHA3-256");
        algorithms.put(NISTObjectIdentifiers.id_sha3_384, "SHA3-384");
        algorithms.put(NISTObjectIdentifiers.id_sha3_512, "SHA3-512");
        algorithms.put(OIWObjectIdentifiers.elGamalAlgorithm, "ELGAMAL");
        algorithms.put(OIWObjectIdentifiers.idSHA1, "SHA1");
        algorithms.put(OIWObjectIdentifiers.md5WithRSA, "MD5WITHRSA");
        algorithms.put(OIWObjectIdentifiers.sha1WithRSA, "SHA1WITHRSA");
        algorithms.put(PKCSObjectIdentifiers.id_RSAES_OAEP, "RSAOAEP");
        algorithms.put(PKCSObjectIdentifiers.id_RSASSA_PSS, "RSAPSS");
        algorithms.put(PKCSObjectIdentifiers.md2WithRSAEncryption, "MD2WITHRSA");
        algorithms.put(PKCSObjectIdentifiers.md5, "MD5");
        algorithms.put(PKCSObjectIdentifiers.md5WithRSAEncryption, "MD5WITHRSA");
        algorithms.put(PKCSObjectIdentifiers.rsaEncryption, "RSA");
        algorithms.put(PKCSObjectIdentifiers.sha1WithRSAEncryption, "SHA1WITHRSA");
        algorithms.put(PKCSObjectIdentifiers.sha224WithRSAEncryption, "SHA224WITHRSA");
        algorithms.put(PKCSObjectIdentifiers.sha256WithRSAEncryption, "SHA256WITHRSA");
        algorithms.put(PKCSObjectIdentifiers.sha384WithRSAEncryption, "SHA384WITHRSA");
        algorithms.put(PKCSObjectIdentifiers.sha512WithRSAEncryption, "SHA512WITHRSA");
        algorithms.put(NISTObjectIdentifiers.id_rsassa_pkcs1_v1_5_with_sha3_224, "SHA3-224WITHRSA");
        algorithms.put(NISTObjectIdentifiers.id_rsassa_pkcs1_v1_5_with_sha3_256, "SHA3-256WITHRSA");
        algorithms.put(NISTObjectIdentifiers.id_rsassa_pkcs1_v1_5_with_sha3_384, "SHA3-384WITHRSA");
        algorithms.put(NISTObjectIdentifiers.id_rsassa_pkcs1_v1_5_with_sha3_512, "SHA3-512WITHRSA");
        algorithms.put(NISTObjectIdentifiers.id_ecdsa_with_sha3_224, "SHA3-224WITHECDSA");
        algorithms.put(NISTObjectIdentifiers.id_ecdsa_with_sha3_256, "SHA3-256WITHECDSA");
        algorithms.put(NISTObjectIdentifiers.id_ecdsa_with_sha3_384, "SHA3-384WITHECDSA");
        algorithms.put(NISTObjectIdentifiers.id_ecdsa_with_sha3_512, "SHA3-512WITHECDSA");
        algorithms.put(NISTObjectIdentifiers.dsa_with_sha224, "SHA224WITHDSA");
        algorithms.put(NISTObjectIdentifiers.dsa_with_sha256, "SHA256WITHDSA");
        algorithms.put(NISTObjectIdentifiers.dsa_with_sha384, "SHA384WITHDSA");
        algorithms.put(NISTObjectIdentifiers.dsa_with_sha512, "SHA512WITHDSA");
        algorithms.put(NISTObjectIdentifiers.id_dsa_with_sha3_224, "SHA3-224WITHDSA");
        algorithms.put(NISTObjectIdentifiers.id_dsa_with_sha3_256, "SHA3-256WITHDSA");
        algorithms.put(NISTObjectIdentifiers.id_dsa_with_sha3_384, "SHA3-384WITHDSA");
        algorithms.put(NISTObjectIdentifiers.id_dsa_with_sha3_512, "SHA3-512WITHDSA");

        algorithms.put(PKCSObjectIdentifiers.RC2_CBC, "RC2/CBC");
        algorithms.put(PKCSObjectIdentifiers.des_EDE3_CBC, "DESEDE-3KEY/CBC");
        algorithms.put(NISTObjectIdentifiers.id_aes128_ECB, "AES-128/ECB");
        algorithms.put(NISTObjectIdentifiers.id_aes192_ECB, "AES-192/ECB");
        algorithms.put(NISTObjectIdentifiers.id_aes256_ECB, "AES-256/ECB");
        algorithms.put(NISTObjectIdentifiers.id_aes128_CBC, "AES-128/CBC");
        algorithms.put(NISTObjectIdentifiers.id_aes192_CBC, "AES-192/CBC");
        algorithms.put(NISTObjectIdentifiers.id_aes256_CBC, "AES-256/CBC");
        algorithms.put(NISTObjectIdentifiers.id_aes128_CFB, "AES-128/CFB");
        algorithms.put(NISTObjectIdentifiers.id_aes192_CFB, "AES-192/CFB");
        algorithms.put(NISTObjectIdentifiers.id_aes256_CFB, "AES-256/CFB");
        algorithms.put(NISTObjectIdentifiers.id_aes128_OFB, "AES-128/OFB");
        algorithms.put(NISTObjectIdentifiers.id_aes192_OFB, "AES-192/OFB");
        algorithms.put(NISTObjectIdentifiers.id_aes256_OFB, "AES-256/OFB");
    }

    public boolean hasAlgorithmName(ASN1ObjectIdentifier objectIdentifier)
    {
        return algorithms.containsKey(objectIdentifier);
    }

    public String getAlgorithmName(ASN1ObjectIdentifier objectIdentifier)
    {
        String name = (String)algorithms.get(objectIdentifier);

        return (name != null) ? name : objectIdentifier.getId();
    }

    public String getAlgorithmName(AlgorithmIdentifier algorithmIdentifier)
    {
        // TODO: take into account PSS/OAEP params
        return getAlgorithmName(algorithmIdentifier.getAlgorithm());
    }
}
