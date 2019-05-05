package org.bouncycastle.crypto;

import com.joshvm.java.security.SecureRandom;

/**
 * Basic registrar class for providing defaults for cryptography services in this module.
 */
public final class CryptoServicesRegistrar
{
    private static volatile SecureRandom defaultSecureRandom;


    private CryptoServicesRegistrar()
    {

    }

    /**
     * Return the default source of randomness.
     *
     * @return the default SecureRandom
     * @throws IllegalStateException if no source of randomness has been provided.
     */
    public static SecureRandom getSecureRandom()
    {
        if (defaultSecureRandom == null)
        {
            return new SecureRandom();
        }
        
        return defaultSecureRandom;
    }

    /**
     * Set a default secure random to be used where none is otherwise provided.
     *
     * @param secureRandom the SecureRandom to use as the default.
     */
    public static void setSecureRandom(SecureRandom secureRandom)
    {
        defaultSecureRandom = secureRandom;
    }

}
