
package com.joshvm.java.util;

import java.util.NoSuchElementException;

public interface Iterator
{
    public abstract boolean hasNext();
    public abstract Object next() throws NoSuchElementException;
    public abstract void remove() throws RuntimeException,IllegalStateException;
}
