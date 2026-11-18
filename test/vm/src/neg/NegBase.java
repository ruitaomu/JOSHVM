/*
 * Base class for the negative tests. run-tests.sh patches the compiled
 * class with ClassPatcher and expects the VM to reject each variant.
 * Unpatched, it must run and print PASS.
 */
public class NegBase {
    // Bytecode: 0 iload_0, 1 ifle 6, 4 iconst_1, 5 ireturn,
    //           6 iconst_2, 7 ireturn; one same_frame at bci 6.
    static int branch(int x) {
        if (x > 0) {
            return 1;
        }
        return 2;
    }

    static void iface(Runnable r) {
        r.run();
    }

    public static void main(String[] args) {
        if (branch(1) == 1 && branch(0) == 2) {
            System.out.println("PASS");
        }
    }
}
