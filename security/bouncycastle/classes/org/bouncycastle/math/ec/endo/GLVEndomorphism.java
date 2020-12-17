package org.bouncycastle.math.ec.endo;

import com.joshvm.java.math.BigInteger;

public interface GLVEndomorphism extends ECEndomorphism
{
    BigInteger[] decomposeScalar(BigInteger k);
}
