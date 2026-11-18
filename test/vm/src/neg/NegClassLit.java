/*
 * Base class for the "ldc of a class constant in an old class file" test.
 * No branches, so it needs no stack maps when its version is patched down.
 */
public class NegClassLit {
    public static void main(String[] args) {
        Class c = NegClassLit.class;
        System.out.println(c == null ? "FAILED" : "PASS");
    }
}
