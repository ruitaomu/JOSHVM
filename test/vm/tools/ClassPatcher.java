import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

/**
 * Host-side tool that rewrites a class file to produce the malformed or
 * unsupported inputs used by the negative VM tests.
 *
 * Usage: ClassPatcher in.class out.class op [args] [op [args]] ...
 *
 *   version MAJOR            set the major version
 *   cptag FROM TO            change the tag of the first constant with tag FROM
 *   utf8 OLD NEW             replace the content of a Utf8 constant
 *   code METHOD PAT REPL     in METHOD's bytecode, replace the first match of
 *                            the hex pattern PAT ("??" matches any byte) by
 *                            REPL ("??" keeps the original byte)
 *   smt METHOD HEX           replace the data of METHOD's StackMapTable
 *                            attribute by HEX; "-" removes the attribute
 *   fuzzsmt SEED             randomly damage the StackMapTable of one
 *                            method, reproducibly for a given SEED
 */
public class ClassPatcher {

    static class Attr {
        int name;
        byte[] data;
    }

    static class Member {
        int access, name, desc;
        List<Attr> attrs;
    }

    int minor, major;
    int cpCount;
    // tag and raw payload per constant; entries after long/double are null
    int[] cpTag;
    byte[][] cpData;
    int access, thisClass, superClass;
    int[] interfaces;
    List<Member> fields, methods;
    List<Attr> attrs;

    void read(String path) throws IOException {
        DataInputStream in = new DataInputStream(new FileInputStream(path));
        try {
            if (in.readInt() != 0xCAFEBABE) {
                throw new IOException("not a class file: " + path);
            }
            minor = in.readUnsignedShort();
            major = in.readUnsignedShort();
            cpCount = in.readUnsignedShort();
            cpTag = new int[cpCount];
            cpData = new byte[cpCount][];
            for (int i = 1; i < cpCount; i++) {
                int tag = in.readUnsignedByte();
                int len;
                switch (tag) {
                case 1:  len = -1; break;
                case 3: case 4: case 9: case 10: case 11: case 12: case 18:
                    len = 4; break;
                case 5: case 6: len = 8; break;
                case 7: case 8: case 16: len = 2; break;
                case 15: len = 3; break;
                default: throw new IOException("unknown constant tag " + tag);
                }
                cpTag[i] = tag;
                if (len < 0) {
                    len = in.readUnsignedShort();
                }
                cpData[i] = new byte[len];
                in.readFully(cpData[i]);
                if (tag == 5 || tag == 6) {
                    i++;
                }
            }
            access = in.readUnsignedShort();
            thisClass = in.readUnsignedShort();
            superClass = in.readUnsignedShort();
            interfaces = new int[in.readUnsignedShort()];
            for (int i = 0; i < interfaces.length; i++) {
                interfaces[i] = in.readUnsignedShort();
            }
            fields = readMembers(in);
            methods = readMembers(in);
            attrs = readAttrs(in);
        } finally {
            in.close();
        }
    }

    static List<Member> readMembers(DataInputStream in) throws IOException {
        int n = in.readUnsignedShort();
        List<Member> list = new ArrayList<Member>();
        for (int i = 0; i < n; i++) {
            Member m = new Member();
            m.access = in.readUnsignedShort();
            m.name = in.readUnsignedShort();
            m.desc = in.readUnsignedShort();
            m.attrs = readAttrs(in);
            list.add(m);
        }
        return list;
    }

    static List<Attr> readAttrs(DataInputStream in) throws IOException {
        int n = in.readUnsignedShort();
        List<Attr> list = new ArrayList<Attr>();
        for (int i = 0; i < n; i++) {
            Attr a = new Attr();
            a.name = in.readUnsignedShort();
            a.data = new byte[in.readInt()];
            in.readFully(a.data);
            list.add(a);
        }
        return list;
    }

    void write(String path) throws IOException {
        DataOutputStream out = new DataOutputStream(new FileOutputStream(path));
        try {
            out.writeInt(0xCAFEBABE);
            out.writeShort(minor);
            out.writeShort(major);
            out.writeShort(cpCount);
            for (int i = 1; i < cpCount; i++) {
                if (cpData[i] == null) {
                    continue;
                }
                out.writeByte(cpTag[i]);
                if (cpTag[i] == 1) {
                    out.writeShort(cpData[i].length);
                }
                out.write(cpData[i]);
            }
            out.writeShort(access);
            out.writeShort(thisClass);
            out.writeShort(superClass);
            out.writeShort(interfaces.length);
            for (int i = 0; i < interfaces.length; i++) {
                out.writeShort(interfaces[i]);
            }
            writeMembers(out, fields);
            writeMembers(out, methods);
            writeAttrs(out, attrs);
        } finally {
            out.close();
        }
    }

    static void writeMembers(DataOutputStream out, List<Member> list)
            throws IOException {
        out.writeShort(list.size());
        for (Member m : list) {
            out.writeShort(m.access);
            out.writeShort(m.name);
            out.writeShort(m.desc);
            writeAttrs(out, m.attrs);
        }
    }

    static void writeAttrs(DataOutputStream out, List<Attr> list)
            throws IOException {
        out.writeShort(list.size());
        for (Attr a : list) {
            out.writeShort(a.name);
            out.writeInt(a.data.length);
            out.write(a.data);
        }
    }

    String utf8(int index) throws IOException {
        if (cpTag[index] != 1) {
            throw new IOException("constant " + index + " is not Utf8");
        }
        return new String(cpData[index], "UTF-8");
    }

    Attr findAttr(List<Attr> list, String name) throws IOException {
        for (Attr a : list) {
            if (utf8(a.name).equals(name)) {
                return a;
            }
        }
        return null;
    }

    Attr codeOf(String method) throws IOException {
        for (Member m : methods) {
            if (utf8(m.name).equals(method)) {
                Attr code = findAttr(m.attrs, "Code");
                if (code != null) {
                    return code;
                }
            }
        }
        throw new IOException("no method with code: " + method);
    }

    static int u2(byte[] b, int off) {
        return ((b[off] & 0xff) << 8) | (b[off + 1] & 0xff);
    }

    static int u4(byte[] b, int off) {
        return (u2(b, off) << 16) | u2(b, off + 2);
    }

    /** Offset of the attribute table (its count) inside Code data. */
    static int codeAttrsOffset(byte[] code) {
        int codeLength = u4(code, 4);
        int pos = 8 + codeLength;
        return pos + 2 + 8 * u2(code, pos);
    }

    static byte[] hex(String s) {
        byte[] b = new byte[s.length() / 2];
        for (int i = 0; i < b.length; i++) {
            b[i] = (byte) Integer.parseInt(s.substring(2 * i, 2 * i + 2), 16);
        }
        return b;
    }

    void opVersion(int v) {
        major = v;
    }

    void opCpTag(int from, int to) throws IOException {
        for (int i = 1; i < cpCount; i++) {
            if (cpData[i] != null && cpTag[i] == from) {
                cpTag[i] = to;
                return;
            }
        }
        throw new IOException("no constant with tag " + from);
    }

    void opUtf8(String oldValue, String newValue) throws IOException {
        for (int i = 1; i < cpCount; i++) {
            if (cpData[i] != null && cpTag[i] == 1
                    && utf8(i).equals(oldValue)) {
                cpData[i] = newValue.getBytes("UTF-8");
                return;
            }
        }
        throw new IOException("no Utf8 constant: " + oldValue);
    }

    void opCode(String method, String pattern, String replacement)
            throws IOException {
        byte[] code = codeOf(method).data;
        int codeLength = u4(code, 4);
        int n = pattern.length() / 2;
        for (int start = 8; start + n <= 8 + codeLength; start++) {
            boolean match = true;
            for (int i = 0; i < n && match; i++) {
                String p = pattern.substring(2 * i, 2 * i + 2);
                match = p.equals("??")
                        || (code[start + i] & 0xff) == Integer.parseInt(p, 16);
            }
            if (match) {
                for (int i = 0; i < replacement.length() / 2; i++) {
                    String r = replacement.substring(2 * i, 2 * i + 2);
                    if (!r.equals("??")) {
                        code[start + i] = (byte) Integer.parseInt(r, 16);
                    }
                }
                return;
            }
        }
        throw new IOException("pattern " + pattern + " not found in " + method);
    }

    List<Attr> readCodeAttrs(Attr code) throws IOException {
        int tableOffset = codeAttrsOffset(code.data);
        DataInputStream in = new DataInputStream(
                new java.io.ByteArrayInputStream(code.data, tableOffset,
                        code.data.length - tableOffset));
        return readAttrs(in);
    }

    void writeCodeAttrs(Attr code, List<Attr> codeAttrs) throws IOException {
        ByteArrayOutputStream bytes = new ByteArrayOutputStream();
        bytes.write(code.data, 0, codeAttrsOffset(code.data));
        writeAttrs(new DataOutputStream(bytes), codeAttrs);
        code.data = bytes.toByteArray();
    }

    void opSmt(String method, String data) throws IOException {
        Attr code = codeOf(method);
        List<Attr> codeAttrs = readCodeAttrs(code);
        Attr smt = findAttr(codeAttrs, "StackMapTable");
        if (smt == null) {
            throw new IOException("no StackMapTable in " + method);
        }
        if (data.equals("-")) {
            codeAttrs.remove(smt);
        } else {
            smt.data = hex(data);
        }
        writeCodeAttrs(code, codeAttrs);
    }

    static final int[] INTERESTING = {
        0, 1, 5, 6, 7, 8, 9, 63, 64, 127, 128, 246, 247, 248, 250, 251, 252,
        254, 255
    };

    /** Randomly damages the StackMapTable of one method. */
    void opFuzzSmt(long seed) throws IOException {
        java.util.Random random = new java.util.Random(seed);
        List<Attr> candidates = new ArrayList<Attr>();
        for (Member m : methods) {
            Attr code = findAttr(m.attrs, "Code");
            if (code != null
                    && findAttr(readCodeAttrs(code), "StackMapTable") != null) {
                candidates.add(code);
            }
        }
        if (candidates.isEmpty()) {
            throw new IOException("no method has a StackMapTable");
        }
        Attr code = candidates.get(random.nextInt(candidates.size()));
        List<Attr> codeAttrs = readCodeAttrs(code);
        Attr smt = findAttr(codeAttrs, "StackMapTable");
        byte[] data = smt.data;
        switch (random.nextInt(4)) {
        case 0:         // random values in up to three bytes
            for (int n = 1 + random.nextInt(3); n > 0; n--) {
                data[random.nextInt(data.length)] = (byte) random.nextInt(256);
            }
            break;
        case 1:         // boundary values in up to two bytes
            for (int n = 1 + random.nextInt(2); n > 0; n--) {
                data[random.nextInt(data.length)] =
                    (byte) INTERESTING[random.nextInt(INTERESTING.length)];
            }
            break;
        case 2: {       // truncated
            byte[] shorter = new byte[random.nextInt(data.length)];
            System.arraycopy(data, 0, shorter, 0, shorter.length);
            data = shorter;
            break;
        }
        default: {      // trailing bytes
            byte[] longer = new byte[data.length + 1 + random.nextInt(4)];
            random.nextBytes(longer);
            System.arraycopy(data, 0, longer, 0, data.length);
            data = longer;
            break;
        }
        }
        smt.data = data;
        writeCodeAttrs(code, codeAttrs);
    }

    public static void main(String[] args) throws IOException {
        ClassPatcher p = new ClassPatcher();
        p.read(args[0]);
        int i = 2;
        while (i < args.length) {
            String op = args[i++];
            if (op.equals("version")) {
                p.opVersion(Integer.parseInt(args[i++]));
            } else if (op.equals("cptag")) {
                p.opCpTag(Integer.parseInt(args[i]),
                          Integer.parseInt(args[i + 1]));
                i += 2;
            } else if (op.equals("utf8")) {
                p.opUtf8(args[i], args[i + 1]);
                i += 2;
            } else if (op.equals("code")) {
                p.opCode(args[i], args[i + 1], args[i + 2]);
                i += 3;
            } else if (op.equals("smt")) {
                p.opSmt(args[i], args[i + 1]);
                i += 2;
            } else if (op.equals("fuzzsmt")) {
                p.opFuzzSmt(Long.parseLong(args[i++]));
            } else {
                throw new IOException("unknown operation: " + op);
            }
        }
        p.write(args[1]);
    }
}
