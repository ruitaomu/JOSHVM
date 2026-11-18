/*
 * This file is part of JOSHVM and is distributed under the terms of the
 * GNU General Public License version 2 (see legal/GNU/gpl-2.0.txt).
 */

package java.lang;

/**
 * Indicates an access to a field that an object or class no longer has.
 * <p>
 * The compiler refers to this class in the code it emits for a
 * <code>switch</code> on an enum. The VM itself reports a missing field
 * with a plain <code>java.lang.Error</code>, as it does without this
 * class.
 */
public class NoSuchFieldError extends IncompatibleClassChangeError {

    public NoSuchFieldError() {
    }

    public NoSuchFieldError(String s) {
        super(s);
    }
}
