package org.bouncycastle.util;

import com.joshvm.java.util.Collection;

public interface Store
{
    Collection getMatches(Selector selector)
        throws StoreException;
}
