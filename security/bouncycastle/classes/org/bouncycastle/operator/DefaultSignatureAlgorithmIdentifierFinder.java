package org.bouncycastle.operator;

import org.bouncycastle.asn1.ASN1Encodable;
import org.bouncycastle.asn1.ASN1Integer;
import org.bouncycastle.asn1.ASN1ObjectIdentifier;
import org.bouncycastle.asn1.DERNull;
import org.bouncycastle.asn1.nist.NISTObjectIdentifiers;
import org.bouncycastle.asn1.oiw.OIWObjectIdentifiers;
import org.bouncycastle.asn1.pkcs.PKCSObjectIdentifiers;
import org.bouncycastle.asn1.pkcs.RSASSAPSSparams;
import org.bouncycastle.asn1.x509.AlgorithmIdentifier;
import org.bouncycastle.util.Strings;

import com.joshvm.java.util.HashMap;
import com.joshvm.java.util.HashSet;
import com.joshvm.java.util.Map;
import com.joshvm.java.util.Set;

public class DefaultSignatureAlgorithmIdentifierFinder
    implements SignatureAlgorithmIdentifierFinder
{
    private static Map algorithms = new HashMap();
    private static Set noParams = new HashSet();
    private static Map params = new HashMap();
    private static Set pkcs15RsaEncryption = new HashSet();
    private static Map digestOids = new HashMap();

    private static final ASN1ObjectIdentifier ENCRYPTION_RSA = PKCSObjectIdentifiers.rsaEncryption;
    private static final ASN1ObjectIdentifier ENCRYPTION_RSA_PSS = PKCSObjectIdentifiers.id_RSASSA_PSS;
    
    static
    {
        algorithms.put("MD2WITHRSAENCRYPTION", PKCSObjectIdentifiers.md2WithRSAEncryption);
        algorithms.put("MD2WITHRSA", PKCSObjectIdentifiers.md2WithRSAEncryption);
        algorithms.put("MD5WITHRSAENCRYPTION", PKCSObjectIdentifiers.md5WithRSAEncryption);
        algorithms.put("MD5WITHRSA", PKCSObjectIdentifiers.md5WithRSAEncryption);
        algorithms.put("SHA1WITHRSAENCRYPTION", PKCSObjectIdentifiers.sha1WithRSAEncryption);
        algorithms.put("SHA1WITHRSA", PKCSObjectIdentifiers.sha1WithRSAEncryption);
        algorithms.put("SHA224WITHRSAENCRYPTION", PKCSObjectIdentifiers.sha224WithRSAEncryption);
        algorithms.put("SHA224WITHRSA", PKCSObjectIdentifiers.sha224WithRSAEncryption);
        algorithms.put("SHA256WITHRSAENCRYPTION", PKCSObjectIdentifiers.sha256WithRSAEncryption);
        algorithms.put("SHA256WITHRSA", PKCSObjectIdentifiers.sha256WithRSAEncryption);
        algorithms.put("SHA384WITHRSAENCRYPTION", PKCSObjectIdentifiers.sha384WithRSAEncryption);
        algorithms.put("SHA384WITHRSA", PKCSObjectIdentifiers.sha384WithRSAEncryption);
        algorithms.put("SHA512WITHRSAENCRYPTION", PKCSObjectIdentifiers.sha512WithRSAEncryption);
        algorithms.put("SHA512WITHRSA", PKCSObjectIdentifiers.sha512WithRSAEncryption);
        algorithms.put("SHA1WITHRSAANDMGF1", PKCSObjectIdentifiers.id_RSASSA_PSS);
        algorithms.put("SHA224WITHRSAANDMGF1", PKCSObjectIdentifiers.id_RSASSA_PSS);
        algorithms.put("SHA256WITHRSAANDMGF1", PKCSObjectIdentifiers.id_RSASSA_PSS);
        algorithms.put("SHA384WITHRSAANDMGF1", PKCSObjectIdentifiers.id_RSASSA_PSS);
        algorithms.put("SHA512WITHRSAANDMGF1", PKCSObjectIdentifiers.id_RSASSA_PSS);
        algorithms.put("SHA3-224WITHRSAANDMGF1", PKCSObjectIdentifiers.id_RSASSA_PSS);
        algorithms.put("SHA3-256WITHRSAANDMGF1", PKCSObjectIdentifiers.id_RSASSA_PSS);
        algorithms.put("SHA3-384WITHRSAANDMGF1", PKCSObjectIdentifiers.id_RSASSA_PSS);
        algorithms.put("SHA3-512WITHRSAANDMGF1", PKCSObjectIdentifiers.id_RSASSA_PSS);
        algorithms.put("SHA224WITHDSA", NISTObjectIdentifiers.dsa_with_sha224);
        algorithms.put("SHA256WITHDSA", NISTObjectIdentifiers.dsa_with_sha256);
        algorithms.put("SHA384WITHDSA", NISTObjectIdentifiers.dsa_with_sha384);
        algorithms.put("SHA512WITHDSA", NISTObjectIdentifiers.dsa_with_sha512);
        algorithms.put("SHA3-224WITHDSA", NISTObjectIdentifiers.id_dsa_with_sha3_224);
        algorithms.put("SHA3-256WITHDSA", NISTObjectIdentifiers.id_dsa_with_sha3_256);
        algorithms.put("SHA3-384WITHDSA", NISTObjectIdentifiers.id_dsa_with_sha3_384);
        algorithms.put("SHA3-512WITHDSA", NISTObjectIdentifiers.id_dsa_with_sha3_512);
        algorithms.put("SHA3-224WITHECDSA", NISTObjectIdentifiers.id_ecdsa_with_sha3_224);
        algorithms.put("SHA3-256WITHECDSA", NISTObjectIdentifiers.id_ecdsa_with_sha3_256);
        algorithms.put("SHA3-384WITHECDSA", NISTObjectIdentifiers.id_ecdsa_with_sha3_384);
        algorithms.put("SHA3-512WITHECDSA", NISTObjectIdentifiers.id_ecdsa_with_sha3_512);
        algorithms.put("SHA3-224WITHRSA", NISTObjectIdentifiers.id_rsassa_pkcs1_v1_5_with_sha3_224);
        algorithms.put("SHA3-256WITHRSA", NISTObjectIdentifiers.id_rsassa_pkcs1_v1_5_with_sha3_256);
        algorithms.put("SHA3-384WITHRSA", NISTObjectIdentifiers.id_rsassa_pkcs1_v1_5_with_sha3_384);
        algorithms.put("SHA3-512WITHRSA", NISTObjectIdentifiers.id_rsassa_pkcs1_v1_5_with_sha3_512);
        algorithms.put("SHA3-224WITHRSAENCRYPTION", NISTObjectIdentifiers.id_rsassa_pkcs1_v1_5_with_sha3_224);
        algorithms.put("SHA3-256WITHRSAENCRYPTION", NISTObjectIdentifiers.id_rsassa_pkcs1_v1_5_with_sha3_256);
        algorithms.put("SHA3-384WITHRSAENCRYPTION", NISTObjectIdentifiers.id_rsassa_pkcs1_v1_5_with_sha3_384);
        algorithms.put("SHA3-512WITHRSAENCRYPTION", NISTObjectIdentifiers.id_rsassa_pkcs1_v1_5_with_sha3_512);
        
        //
        // According to RFC 3279, the ASN.1 encoding SHALL (id-dsa-with-sha1) or MUST (ecdsa-with-SHA*) omit the parameters field.
        // The parameters field SHALL be NULL for RSA based signature algorithms.
        //
        noParams.add(NISTObjectIdentifiers.dsa_with_sha224);
        noParams.add(NISTObjectIdentifiers.dsa_with_sha256);
        noParams.add(NISTObjectIdentifiers.dsa_with_sha384);
        noParams.add(NISTObjectIdentifiers.dsa_with_sha512);
        noParams.add(NISTObjectIdentifiers.id_dsa_with_sha3_224);
        noParams.add(NISTObjectIdentifiers.id_dsa_with_sha3_256);
        noParams.add(NISTObjectIdentifiers.id_dsa_with_sha3_384);
        noParams.add(NISTObjectIdentifiers.id_dsa_with_sha3_512);
        noParams.add(NISTObjectIdentifiers.id_ecdsa_with_sha3_224);
        noParams.add(NISTObjectIdentifiers.id_ecdsa_with_sha3_256);
        noParams.add(NISTObjectIdentifiers.id_ecdsa_with_sha3_384);
        noParams.add(NISTObjectIdentifiers.id_ecdsa_with_sha3_512);

        
        //
        // PKCS 1.5 encrypted  algorithms
        //
        pkcs15RsaEncryption.add(PKCSObjectIdentifiers.sha1WithRSAEncryption);
        pkcs15RsaEncryption.add(PKCSObjectIdentifiers.sha224WithRSAEncryption);
        pkcs15RsaEncryption.add(PKCSObjectIdentifiers.sha256WithRSAEncryption);
        pkcs15RsaEncryption.add(PKCSObjectIdentifiers.sha384WithRSAEncryption);
        pkcs15RsaEncryption.add(PKCSObjectIdentifiers.sha512WithRSAEncryption);
        pkcs15RsaEncryption.add(NISTObjectIdentifiers.id_rsassa_pkcs1_v1_5_with_sha3_224);
        pkcs15RsaEncryption.add(NISTObjectIdentifiers.id_rsassa_pkcs1_v1_5_with_sha3_256);
        pkcs15RsaEncryption.add(NISTObjectIdentifiers.id_rsassa_pkcs1_v1_5_with_sha3_384);
        pkcs15RsaEncryption.add(NISTObjectIdentifiers.id_rsassa_pkcs1_v1_5_with_sha3_512);

        //
        // explicit params
        //
        AlgorithmIdentifier sha1AlgId = new AlgorithmIdentifier(OIWObjectIdentifiers.idSHA1, DERNull.INSTANCE);
        params.put("SHA1WITHRSAANDMGF1", createPSSParams(sha1AlgId, 20));

        AlgorithmIdentifier sha224AlgId = new AlgorithmIdentifier(NISTObjectIdentifiers.id_sha224, DERNull.INSTANCE);
        params.put("SHA224WITHRSAANDMGF1", createPSSParams(sha224AlgId, 28));

        AlgorithmIdentifier sha256AlgId = new AlgorithmIdentifier(NISTObjectIdentifiers.id_sha256, DERNull.INSTANCE);
        params.put("SHA256WITHRSAANDMGF1", createPSSParams(sha256AlgId, 32));

        AlgorithmIdentifier sha384AlgId = new AlgorithmIdentifier(NISTObjectIdentifiers.id_sha384, DERNull.INSTANCE);
        params.put("SHA384WITHRSAANDMGF1", createPSSParams(sha384AlgId, 48));

        AlgorithmIdentifier sha512AlgId = new AlgorithmIdentifier(NISTObjectIdentifiers.id_sha512, DERNull.INSTANCE);
        params.put("SHA512WITHRSAANDMGF1", createPSSParams(sha512AlgId, 64));

        AlgorithmIdentifier sha3_224AlgId = new AlgorithmIdentifier(NISTObjectIdentifiers.id_sha3_224, DERNull.INSTANCE);
        params.put("SHA3-224WITHRSAANDMGF1", createPSSParams(sha3_224AlgId, 28));

        AlgorithmIdentifier sha3_256AlgId = new AlgorithmIdentifier(NISTObjectIdentifiers.id_sha3_256, DERNull.INSTANCE);
        params.put("SHA3-256WITHRSAANDMGF1", createPSSParams(sha3_256AlgId, 32));

        AlgorithmIdentifier sha3_384AlgId = new AlgorithmIdentifier(NISTObjectIdentifiers.id_sha3_384, DERNull.INSTANCE);
        params.put("SHA3-384WITHRSAANDMGF1", createPSSParams(sha3_384AlgId, 48));

        AlgorithmIdentifier sha3_512AlgId = new AlgorithmIdentifier(NISTObjectIdentifiers.id_sha3_512, DERNull.INSTANCE);
        params.put("SHA3-512WITHRSAANDMGF1", createPSSParams(sha3_512AlgId, 64));

        //
        // digests
        //
        digestOids.put(PKCSObjectIdentifiers.sha224WithRSAEncryption, NISTObjectIdentifiers.id_sha224);
        digestOids.put(PKCSObjectIdentifiers.sha256WithRSAEncryption, NISTObjectIdentifiers.id_sha256);
        digestOids.put(PKCSObjectIdentifiers.sha384WithRSAEncryption, NISTObjectIdentifiers.id_sha384);
        digestOids.put(PKCSObjectIdentifiers.sha512WithRSAEncryption, NISTObjectIdentifiers.id_sha512);
        digestOids.put(NISTObjectIdentifiers.dsa_with_sha224, NISTObjectIdentifiers.id_sha224);
        digestOids.put(NISTObjectIdentifiers.dsa_with_sha224, NISTObjectIdentifiers.id_sha256);
        digestOids.put(NISTObjectIdentifiers.dsa_with_sha224, NISTObjectIdentifiers.id_sha384);
        digestOids.put(NISTObjectIdentifiers.dsa_with_sha224, NISTObjectIdentifiers.id_sha512);
        digestOids.put(NISTObjectIdentifiers.id_dsa_with_sha3_224, NISTObjectIdentifiers.id_sha3_224);
        digestOids.put(NISTObjectIdentifiers.id_dsa_with_sha3_256, NISTObjectIdentifiers.id_sha3_256);
        digestOids.put(NISTObjectIdentifiers.id_dsa_with_sha3_384, NISTObjectIdentifiers.id_sha3_384);
        digestOids.put(NISTObjectIdentifiers.id_dsa_with_sha3_512, NISTObjectIdentifiers.id_sha3_512);
        digestOids.put(NISTObjectIdentifiers.id_ecdsa_with_sha3_224, NISTObjectIdentifiers.id_sha3_224);
        digestOids.put(NISTObjectIdentifiers.id_ecdsa_with_sha3_256, NISTObjectIdentifiers.id_sha3_256);
        digestOids.put(NISTObjectIdentifiers.id_ecdsa_with_sha3_384, NISTObjectIdentifiers.id_sha3_384);
        digestOids.put(NISTObjectIdentifiers.id_ecdsa_with_sha3_512, NISTObjectIdentifiers.id_sha3_512);
        digestOids.put(NISTObjectIdentifiers.id_rsassa_pkcs1_v1_5_with_sha3_224, NISTObjectIdentifiers.id_sha3_224);
        digestOids.put(NISTObjectIdentifiers.id_rsassa_pkcs1_v1_5_with_sha3_256, NISTObjectIdentifiers.id_sha3_256);
        digestOids.put(NISTObjectIdentifiers.id_rsassa_pkcs1_v1_5_with_sha3_384, NISTObjectIdentifiers.id_sha3_384);
        digestOids.put(NISTObjectIdentifiers.id_rsassa_pkcs1_v1_5_with_sha3_512, NISTObjectIdentifiers.id_sha3_512);

        digestOids.put(PKCSObjectIdentifiers.md2WithRSAEncryption, PKCSObjectIdentifiers.md2);
        digestOids.put(PKCSObjectIdentifiers.md4WithRSAEncryption, PKCSObjectIdentifiers.md4);
        digestOids.put(PKCSObjectIdentifiers.md5WithRSAEncryption, PKCSObjectIdentifiers.md5);
        digestOids.put(PKCSObjectIdentifiers.sha1WithRSAEncryption, OIWObjectIdentifiers.idSHA1);
    }

    private static AlgorithmIdentifier generate(String signatureAlgorithm)
    {
        AlgorithmIdentifier sigAlgId;
        AlgorithmIdentifier encAlgId;
        AlgorithmIdentifier digAlgId;

        String algorithmName = Strings.toUpperCase(signatureAlgorithm);
        ASN1ObjectIdentifier sigOID = (ASN1ObjectIdentifier)algorithms.get(algorithmName);
        if (sigOID == null)
        {
            throw new IllegalArgumentException("Unknown signature type requested: " + algorithmName);
        }

        if (noParams.contains(sigOID))
        {
            sigAlgId = new AlgorithmIdentifier(sigOID);
        }
        else if (params.containsKey(algorithmName))
        {
            sigAlgId = new AlgorithmIdentifier(sigOID, (ASN1Encodable)params.get(algorithmName));
        }
        else
        {
            sigAlgId = new AlgorithmIdentifier(sigOID, DERNull.INSTANCE);
        }

        if (pkcs15RsaEncryption.contains(sigOID))
        {
            encAlgId = new AlgorithmIdentifier(PKCSObjectIdentifiers.rsaEncryption, DERNull.INSTANCE);
        }
        else
        {
            encAlgId = sigAlgId;
        }

        if (sigAlgId.getAlgorithm().equals(PKCSObjectIdentifiers.id_RSASSA_PSS))
        {
            digAlgId = ((RSASSAPSSparams)sigAlgId.getParameters()).getHashAlgorithm();
        }
        else
        {
            digAlgId = new AlgorithmIdentifier((ASN1ObjectIdentifier)digestOids.get(sigOID), DERNull.INSTANCE);
        }

        return sigAlgId;
    }

    private static RSASSAPSSparams createPSSParams(AlgorithmIdentifier hashAlgId, int saltSize)
    {
        return new RSASSAPSSparams(
            hashAlgId,
            new AlgorithmIdentifier(PKCSObjectIdentifiers.id_mgf1, hashAlgId),
            new ASN1Integer(saltSize),
            new ASN1Integer(1));
    }

    public AlgorithmIdentifier find(String sigAlgName)
    {
        return generate(sigAlgName);
    }
}
