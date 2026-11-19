/*
 * This file is part of JOSHVM and is distributed under the terms of the
 * GNU General Public License version 2 (see legal/GNU/gpl-2.0.txt).
 */

package java.lang;

import java.util.Hashtable;

/**
 * The common base class of all enumeration types.
 */
public abstract class Enum<E extends Enum<E>> implements Comparable<E> {

    /*
     * The constants of every enum type created so far, keyed by the enum
     * class. valueOf(Class, String) needs them, and without reflection
     * there is no way to call the static values() method of an arbitrary
     * enum class, so each constant records itself here when it is
     * constructed. Created on first use: programs without enums pay
     * nothing.
     */
    private static Hashtable constants;

    private final String name;
    private final int ordinal;

    /**
     * Called only by the code the compiler emits for an enum declaration.
     */
    protected Enum(String name, int ordinal) {
        this.name = name;
        this.ordinal = ordinal;
        register(this);
    }

    private static synchronized void register(Enum constant) {
        if (constants == null) {
            constants = new Hashtable();
        }
        Class type = constant.getDeclaringClass();
        Enum[] old = (Enum[]) constants.get(type);
        // Constants are constructed in ordinal order.
        Enum[] list = new Enum[constant.ordinal + 1];
        if (old != null) {
            System.arraycopy(old, 0, list, 0,
                             Math.min(old.length, list.length));
        }
        list[constant.ordinal] = constant;
        constants.put(type, list);
    }

    public final String name() {
        return name;
    }

    public final int ordinal() {
        return ordinal;
    }

    public String toString() {
        return name;
    }

    public final boolean equals(Object other) {
        return this == other;
    }

    public final int hashCode() {
        return super.hashCode();
    }

    public final int compareTo(E o) {
        Enum other = (Enum) o;
        if (getDeclaringClass() != other.getDeclaringClass()) {
            throw new ClassCastException();
        }
        return ordinal - other.ordinal;
    }

    /**
     * Returns the class of this constant's enum type. It differs from
     * <code>getClass()</code> for a constant declared with a class body.
     */
    public final Class<E> getDeclaringClass() {
        Class type = getClass();
        Class parent = type.getSuperclass0();
        return (parent == Enum.class) ? type : parent;
    }

    /**
     * Returns the constant of <code>enumType</code> with the given name.
     *
     * @exception IllegalArgumentException if <code>enumType</code> has no
     *            constant with that name or is not an enum type
     * @exception NullPointerException if an argument is <code>null</code>
     */
    public static <T extends Enum<T>> T valueOf(Class<T> enumType,
                                                String name) {
        if (name == null) {
            throw new NullPointerException();
        }
        try {
            // Runs the static initializer, which creates the constants.
            Class.forName(enumType.getName());
        } catch (ClassNotFoundException e) {
            throw new IllegalArgumentException(enumType.getName());
        }
        Enum[] list = null;
        synchronized (Enum.class) {
            if (constants != null) {
                list = (Enum[]) constants.get(enumType);
            }
        }
        if (list != null) {
            for (int i = 0; i < list.length; i++) {
                if (list[i] != null && list[i].name.equals(name)) {
                    return (T) list[i];
                }
            }
        }
        throw new IllegalArgumentException(name);
    }
}
