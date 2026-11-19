/*
 * This file is part of JOSHVM and is distributed under the terms of the
 * GNU General Public License version 2 (see legal/GNU/gpl-2.0.txt).
 */

package java.lang;

/**
 * A resource that must be closed when it is no longer needed. The
 * <code>try</code>-with-resources statement closes it automatically.
 */
public interface AutoCloseable {

    void close() throws Exception;
}
