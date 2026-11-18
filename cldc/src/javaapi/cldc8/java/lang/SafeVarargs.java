/*
 * This file is part of JOSHVM and is distributed under the terms of the
 * GNU General Public License version 2 (see legal/GNU/gpl-2.0.txt).
 */

package java.lang;

import java.lang.annotation.Documented;
import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

/**
 * Asserts that the annotated method or constructor does nothing unsafe
 * with its variable-arity parameter.
 */
@Documented
@Retention(RetentionPolicy.RUNTIME)
@Target({ElementType.CONSTRUCTOR, ElementType.METHOD})
public @interface SafeVarargs {
}
