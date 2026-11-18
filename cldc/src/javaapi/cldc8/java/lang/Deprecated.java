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
 * Marks a program element that programmers are discouraged from using.
 */
@Documented
@Retention(RetentionPolicy.RUNTIME)
@Target({ElementType.CONSTRUCTOR, ElementType.FIELD,
         ElementType.LOCAL_VARIABLE, ElementType.METHOD,
         ElementType.PACKAGE, ElementType.PARAMETER, ElementType.TYPE})
public @interface Deprecated {
}
