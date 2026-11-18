/*
 * Base class for the method-resolution negative tests: the method
 * reference owned by the array class "[I" is patched by run-tests.sh.
 * Unpatched, clone() on a null array throws NullPointerException.
 */
public class NegClone {
    static Object c(int[] a) {
        return a.clone();
    }

    public static void main(String[] args) {
        try {
            c(null);
        } catch (NullPointerException e) {
            System.out.println("PASS");
        }
    }
}
