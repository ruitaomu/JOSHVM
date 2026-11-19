/*
 * Control for the tests that run compiled code: hot methods and a hot
 * loop in main using only what every earlier class library version has,
 * so a failure here that the interpreter does not show is in the compiler
 * rather than in a newer feature.
 */
public class HotLoops {
    static int failures = 0;

    static void check(boolean ok, String what) {
        if (!ok) {
            failures++;
            System.out.print("FAIL: ");
            System.out.println(what);
        }
    }

    static class Counter {
        int count;

        void add(int n) {
            count += n;
        }
    }

    static class Doubler extends Counter {
        void add(int n) {
            count += 2 * n;
        }
    }

    static int sum(int[] values) {
        int total = 0;
        for (int i = 0; i < values.length; i++) {
            total += values[i];
        }
        return total;
    }

    static long mix(long a, int b) {
        return (a << 3) ^ (a >>> 5) ^ b;
    }

    static int divide(int a, int b) {
        try {
            return a / b;
        } catch (ArithmeticException e) {
            return -1;
        }
    }

    static Object[] pair(Object first, Object second) {
        return new Object[] { first, second };
    }

    public static void main(String[] args) {
        int[] values = { 1, 2, 3, 4 };
        Counter plain = new Counter();
        Counter doubled = new Doubler();
        long hash = 7;
        int total = 0;
        int quotients = 0;
        Object kept = null;

        for (int i = 0; i < 20000; i++) {
            total += sum(values);
            hash = mix(hash, i);
            quotients += divide(i, i & 3);
            ((i & 1) == 0 ? plain : doubled).add(1);
            Object[] made = pair(plain, values);
            if (made[1] == values) {
                kept = made;
            }
        }

        long expected = 7;
        for (int i = 0; i < 20000; i++) {
            expected = (expected << 3) ^ (expected >>> 5) ^ i;
        }

        check(total == 200000, "array sum");
        check(hash == expected, "long arithmetic");
        // i / 0 for every fourth i, else i/1 + i/2 + i/3
        int wanted = 0;
        for (int i = 0; i < 20000; i++) {
            wanted += (i & 3) == 0 ? -1 : i / (i & 3);
        }
        check(quotients == wanted, "division and exception");
        check(plain.count == 10000 && doubled.count == 20000, "virtual call");
        check(kept != null && ((Object[]) kept)[0] == plain, "allocation");

        System.out.println(failures == 0 ? "PASS" : "FAILED");
    }
}
