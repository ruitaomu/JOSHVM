package org.bouncycastle.math.field;

import com.joshvm.java.math.BigInteger;

public interface FiniteField
{
    BigInteger getCharacteristic();

    int getDimension();
}
