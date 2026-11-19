/*
 * This file is part of JOSHVM and is distributed under the terms of the
 * GNU General Public License version 2 (see legal/GNU/gpl-2.0.txt).
 */

package java.lang;

/**
 * A mutable sequence of characters with the same operations as
 * {@link StringBuffer} but without synchronization. The compiler uses it
 * for string concatenation.
 */
public final class StringBuilder implements CharSequence {

    private char[] value;
    private int count;

    public StringBuilder() {
        this(16);
    }

    public StringBuilder(int capacity) {
        value = new char[capacity];
    }

    public StringBuilder(String str) {
        this(str.length() + 16);
        append(str);
    }

    public StringBuilder(CharSequence seq) {
        this(seq.length() + 16);
        append(seq);
    }

    public int length() {
        return count;
    }

    public int capacity() {
        return value.length;
    }

    public void ensureCapacity(int minimumCapacity) {
        if (minimumCapacity > value.length) {
            int newCapacity = (value.length + 1) * 2;
            if (newCapacity < 0) {
                newCapacity = Integer.MAX_VALUE;
            } else if (minimumCapacity > newCapacity) {
                newCapacity = minimumCapacity;
            }
            char[] newValue = new char[newCapacity];
            System.arraycopy(value, 0, newValue, 0, count);
            value = newValue;
        }
    }

    public void trimToSize() {
        if (count < value.length) {
            char[] newValue = new char[count];
            System.arraycopy(value, 0, newValue, 0, count);
            value = newValue;
        }
    }

    public void setLength(int newLength) {
        if (newLength < 0) {
            throw new StringIndexOutOfBoundsException(newLength);
        }
        ensureCapacity(newLength);
        for (int i = count; i < newLength; i++) {
            value[i] = '\0';
        }
        count = newLength;
    }

    public char charAt(int index) {
        if (index < 0 || index >= count) {
            throw new StringIndexOutOfBoundsException(index);
        }
        return value[index];
    }

    public void getChars(int srcBegin, int srcEnd, char[] dst, int dstBegin) {
        if (srcBegin < 0) {
            throw new StringIndexOutOfBoundsException(srcBegin);
        }
        if (srcEnd < 0 || srcEnd > count) {
            throw new StringIndexOutOfBoundsException(srcEnd);
        }
        if (srcBegin > srcEnd) {
            throw new StringIndexOutOfBoundsException(srcEnd - srcBegin);
        }
        System.arraycopy(value, srcBegin, dst, dstBegin, srcEnd - srcBegin);
    }

    public void setCharAt(int index, char ch) {
        if (index < 0 || index >= count) {
            throw new StringIndexOutOfBoundsException(index);
        }
        value[index] = ch;
    }

    public StringBuilder append(Object obj) {
        return append(String.valueOf(obj));
    }

    public StringBuilder append(String str) {
        if (str == null) {
            str = "null";
        }
        int len = str.length();
        ensureCapacity(count + len);
        str.getChars(0, len, value, count);
        count += len;
        return this;
    }

    public StringBuilder append(StringBuffer sb) {
        if (sb == null) {
            return append("null");
        }
        synchronized (sb) {
            int len = sb.length();
            ensureCapacity(count + len);
            sb.getChars(0, len, value, count);
            count += len;
        }
        return this;
    }

    public StringBuilder append(CharSequence s) {
        if (s == null) {
            return append("null");
        }
        if (s instanceof String) {
            return append((String) s);
        }
        return append(s, 0, s.length());
    }

    public StringBuilder append(CharSequence s, int start, int end) {
        if (s == null) {
            s = "null";
        }
        if (start < 0 || start > end || end > s.length()) {
            throw new IndexOutOfBoundsException();
        }
        ensureCapacity(count + end - start);
        for (int i = start; i < end; i++) {
            value[count++] = s.charAt(i);
        }
        return this;
    }

    public StringBuilder append(char[] str) {
        return append(str, 0, str.length);
    }

    public StringBuilder append(char[] str, int offset, int len) {
        ensureCapacity(count + len);
        System.arraycopy(str, offset, value, count, len);
        count += len;
        return this;
    }

    public StringBuilder append(boolean b) {
        return append(String.valueOf(b));
    }

    public StringBuilder append(char c) {
        ensureCapacity(count + 1);
        value[count++] = c;
        return this;
    }

    public StringBuilder append(int i) {
        return append(String.valueOf(i));
    }

    public StringBuilder append(long l) {
        return append(String.valueOf(l));
    }

    public StringBuilder append(float f) {
        return append(String.valueOf(f));
    }

    public StringBuilder append(double d) {
        return append(String.valueOf(d));
    }

    public StringBuilder delete(int start, int end) {
        if (start < 0) {
            throw new StringIndexOutOfBoundsException(start);
        }
        if (end > count) {
            end = count;
        }
        if (start > end) {
            throw new StringIndexOutOfBoundsException();
        }
        int len = end - start;
        if (len > 0) {
            System.arraycopy(value, end, value, start, count - end);
            count -= len;
        }
        return this;
    }

    public StringBuilder deleteCharAt(int index) {
        if (index < 0 || index >= count) {
            throw new StringIndexOutOfBoundsException(index);
        }
        System.arraycopy(value, index + 1, value, index, count - index - 1);
        count--;
        return this;
    }

    public StringBuilder replace(int start, int end, String str) {
        if (start < 0) {
            throw new StringIndexOutOfBoundsException(start);
        }
        if (start > count) {
            throw new StringIndexOutOfBoundsException();
        }
        if (start > end) {
            throw new StringIndexOutOfBoundsException();
        }
        if (end > count) {
            end = count;
        }
        int len = str.length();
        int newCount = count + len - (end - start);
        ensureCapacity(newCount);
        System.arraycopy(value, end, value, start + len, count - end);
        str.getChars(0, len, value, start);
        count = newCount;
        return this;
    }

    public StringBuilder insert(int offset, String str) {
        if (offset < 0 || offset > count) {
            throw new StringIndexOutOfBoundsException(offset);
        }
        if (str == null) {
            str = "null";
        }
        int len = str.length();
        ensureCapacity(count + len);
        System.arraycopy(value, offset, value, offset + len, count - offset);
        str.getChars(0, len, value, offset);
        count += len;
        return this;
    }

    public StringBuilder insert(int offset, Object obj) {
        return insert(offset, String.valueOf(obj));
    }

    public StringBuilder insert(int offset, char[] str) {
        return insert(offset, String.valueOf(str));
    }

    public StringBuilder insert(int offset, CharSequence s) {
        return insert(offset, s == null ? "null" : s.toString());
    }

    public StringBuilder insert(int offset, boolean b) {
        return insert(offset, String.valueOf(b));
    }

    public StringBuilder insert(int offset, char c) {
        if (offset < 0 || offset > count) {
            throw new StringIndexOutOfBoundsException(offset);
        }
        ensureCapacity(count + 1);
        System.arraycopy(value, offset, value, offset + 1, count - offset);
        value[offset] = c;
        count++;
        return this;
    }

    public StringBuilder insert(int offset, int i) {
        return insert(offset, String.valueOf(i));
    }

    public StringBuilder insert(int offset, long l) {
        return insert(offset, String.valueOf(l));
    }

    public StringBuilder insert(int offset, float f) {
        return insert(offset, String.valueOf(f));
    }

    public StringBuilder insert(int offset, double d) {
        return insert(offset, String.valueOf(d));
    }

    public int indexOf(String str) {
        return indexOf(str, 0);
    }

    public int indexOf(String str, int fromIndex) {
        return toString().indexOf(str, fromIndex);
    }

    public StringBuilder reverse() {
        for (int i = 0, j = count - 1; i < j; i++, j--) {
            char c = value[i];
            value[i] = value[j];
            value[j] = c;
        }
        return this;
    }

    public String substring(int start) {
        return substring(start, count);
    }

    public String substring(int start, int end) {
        if (start < 0) {
            throw new StringIndexOutOfBoundsException(start);
        }
        if (end > count) {
            throw new StringIndexOutOfBoundsException(end);
        }
        if (start > end) {
            throw new StringIndexOutOfBoundsException(end - start);
        }
        return new String(value, start, end - start);
    }

    public CharSequence subSequence(int start, int end) {
        return substring(start, end);
    }

    public String toString() {
        return new String(value, 0, count);
    }
}
