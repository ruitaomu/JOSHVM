/*
 * Regression test: compiled with -target 1.4 and preverified, so it uses
 * the CLDC StackMap attribute like every pre-existing application.
 */
public class LegacyPreverified {
    static int sum(int[] a) {
        int s = 0;
        for (int i = 0; i < a.length; i++) {
            if (a[i] > 0) {
                s += a[i];
            }
        }
        return s;
    }

    public static void main(String[] args) {
        String text;
        try {
            text = "sum=" + sum(new int[] { 1, -2, 3 });
        } catch (RuntimeException e) {
            text = "error";
        }
        System.out.println(text.equals("sum=4") ? "PASS" : "FAILED");
    }
}
