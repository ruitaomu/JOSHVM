package org.bouncycastle.math.ec;

import com.joshvm.java.math.BigInteger;

public class WNafUtil {
    public static int getNafWeight(BigInteger k)
    {
        if (k.signum() == 0)
        {
            return 0;
        }

        BigInteger _3k = k.shiftLeft(1).add(k);
        BigInteger diff = _3k.xor(k);

        return diff.bitCount();
    }
}
