/*
 * This file is part of JOSHVM and is distributed under the terms of the
 * GNU General Public License version 2 (see legal/GNU/gpl-2.0.txt).
 */

package java.util;

/**
 * An iterator over a collection of elements.
 */
public interface Iterator<E> {

    boolean hasNext();

    /**
     * @exception NoSuchElementException if there are no more elements
     */
    E next();

    /**
     * Removes the element last returned by <code>next()</code>.
     *
     * @exception UnsupportedOperationException if removal is not supported
     * @exception IllegalStateException if <code>next()</code> has not been
     *            called, or <code>remove()</code> was already called after
     *            the last call to it
     */
    void remove();
}
