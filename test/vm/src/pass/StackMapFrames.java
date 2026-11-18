/*
 * Exercises StackMapTable frame types in a class compiled with
 * -target 1.7 and not preverified.
 *
 * No string concatenation with '+': at this target javac emits
 * StringBuilder calls, which the CLDC 1.1.1 library does not have.
 */
public class StackMapFrames {
    static int failures = 0;

    static void check(boolean ok, String what) {
        if (!ok) {
            failures++;
            System.out.print("FAIL: ");
            System.out.println(what);
        }
    }

    final long seed;
    final Object tag;

    // uninitializedThis across a branch, long argument
    StackMapFrames(long seed, boolean flag) {
        this(flag ? seed : -seed, flag ? (Object) "t" : (Object) new Object());
    }

    StackMapFrames(long seed, Object tag) {
        super();
        this.seed = seed;
        this.tag = tag;
    }

    // same_frame, append_frame, chop_frame
    static int loops(int n) {
        int sum = 0;
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < i; j++) {
                sum += j;
            }
        }
        return sum;
    }

    // long and double locals live across branch targets
    static double wide(long a, double b, int n) {
        long acc = a;
        double d = b;
        while (n-- > 0) {
            if ((n & 1) == 0) {
                acc += n;
            } else {
                d *= 2.0;
            }
        }
        return d + (double) acc;
    }

    // same_locals_1_stack_item (exception handlers)
    static String handlers(Object o, int[] arr, int idx) {
        try {
            return new StringBuffer(o.toString()).append(arr[idx]).toString();
        } catch (NullPointerException e) {
            return "npe";
        } catch (ArrayIndexOutOfBoundsException e) {
            return "aioobe";
        } finally {
            failures += 0;
        }
    }

    // uninitialized object (new) on the stack across a branch
    static StringBuffer newAcrossBranch(boolean big) {
        return new StringBuffer(big ? 64 : 8);
    }

    // many locals of mixed types and a switch
    static int switches(int k, long l, float f, Object o, int[] a) {
        int r;
        switch (k) {
        case 0:  r = 10; break;
        case 1:  r = (int) l; break;
        case 2:  r = (int) f; break;
        case 100: r = a.length; break;
        default: r = o == null ? -1 : -2; break;
        }
        return r;
    }

    synchronized int locked(int x) {
        synchronized (tag) {
            if (x > 0) {
                return x + 1;
            }
        }
        return x - 1;
    }

    public static void main(String[] args) {
        check(loops(5) == 10, "loops");
        check(wide(1L, 1.5, 4) == 6.0 + 1.0 + 2.0, "wide");
        check(handlers("a", new int[] { 7 }, 0).equals("a7"), "handlers ok");
        check(handlers(null, new int[] { 7 }, 0).equals("npe"), "handlers npe");
        check(handlers("a", new int[] { 7 }, 3).equals("aioobe"), "handlers aioobe");
        check(newAcrossBranch(true).capacity() == 64, "new across branch");
        check(switches(0, 5L, 6f, null, null) == 10, "switch 0");
        check(switches(1, 5L, 6f, null, null) == 5, "switch 1");
        check(switches(2, 5L, 6f, null, null) == 6, "switch 2");
        check(switches(100, 5L, 6f, null, new int[3]) == 3, "switch 100");
        check(switches(9, 5L, 6f, "x", null) == -2, "switch default");
        StackMapFrames s = new StackMapFrames(4L, true);
        check(s.seed == 4L && "t".equals(s.tag), "ctor true");
        s = new StackMapFrames(4L, false);
        check(s.seed == -4L && s.tag != null, "ctor false");
        check(s.locked(1) == 2 && s.locked(0) == -1, "locked");
        System.out.println(failures == 0 ? "PASS" : "FAILED");
    }
}
