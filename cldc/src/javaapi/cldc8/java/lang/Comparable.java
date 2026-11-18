/*
 * This file is part of JOSHVM and is distributed under the terms of the
 * GNU General Public License version 2 (see legal/GNU/gpl-2.0.txt).
 */

package java.lang;

/**
 * Imposes a total ordering on the objects of each class that implements it.
 */
public interface Comparable<T> {

    /**
     * Returns a negative integer, zero, or a positive integer as this
     * object is less than, equal to, or greater than <code>o</code>.
     */
    int compareTo(T o);
}
