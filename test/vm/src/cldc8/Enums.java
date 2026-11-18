/*
 * Enumerations. values() of every enum clones an array, which the VM
 * supports although java.lang.Object has no clone().
 */
public class Enums {
    static int failures = 0;

    static void check(boolean ok, String what) {
        if (!ok) {
            failures++;
            System.out.println("FAIL: " + what);
        }
    }

    enum Color { RED, GREEN, BLUE }

    enum Op {
        ADD("+") {
            int apply(int a, int b) { return a + b; }
        },
        MUL("*") {
            int apply(int a, int b) { return a * b; }
        };

        final String symbol;

        Op(String symbol) {
            this.symbol = symbol;
        }

        abstract int apply(int a, int b);
    }

    static int lateInitialized = 0;

    enum Late {
        ONLY;
        static {
            lateInitialized++;
        }
    }

    static String describe(Color c) {
        switch (c) {
        case RED:   return "warm";
        case BLUE:  return "cold";
        default:    return "other";
        }
    }

    public static void main(String[] args) {
        Color[] values = Color.values();
        check(values.length == 3, "values length");
        check(values[0] == Color.RED && values[1] == Color.GREEN
              && values[2] == Color.BLUE, "values order");
        check(values != Color.values(), "values() returns a new array");
        values[0] = null;
        check(Color.values()[0] == Color.RED, "values() is a copy");

        check(Color.GREEN.ordinal() == 1, "ordinal");
        check(Color.GREEN.name().equals("GREEN"), "name");
        check(Color.GREEN.toString().equals("GREEN"), "toString");
        check(("c=" + Color.BLUE).equals("c=BLUE"), "concatenation");
        check(Color.RED.compareTo(Color.BLUE) < 0, "compareTo");
        check(Color.RED.equals(Color.RED) && !Color.RED.equals(Color.BLUE),
              "equals");
        check(Color.RED.hashCode() == Color.RED.hashCode(), "hashCode");
        check(Color.RED.getDeclaringClass() == Color.class,
              "getDeclaringClass");

        check(Color.valueOf("GREEN") == Color.GREEN, "valueOf");
        check(Enum.valueOf(Color.class, "BLUE") == Color.BLUE,
              "Enum.valueOf");
        boolean rejected = false;
        try {
            Color.valueOf("PINK");
        } catch (IllegalArgumentException e) {
            rejected = true;
        }
        check(rejected, "valueOf unknown name");
        rejected = false;
        try {
            Color.valueOf(null);
        } catch (NullPointerException e) {
            rejected = true;
        }
        check(rejected, "valueOf null");

        check(describe(Color.RED).equals("warm"), "switch RED");
        check(describe(Color.BLUE).equals("cold"), "switch BLUE");
        check(describe(Color.GREEN).equals("other"), "switch default");

        int seen = 0;
        for (Color c : Color.values()) {
            seen += c.ordinal() + 1;
        }
        check(seen == 6, "for-each over values()");

        // Constants with bodies are instances of anonymous subclasses.
        check(Op.ADD.apply(3, 4) == 7 && Op.MUL.apply(3, 4) == 12, "bodies");
        check(Op.MUL.symbol.equals("*"), "constructor argument");
        check(Op.ADD.getClass() != Op.class, "constant class");
        check(Op.ADD.getDeclaringClass() == Op.class,
              "getDeclaringClass with body");
        check(Op.valueOf("MUL") == Op.MUL, "valueOf with body");
        check(Op.values().length == 2, "values with bodies");
        rejected = false;
        try {
            ((Comparable) Op.ADD).compareTo(Color.RED);
        } catch (ClassCastException e) {
            rejected = true;
        }
        check(rejected, "compareTo across enum types");

        // valueOf(Class, String) must initialize the enum class itself.
        check(lateInitialized == 0, "enum not initialized early");
        check(Enum.valueOf(Late.class, "ONLY").ordinal() == 0,
              "valueOf initializes");
        check(lateInitialized == 1, "enum initialized once");

        // Enums in the ROM image of the class library
        check(java.lang.annotation.ElementType.values().length == 8,
              "ROMized enum values()");
        check(java.lang.annotation.RetentionPolicy.valueOf("RUNTIME")
                  .ordinal() == 2, "ROMized enum valueOf");

        // Often enough to get values() compiled.
        int total = 0;
        for (int i = 0; i < 20000; i++) {
            total += Color.values().length;
        }
        check(total == 60000, "values() repeated");

        System.out.println(failures == 0 ? "PASS" : "FAILED");
    }
}
