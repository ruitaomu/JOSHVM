/*
 * This file is part of JOSHVM and is distributed under the terms of the
 * GNU General Public License version 2 (see legal/GNU/gpl-2.0.txt).
 */

package java.lang;

/**
 * A readable sequence of <code>char</code> values.
 */
public interface CharSequence {

    int length();

    char charAt(int index);

    CharSequence subSequence(int start, int end);

    String toString();
}
