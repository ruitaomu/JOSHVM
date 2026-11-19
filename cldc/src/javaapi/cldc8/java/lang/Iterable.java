/*
 * This file is part of JOSHVM and is distributed under the terms of the
 * GNU General Public License version 2 (see legal/GNU/gpl-2.0.txt).
 */

package java.lang;

import java.util.Iterator;

/**
 * Implementing this interface allows an object to be the target of the
 * enhanced <code>for</code> statement.
 */
public interface Iterable<T> {

    Iterator<T> iterator();
}
