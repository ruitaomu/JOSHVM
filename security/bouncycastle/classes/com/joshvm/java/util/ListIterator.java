package com.joshvm.java.util;

import java.util.NoSuchElementException;

public interface ListIterator
    extends Iterator
{
    public boolean hasPrevious();

    public Object previous()
        throws NoSuchElementException;

    public int nextIndex();

    public int previousIndex();

    public void set(Object o)
        throws RuntimeException, ClassCastException, IllegalArgumentException, IllegalStateException;

    public void add(Object o)
        throws RuntimeException, ClassCastException, IllegalArgumentException;
}
