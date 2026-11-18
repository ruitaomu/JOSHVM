/*
 * This file is part of JOSHVM and is distributed under the terms of the
 * GNU General Public License version 2 (see legal/GNU/gpl-2.0.txt).
 */

package java.lang;

/**
 * Indicates an incompatible change to the definition of a class on which
 * the executing code depends.
 */
public class IncompatibleClassChangeError extends LinkageError {

    public IncompatibleClassChangeError() {
    }

    public IncompatibleClassChangeError(String s) {
        super(s);
    }
}
