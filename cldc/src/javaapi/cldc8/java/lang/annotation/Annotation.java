/*
 * This file is part of JOSHVM and is distributed under the terms of the
 * GNU General Public License version 2 (see legal/GNU/gpl-2.0.txt).
 */

package java.lang.annotation;

/**
 * The common interface extended by all annotation types. This VM keeps no
 * annotations at run time, so no object ever implements it; it exists for
 * the compiler.
 */
public interface Annotation {

    boolean equals(Object obj);

    int hashCode();

    String toString();

    Class<? extends Annotation> annotationType();
}
