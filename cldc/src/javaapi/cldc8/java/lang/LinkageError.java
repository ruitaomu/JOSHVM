/*
 * This file is part of JOSHVM and is distributed under the terms of the
 * GNU General Public License version 2 (see legal/GNU/gpl-2.0.txt).
 */

package java.lang;

/**
 * Indicates that a class depends on another class that has changed
 * incompatibly since the first was compiled.
 */
public class LinkageError extends Error {

    public LinkageError() {
    }

    public LinkageError(String s) {
        super(s);
    }
}
