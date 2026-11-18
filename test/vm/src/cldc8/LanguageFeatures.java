import java.util.Iterator;
import java.util.NoSuchElementException;

/*
 * Java 5 to 7 language features that depend on the class library.
 */
public class LanguageFeatures {
    static int failures = 0;

    static void check(boolean ok, String what) {
        if (!ok) {
            failures++;
            System.out.println("FAIL: " + what);
        }
    }

    // --- generics, bridge methods, covariant returns

    static class Box<T extends Comparable<T>> implements Comparable<Box<T>> {
        final T value;

        Box(T value) {
            this.value = value;
        }

        public int compareTo(Box<T> other) {    // bridged: compareTo(Object)
            return value.compareTo(other.value);
        }

        Box<T> self() {
            return this;
        }
    }

    static class NamedBox extends Box<String> {
        NamedBox(String name) {
            super(name);
        }

        @Override
        NamedBox self() {                       // covariant, bridged
            return this;
        }
    }

    static <T extends Comparable<T>> T max(T a, T b) {
        return a.compareTo(b) >= 0 ? a : b;
    }

    // --- Iterable and the enhanced for statement

    static class Range implements Iterable<Integer> {
        final int from, to;

        Range(int from, int to) {
            this.from = from;
            this.to = to;
        }

        public Iterator<Integer> iterator() {
            return new Iterator<Integer>() {
                int next = from;

                public boolean hasNext() {
                    return next < to;
                }

                public Integer next() {
                    if (next >= to) {
                        throw new NoSuchElementException();
                    }
                    return next++;
                }

                public void remove() {
                    throw new UnsupportedOperationException();
                }
            };
        }
    }

    // --- varargs

    static int sum(int... values) {
        int total = 0;
        for (int v : values) {
            total += v;
        }
        return total;
    }

    @SafeVarargs
    static <T> int count(T... items) {
        return items.length;
    }

    // --- try-with-resources

    static StringBuilder log = new StringBuilder();

    static class Resource implements AutoCloseable {
        final String name;
        final boolean failOnClose;

        Resource(String name, boolean failOnClose) {
            this.name = name;
            this.failOnClose = failOnClose;
            log.append("open ").append(name).append(';');
        }

        public void close() throws Exception {
            log.append("close ").append(name).append(';');
            if (failOnClose) {
                throw new IllegalStateException("close " + name);
            }
        }
    }

    static String useResources(boolean failBody, boolean failClose)
            throws Exception {
        try (Resource a = new Resource("a", false);
             Resource b = new Resource("b", failClose)) {
            if (failBody) {
                throw new RuntimeException("body");
            }
            return a.name + b.name;
        }
    }

    // --- strings in switch

    static int code(String s) {
        switch (s) {
        case "one":   return 1;
        case "two":   return 2;
        case "Aa":    return 3;     // same hashCode as "BB"
        case "BB":    return 4;
        default:      return 0;
        }
    }

    @Deprecated
    static int old() {
        return 1;
    }

    @SuppressWarnings("deprecation")
    public static void main(String[] args) throws Exception {
        // string concatenation (StringBuilder)
        int n = 7;
        long big = 1L << 40;
        String text = "n=" + n + " big=" + big + " c=" + 'x' + " b=" + true
                      + " d=" + 1.5 + " o=" + null;
        check(text.equals("n=7 big=1099511627776 c=x b=true d=1.5 o=null"),
              "concatenation: " + text);

        StringBuilder sb = new StringBuilder("abc");
        sb.append(1).append('d').insert(0, "<").append(">").reverse();
        check(sb.toString().equals(">d1cba<"), "StringBuilder");
        check(sb.length() == 7 && sb.charAt(1) == 'd', "StringBuilder access");
        sb.setLength(2);
        sb.deleteCharAt(0).replace(0, 1, "xyz");
        check(sb.toString().equals("xyz"), "StringBuilder edit");

        CharSequence seq = "hello";
        check(seq.length() == 5 && seq.charAt(1) == 'e'
              && seq.subSequence(1, 3).toString().equals("el"),
              "String as CharSequence");
        check(new StringBuilder().append(seq, 1, 4).toString().equals("ell"),
              "append CharSequence");
        check(new StringBuilder(new StringBuffer("sb")).toString()
                  .equals("sb"), "StringBuffer as CharSequence");

        // autoboxing and unboxing
        Integer boxed = 5;
        int unboxed = boxed;
        check(unboxed == 5 && boxed.intValue() == 5, "Integer boxing");
        Integer small1 = 100, small2 = 100;
        check(small1 == small2, "small Integers are shared");
        Integer large1 = 100000, large2 = 100000;
        check(large1.equals(large2), "large Integers are equal");
        Object[] mixed = { true, (byte) 1, 'c', (short) 2, 3, 4L, 5f, 6.0 };
        check(mixed[0] instanceof Boolean && mixed[1] instanceof Byte
              && mixed[2] instanceof Character && mixed[3] instanceof Short
              && mixed[4] instanceof Integer && mixed[5] instanceof Long
              && mixed[6] instanceof Float && mixed[7] instanceof Double,
              "boxing types");
        Boolean yes = true;
        check(yes == Boolean.TRUE, "Boolean boxing");
        Character ch1 = 'a', ch2 = 'a';
        check(ch1 == ch2 && ch1 == 'a', "Character boxing");
        Long sum = 0L;
        for (int i = 0; i < 10; i++) {
            sum += i;
        }
        check(sum == 45L, "Long arithmetic");
        Double half = 0.5;
        check(half * 2 == 1.0, "Double unboxing");

        // generics
        check(max("pear", "apple").equals("pear"), "generic method");
        Box<String> b1 = new Box<>("a");            // diamond
        Box<String> b2 = new NamedBox("b");
        check(b1.compareTo(b2) < 0, "compareTo");
        Comparable raw = b2;
        check(raw.compareTo(b1) > 0, "bridge method");
        check(b2.self() == b2 && b2.self() instanceof NamedBox,
              "covariant return");

        // enhanced for
        int total = 0;
        for (int value : new Range(1, 5)) {
            total += value;
        }
        check(total == 10, "for-each over Iterable");
        total = 0;
        for (String s : new String[] { "a", "bb", "ccc" }) {
            total += s.length();
        }
        check(total == 6, "for-each over array");

        // varargs
        check(sum() == 0 && sum(1) == 1 && sum(1, 2, 3) == 6, "varargs");
        check(count("a", "b") == 2, "generic varargs");

        // try-with-resources
        check(useResources(false, false).equals("ab"), "resources");
        check(log.toString().equals("open a;open b;close b;close a;"),
              "close order: " + log);
        log.setLength(0);
        try {
            useResources(true, true);
            check(false, "exception expected");
        } catch (RuntimeException e) {
            check(e.getMessage().equals("body"), "primary exception");
            Throwable[] suppressed = e.getSuppressed();
            check(suppressed.length == 1
                  && suppressed[0] instanceof IllegalStateException
                  && suppressed[0].getMessage().equals("close b"),
                  "suppressed exception");
        }
        try {
            useResources(false, true);
            check(false, "close failure expected");
        } catch (IllegalStateException e) {
            check(e.getSuppressed().length == 0, "nothing suppressed");
        }

        // multi-catch
        String caught = null;
        for (int i = 0; i < 2; i++) {
            try {
                if (i == 0) {
                    throw new ArithmeticException("x");
                }
                Object o = "s";
                Integer bad = (Integer) o;
            } catch (ArithmeticException | ClassCastException e) {
                caught = (caught == null ? "" : caught)
                         + (e instanceof ArithmeticException ? "A" : "C");
            }
        }
        check("AC".equals(caught), "multi-catch");

        // strings in switch
        check(code("one") == 1 && code("two") == 2 && code("x") == 0,
              "string switch");
        check(code("Aa") == 3 && code("BB") == 4, "string switch collision");

        // assert compiles and is disabled
        boolean reached = false;
        assert reached = true;
        check(!reached, "assertions are disabled");
        check(!LanguageFeatures.class.desiredAssertionStatus(),
              "desiredAssertionStatus");
        check(new AssertionError("m").getMessage().equals("m"),
              "AssertionError");

        // literals
        check(0b1010 == 10 && 1_000_000 == 1000000, "Java 7 literals");
        check(old() == 1, "annotations");

        System.out.println(failures == 0 ? "PASS" : "FAILED");
    }
}
