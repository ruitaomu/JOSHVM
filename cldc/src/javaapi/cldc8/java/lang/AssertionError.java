/*
 * This file is part of JOSHVM and is distributed under the terms of the
 * GNU General Public License version 2 (see legal/GNU/gpl-2.0.txt).
 */

package java.lang;

/**
 * Thrown when an <code>assert</code> statement fails. Assertions are
 * always disabled in this VM, so this is only thrown by code that
 * constructs it explicitly.
 */
public class AssertionError extends Error {

    public AssertionError() {
    }

    public AssertionError(Object detailMessage) {
        super(String.valueOf(detailMessage));
        if (detailMessage instanceof Throwable) {
            initCause((Throwable) detailMessage);
        }
    }

    public AssertionError(boolean detailMessage) {
        super(String.valueOf(detailMessage));
    }

    public AssertionError(char detailMessage) {
        super(String.valueOf(detailMessage));
    }

    public AssertionError(int detailMessage) {
        super(String.valueOf(detailMessage));
    }

    public AssertionError(long detailMessage) {
        super(String.valueOf(detailMessage));
    }

    public AssertionError(float detailMessage) {
        super(String.valueOf(detailMessage));
    }

    public AssertionError(double detailMessage) {
        super(String.valueOf(detailMessage));
    }
}
