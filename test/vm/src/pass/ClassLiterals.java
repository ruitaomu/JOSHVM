/*
 * ldc / ldc_w of a class constant (class literals), class file version 51.
 */
public class ClassLiterals {
    static int initialized = 0;

    static class Lazy {
        static {
            initialized++;
        }
        static int touch() {
            return 1;
        }
    }

    public static void main(String[] args) throws Exception {
        boolean ok = true;

        Class self = ClassLiterals.class;
        ok &= self.getName().equals("ClassLiterals");
        ok &= self == Class.forName("ClassLiterals");
        ok &= self == new ClassLiterals().getClass();

        ok &= String.class.getName().equals("java.lang.String");
        ok &= String.class == "x".getClass();
        ok &= int[].class == new int[0].getClass();
        ok &= String[][].class.isArray();
        ok &= Runnable.class.isInterface();

        // A class literal must not initialize the class.
        Class lazy = Lazy.class;
        ok &= initialized == 0;
        ok &= lazy.getName().equals("ClassLiterals$Lazy");
        Lazy.touch();
        ok &= initialized == 1;
        ok &= lazy == Lazy.class;

        // Run the same literal often enough to get the method compiled.
        int same = 0;
        for (int i = 0; i < 20000; i++) {
            if (literal() == self) {
                same++;
            }
        }
        ok &= same == 20000;

        synchronized (ClassLiterals.class) {
            ok &= true;
        }

        System.out.println(ok ? "PASS" : "FAILED");
    }

    static Class literal() {
        return ClassLiterals.class;
    }
}
