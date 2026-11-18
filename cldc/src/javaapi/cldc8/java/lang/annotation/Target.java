/*
 * This file is part of JOSHVM and is distributed under the terms of the
 * GNU General Public License version 2 (see legal/GNU/gpl-2.0.txt).
 */

package java.lang.annotation;

/**
 * Indicates the kinds of program element the annotated type applies to.
 */
@Documented
@Retention(RetentionPolicy.RUNTIME)
@Target(ElementType.ANNOTATION_TYPE)
public @interface Target {
    ElementType[] value();
}
