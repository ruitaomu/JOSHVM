/*
 * clone() on arrays. java.lang.Object has no clone() in this library; the
 * VM supports it for array receivers only.
 */
public class ArrayClone {
    static int failures = 0;

    static void check(boolean ok, String what) {
        if (!ok) {
            failures++;
            System.out.print("FAIL: ");
            System.out.println(what);
        }
    }

    static int[] cloneInts(int[] a) {
        return (int[]) a.clone();
    }

    public static void main(String[] args) {
        int[] ints = { 1, 2, 3 };
        int[] intsCopy = (int[]) ints.clone();
        check(intsCopy != ints, "int[] distinct");
        check(intsCopy.length == 3 && intsCopy[0] == 1 && intsCopy[2] == 3,
              "int[] content");
        intsCopy[0] = 9;
        check(ints[0] == 1, "int[] independent");

        byte[] bytes = { 1, 2, 3, 4, 5 };
        byte[] bytesCopy = (byte[]) bytes.clone();
        check(bytesCopy.length == 5 && bytesCopy[4] == 5, "byte[]");

        char[] chars = "hello".toCharArray();
        check(new String((char[]) chars.clone()).equals("hello"), "char[]");

        short[] shorts = { -1, 7 };
        check(((short[]) shorts.clone())[0] == -1, "short[]");

        boolean[] flags = { true, false, true };
        check(((boolean[]) flags.clone())[2], "boolean[]");

        long[] longs = { 1L << 40, -5L };
        long[] longsCopy = (long[]) longs.clone();
        check(longsCopy[0] == (1L << 40) && longsCopy[1] == -5L, "long[]");

        double[] doubles = { 1.5, -2.25 };
        check(((double[]) doubles.clone())[1] == -2.25, "double[]");

        float[] floats = { 0.5f };
        check(((float[]) floats.clone())[0] == 0.5f, "float[]");

        String[] strings = { "a", "b" };
        String[] stringsCopy = (String[]) strings.clone();
        check(stringsCopy != strings && stringsCopy[1] == strings[1],
              "String[] shallow");
        check(stringsCopy.getClass() == String[].class, "String[] type");

        int[][] nested = { { 1 }, { 2, 3 } };
        int[][] nestedCopy = (int[][]) nested.clone();
        check(nestedCopy != nested && nestedCopy[1] == nested[1],
              "int[][] shallow");

        check(((int[]) new int[0].clone()).length == 0, "empty");

        Object asObject = ints;
        check(asObject instanceof int[], "sanity");

        boolean npe = false;
        try {
            cloneInts(null);
        } catch (NullPointerException e) {
            npe = true;
        }
        check(npe, "null receiver");

        // Often enough to get cloneInts compiled.
        int total = 0;
        for (int i = 0; i < 20000; i++) {
            total += cloneInts(ints).length;
        }
        check(total == 60000, "repeated");

        // Large enough to force collections while cloning.
        Object[] keep = new Object[8];
        byte[] big = new byte[4096];
        big[4095] = 42;
        for (int i = 0; i < 4000; i++) {
            keep[i & 7] = big.clone();
        }
        check(((byte[]) keep[3])[4095] == 42, "gc");

        System.out.println(failures == 0 ? "PASS" : "FAILED");
    }
}
