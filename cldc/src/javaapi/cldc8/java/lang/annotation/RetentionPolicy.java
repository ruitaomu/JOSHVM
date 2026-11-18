/*
 * This file is part of JOSHVM and is distributed under the terms of the
 * GNU General Public License version 2 (see legal/GNU/gpl-2.0.txt).
 */

package java.lang.annotation;

/**
 * How long annotations of an annotation type are retained. This VM does
 * not make annotations available at run time whatever the policy.
 */
public enum RetentionPolicy {
    SOURCE,
    CLASS,
    RUNTIME
}
