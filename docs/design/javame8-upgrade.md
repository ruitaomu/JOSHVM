# Detailed Design for Upgrading JOSHVM to Java ME 8

Status: P1 (VM core) and the main body of P2 (libraries required for language features) have been implemented and tested on Linux i386 and Win32 i386 targets.
See "Implementation Status" below. P3 and P4 have not started.

## Implementation Status (2026-11-19)

### Implemented and Verified

Test environments: Ubuntu 24.04, gcc 13, Windows/MSVC, and OpenJDK 8. Linux 32-bit programs run
under `qemu-i386`. `test/vm/run-tests.sh` passes for the debug (`cldc_vm_g`) and product (`cldc_vm`) VMs,
dynamic JIT, `-int`, and forced compilation `-comp`, and it covers both class-library configurations.

| Item | Implementation |
|---|---|
| Maximum version 51 | `JAVA_MAX_SUPPORTED_VERSION`; 52 is rejected |
| `invokedynamic` | Constant tags 15, 16, and 18 and bytecode 0xBA are all rejected with "invokedynamic is not supported" |
| `StackMapTable` | `ClassFileParser::parse_stackmap_table()`, used for version 50 and above, expands to the existing internal format during parsing; verifier unchanged |
| `ldc` class constants | Interpreter dispatches through `quicken()` to `_fast_class_ldc` and `_fast_class_ldc_w`, calling runtime `ldc_class()`; the verifier rejects classes below version 49 |
| Array `clone` | `ConstantPool::array_clone_method_at()` binds `clone()Object` on array classes to `JVM.cloneArray()`; no method is added to `Object` |
| Method references on array classes | Previously array classes were treated as instance classes (assertion triggered in debug builds). Resolution now proceeds as it would for `Object`; field references and interface method references throw `IncompatibleClassChangeError` |
| Class library | With `ENABLE_CLDC_8=true`, compile with `-source 1.7 -target 1.7`, do not preverify, and ROMize the full library |
| Added classes | `StringBuilder`, `CharSequence`, `Comparable`, `Enum`, `Iterable`, `java.util.Iterator`, `AutoCloseable`, `AssertionError`, `LinkageError`, `IncompatibleClassChangeError`, `NoSuchFieldError`, seven types in `java.lang.annotation`, `Override`, `Deprecated`, `SuppressWarnings`, `SafeVarargs` |
| Modified classes | Eight wrapper classes add `valueOf(primitive type)`; `String` and `StringBuffer` implement `CharSequence` (`String` also implements `Comparable<String>`); `Throwable` adds `addSuppressed` and `getSuppressed`; `Class` is genericized and adds `desiredAssertionStatus()` |

### Build and Tests

```sh
# Default configuration (CLDC 1.1.1 class library)
./build-linux-i386-cldc.sh -C -J <JDK 8>
# Class library with Java 7 language level
ENABLE_CLDC_8=true ./build-linux-i386-cldc.sh -C -J <JDK 8>
# Tests (against the most recent build)
JDK_DIR=<JDK 8> test/vm/run-tests.sh
```

- Both configurations share the output directory, so switching requires `-C`.
- `PREVERIFY_ORIGINAL=<path>`: use a preverify executable that you compiled yourself. The precompiled one in the repository neither reports errors nor produces output on this machine,
  so build it from `preverifier/build/linux` (see `tools/docker/linux-i386/build-and-test.sh`).
- `HOST_EXE_RUN=qemu-i386`: run preverify, loopgen, romgen, and tests on a machine that cannot execute 32-bit programs directly.
- `tools/docker/linux-i386/` and `.github/workflows/build-i386.yml` package the Linux and Win32 i386
  steps described above. The latter's two jobs build and test the default class library and the `ENABLE_CLDC_8=true` class library, respectively.

The following additional changes were made so that the baseline can build and run with the current toolchain:

- `cldc/build/share/jvm.make`: add `-fno-strict-aliasing -fno-delete-null-pointer-checks
  -fno-lifetime-dse -fwrapv -Wno-narrowing` to gcc builds (without these, romgen segfaults directly). **This affects every target using gcc**,
  including ESP32 and STM32, which were not built here.
- `jvm.make`, `root.make`: source-file lists are no longer passed through shell command lines (which exceed the single-argument length limit); GNU make 4.0 or later is required.
- `cldc/src/vm/os/linux/OS_linux.cpp`: `MINSIGSTKSZ` is no longer a compile-time constant.
- `javacall/implementation/linux/cldc/`: remove `stropts.h`; change the Java heap to executable `mmap` (the JIT could not execute code in the
  heap where it was previously allocated with `malloc`); timer threads stop using condition variables instead of `pthread_cancel`.

### Deviations from the Design Below

The sections below preserve the original pre-implementation design. The implementation differs as follows:

| Section | Original design | Actual approach | Reason |
|---|---|---|---|
| 3 | The `cldc8` directory would be a complete copy of `cldc1.1.1` | `cldc8` contains only added and modified files, and the build overlays files with the same paths in `cldc1.1.1` (and in `Comparable` in the DIO module) | Avoid copying 36,000 lines of unchanged code |
| 4.4 | Plan A was "do not rewrite, always take the slow path," but interpreter dispatch was unclear | Add two bytecodes that exist only in the dispatch table; `ldc` in a method is never rewritten. Change the templates for `ldc` and `ldc_w` from "redo" to "dispatch according to the return value" | Isolate class mirrors are per-task, so they cannot be cached in a shared constant pool; not rewriting methods also avoids touching GC stack maps, ROM output, and the debugger |
| 4.4 | The version check location was unspecified | Add an internal class flag, `JVM_ACC_LDC_CLASS_ALLOWED` (bit 21), and check it in the verifier | The class-file version is no longer available during verification |
| 4.4 | The JIT would abandon compilation of an entire method when encountering a class constant | Generate an uncommon trap at that bytecode and return to the interpreter | This reuses an existing mechanism, while the rest of the method remains compilable |
| 4.8 | The entry form was undecided (a hidden native was favored) | In `com.sun.cldchi.jvm.JVM`, use `private final native Object cloneArray()`; the receiver is the array | This class is in a hidden package, is not renamed during ROMization, and native methods are not removed as dead code |
| 5 | `Enum.valueOf` would use Plan A: native code reads `$VALUES` by name | Plan B: register every constant in a table in `Enum` as it is constructed | Product ROM images do not preserve field names, so Plan A does not work for ROMized enum classes; Plan B does not depend on javac naming |
| 5 | OpenJDK 7 would be the first source | All added classes are rewritten, and modified classes are based on the original `cldc1.1.1` files | The required classes are all small |

### Not Done or Not Verified

- **ARM and Thumb-2 interpreters**: `bc_fast_class_ldc` in `TemplateTable_arm.cpp` and `TemplateTable_thumb2.cpp`
  is modeled on `bc_new` and **has not been compiled or run**. Class literals must be verified first on these targets.
- **Other targets**: Only Linux i386 was built and tested. `ENABLE_CLDC_8` was not added to `build-*.sh` for other platforms.
- **Size**: The ROM/RAM increments required by Section 7 for Thumb-2 configurations were not measured. i386 values are in Section 7.
- **Differential testing** (comparison with HotSpot verifier conclusions) was not performed. Fuzz testing covered only the `StackMapTable`
  attribute, with 400 variants in a single run (40 is the default).
- **Not done for P2**: `java.io.Closeable` and stream/connection classes implementing `AutoCloseable`; `Vector` and similar classes implementing `Iterable`;
  wrapper classes implementing `Comparable`; `Number`; `Appendable`; genericization of existing collection classes.
- **Layout constraint checklist and startup assertions in 4.9**: Not done. The modified `Throwable`, `Class`, and `String` classes only have fields or methods appended at the end.
- **Debugger**: kdp breakpoints and stepping for the new class format were not verified.
- **Application-side SDK**: `sdk/` and `test/rule.gmk` were not changed.
- VM-thrown missing-field errors are still `java.lang.Error` with text, rather than the new `NoSuchFieldError`.

## 0. Reading Notes

This document has two kinds of evidence, with different confidence levels:

- **Verified**: Conclusions obtained directly from code in this repository, with file and line references.
- **Pending verification**: Statements concerning the Java ME 8 specifications (CLDC 8 / JSR 360, MEEP 8 / JSR 361) are based on memory,
  without comparison against the specification text. Section 10 lists all items that still require verification before work begins.

- **Confirmed**: The project lead confirmed four prerequisites on 2026-11-18 (see Section 10): the class-file maximum version is 51;
  `invokedynamic` is unsupported; the VM must accept `StackMapTable` directly; and `Object.clone` is not in CLDC 8 `Object`.

The design in Section 1 and later was written before implementation, with no build or runtime verification; implementation status is given in "Implementation Status" above.

## 1. Goals and Scope

### Goals

| Phase | Deliverable | Completion Criteria |
|---|---|---|
| P1 | VM can load and verify class files with versions 49–51 | Classes compiled with `javac -source 1.7 -target 1.7` and without preverification can run |
| P2 | Base class library required for Java 5–7 language features | Generics, enums, foreach, boxing, varargs, assertions, string switch, try-with-resources, and multicatch are all usable |
| P3 | CLDC 8 class library | Collection framework, NIO subset, logging, permission model, GCF extensions |
| P4 | MEEP 8 application model | Implement a new application-management model on top of `jams` |

This document gives a detailed design for P1 and P2 and only a summary for P3 and P4, to be refined after P2 is complete.

### Non-goals

- `invokedynamic`, lambdas, and default methods (class-file version 52).
- Reflection (`java.lang.reflect`) and runtime annotation access.
- Oracle TCK certification. Before passing the TCK, externally describe this only as "an implementation of a subset of the CLDC 8 APIs."

## 2. Current State (Verified)

| Item | Current state | Location |
|---|---|---|
| Class-file version | Accepts 45–48 | `cldc/src/vm/share/utilities/GlobalDefinitions.hpp:775-776` |
| Constant pool | Recognizes only JVMS second-edition tags; all others report `unknown_constant_tag` | `ClassFileParser.cpp:178-259` |
| Stack maps | Parses only the CLDC `StackMap` attribute; unknown attributes are skipped | `ClassFileParser.cpp:1228-1257` |
| Internal stack-map format | Each mapping point is expanded into a complete frame: an `int` array (offset, locals, stack) plus a class-name array; long/double occupy two slots | `parse_stackmaps` in `ClassFileParser.cpp` |
| Verifier | Approximately 2,700 lines, consumes the internal format above | `cldc/src/vm/share/verifier/` |
| `ldc` verification | Reference-type constants are always treated as `java.lang.String` | `VerifyMethodCodes.cpp:394-402` |
| Class attributes | Handles only `SourceFile`, `InnerClasses`, and `Synthetic` | `ClassFileParser.cpp:1401-1434` |
| Access flags | Masked against a whitelist on read; unrecognized bits are discarded | `ClassFileParser.cpp:624, 948, 1558`; `JvmConst.hpp:98-120` |
| VM internal flags | Reuses positions introduced as Java 5 flags (see 4.5) | `AccessFlags.hpp:37-63` |
| Class-library selection | If `ENABLE_CLDC_111` is true, use `cldc1.1.1` | `cldc/build/share/root.make:105-114` |
| Class-library contents | 45 `java.lang` classes and 13 `java.util` classes; missing `StringBuilder`, `Enum`, `Iterable`, `Comparable`, `AutoCloseable`, `AssertionError`; `Boolean` has no `valueOf` | `cldc/src/javaapi/cldc1.1.1/` |
| Compilation parameters | `-source 1.4 -target 1.4`, in three places | `jvm.make:59`, `tests.make:44`, `test/rule.gmk:52` |
| Preverification | System classes are uniformly passed through `preverify` after compilation | `jvm.make:699-704` |
| `jcc` | Constant `JAVA_VERSION = 45`, recognizes only `StackMap` | `cldc/src/tools/jcc/util/ClassFileConst.java:69`, `components/MethodInfo.java:95` |
| ROM configuration | Initialization and renaming rules are hard-coded by class name | `cldc/src/vm/cldc_rom.cfg` |
| Tests | Test source referenced by `tests.make` is not in the repository; `test/` contains only two examples | `cldc/build/share/tests.make` |

## 3. Overall Strategy

1. **Add a configuration rather than replace it**. Add the build switch `ENABLE_CLDC_8` (off by default) and the class-library directory
   `cldc/src/javaapi/cldc8/`, and add it first in the selection chain at `root.make:105`. Preserve the existing CLDC 1.1.1 configuration as
   a buildable configuration for low-end MCUs and regression comparison.
2. **Avoid forking VM changes**. P1 VM changes (version number, `StackMapTable`, `ldc` class constants) have no side effects on old class files,
   so they are not guarded by `ENABLE_CLDC_8`. This makes both configurations share the same verification path and reduces combinations to test.
   Only parts that increase size (such as array `clone` and enum support native code) are switch-controlled.
3. **Reuse the existing verifier**. Convert `StackMapTable` to the existing internal format while parsing, so the verifier and GC stack-map compression
   (`StackmapGenerator`) require no changes. See 4.3.
4. **Add tests before changing code**. The verifier is the security boundary, and the first P1 task is the test framework (Section 8).

## 4. P1: VM Core

### 4.1 Version Number

- Change `JAVA_MAX_SUPPORTED_VERSION` from 48 to 51.
- `ClassFileParser.cpp:719` has a special case for 45.3. Read the context, then confirm behavior for new versions (not yet examined).
- The major version is already saved in the parsing state; all later version-dependent logic reads it from there.

### 4.2 Constant Pool

Three tags are new in version 51: `MethodHandle` (15), `MethodType` (16), and `InvokeDynamic` (18).

Design: **Explicitly unsupported and rejected** (confirmed). Java 7 source that does not use lambdas does not generate these three constants.
When one is encountered, throw `ClassFormatError` and add a dedicated error message ("invokedynamic is not supported"),
rather than the generic `unknown_constant_tag`, so the cause is clear to users.

The `invokedynamic` bytecode (0xBA) is also rejected during verification. Confirm whether 0xBA is already used as an internal
quick bytecode in `Bytecodes.hpp` (not checked).

### 4.3 StackMapTable

This is the core of P1.

**Format differences**

| | Existing CLDC `StackMap` | New `StackMapTable` |
|---|---|---|
| Offset | Absolute offset in each frame | `offset_delta`; first frame uses the value directly; subsequent frames use "previous frame offset + delta + 1" |
| Contents | Locals and stack are listed completely in each frame | Increments relative to the previous frame: `same`, `same_locals_1_stack_item`, `chop`, `append`, `full_frame`, and extended forms |
| Initial frame | None | Implicitly supplied by the method descriptor |
| Type items | Tags 0–8 | Tags 0–8 |

**Expand while parsing**

In `ClassFileParser`, add `parse_stackmap_table()` alongside `parse_stackmaps()` and emit the same
`ObjArray` (one pair of `stackmap_scalars` and `stackmap_classes` per mapping point):

1. Construct the initial frame from the method access flags and descriptor (local variable 0 is `this` for an instance method; in `<init>` it is an uninitialized `this`).
2. Maintain a "current frame," read each frame record in turn and update it in place, and write one complete frame to the output array for each record.
3. Expand long/double into two slots (`ITEM_Long` plus `ITEM_Long_2`), consistent with the existing convention.
4. Encode uninitialized objects (tag 8, with a `new` instruction offset) using the existing `ENCODE_NEWOBJECT` encoding.
5. Map uninitialized `this` (tag 6) to the existing `ITEM_InitObject`.

**Required checks during parsing** (each one requires a corresponding negative test):

- Attribute length matches the number of bytes actually consumed; a `Code` attribute cannot contain two stack-map attributes.
- Every offset is smaller than `code_length` and strictly increasing.
- `chop` cannot reduce the count below zero; `append` and `full_frame` cannot exceed `max_locals` or `max_stack` (including the second slot for long/double).
- Type tags are within 0–8; for an `Object` item, the constant-pool index is valid and points to a class.
- For tag 8, the offset points to a `new` instruction (the existing `check_news` already does this; confirm that the new path goes through it).

**Which attribute to select**

| Class version | Behavior |
|---|---|
| 45–49 | Preserve current behavior and require `StackMap` (i.e., preverification is required) |
| 50–51 | Use `StackMapTable`; if `StackMap` is also present, ignore the latter |

For version 50 and above, do not fall back to type inference: a method with branch targets but no mapping must fail verification.
`jsr` and `ret` are not permitted in the new version; existing handling of these two instructions in the verifier was not checked, so confirm that they are rejected.

**Memory impact**: The expanded internal format is identical to the existing format, so peak memory is expected to remain unchanged (not measured).

**Pending verification**: Determine whether the existing verifier's type rules are equivalent to the type-checking rules in JVMS 7, especially interface assignment
(`VerifierFrame.cpp:629` permits all interface targets), protected-member access, and `putfield` of an uninitialized
`this` in `<init>` (handled in `VerifyMethodCodes.cpp:774-815`). Differentially compare the test set in Section 8 with HotSpot verification results.

### 4.4 Loading Class Constants with `ldc`

Starting with version 49, `ldc` and `ldc_w` can reference `CONSTANT_Class`, producing a `java.lang.Class` object for that class.
For `Foo.class`, javac generates this instruction (older versions generated a call to `Class.forName`).

Required changes:

| Location | Change |
|---|---|
| Verifier `VerifyMethodCodes::push_obj` | Currently, all non-null reference constants are treated as `String`. The bytecode-traversal layer must pass the constant tag down so class constants push `java.lang.Class`. Also permit this only for class version ≥ 49. |
| Interpreter | Add a class-constant branch to the first execution path of `ldc`: resolve the class (which may trigger loading and throw `NoClassDefFoundError`) and push the class mirror (`JavaClassObj`). |
| Bytecode quickening | Near `Method.cpp:1380`, change `ldc` to `_fast_*ldc`. There are two plans for class constants:<br>A. Do not rewrite; always use the runtime slow path. Minimum implementation effort and slower.<br>B. Add a fast bytecode that resolves the class and obtains the mirror directly from the constant pool.<br>Recommend Plan A first for correctness, then Plan B if profiling shows a hotspot. |
| C interpreter | `fast_ldc` in `Interpreter_c.cpp` and its call sites. |
| Assembly interpreter | `ldc` templates in `TemplateTable_{arm,thumb,thumb2,i386}.cpp`. Under Plan A, only the slow path needs to call the runtime. |
| JIT | Constant loading in `BytecodeCompileClosure`. Under Plan A, encountering a class constant can abandon compilation of that method (fall back to the interpreter) and be optimized later. |
| ROMization | `BytecodeOptimizer` and `ConstantPoolRewriter` must recognize this use and ensure the class-constant entry remains correct after constant-pool reordering and merging. |

Note: `ldc` class constants do **not** trigger class initialization, only loading and linking.

### 4.5 Access Flags

The flags added in Java 5 overlap VM internal flags (`AccessFlags.hpp`):

| Standard flag | Value | VM internal flag at the same bit |
|---|---|---|
| `ACC_BRIDGE` (method) | 0x0040 | `JVM_ACC_HAS_MONITOR_BYTECODES` |
| `ACC_VARARGS` (method) | 0x0080 | `JVM_ACC_HAS_INVOKE_BYTECODES` |
| `ACC_SYNTHETIC` | 0x1000 | `JVM_ACC_HAS_NO_STACKMAPS` |
| `ACC_ANNOTATION` (class) | 0x2000 | `JVM_ACC_SYNTHETIC` (VM internal flag in bit 13) |
| `ACC_ENUM` | 0x4000 | `JVM_ACC_HAS_NO_EXCEPTION_TABLE` |

The values are currently masked against a whitelist during read, so these bits are discarded and cannot contaminate internal flags. Design:

- **Keep the whitelist unchanged**. With no reflection, `BRIDGE`, `VARARGS`, `ENUM`, and `ANNOTATION` have no runtime consumers.
- **The only flag requiring handling is `ACC_SYNTHETIC`**: starting with version 49, javac uses this flag instead of the `Synthetic` attribute to mark synthetic members.
  On read, if the original flags contain 0x1000, set the internal `JVM_ACC_SYNTHETIC`. First determine the internal synthetic flag's consumers
  (debugger and ROM optimizer); if it has no practical use, omit this mapping.
- Add an assertion or test to ensure that bits outside the whitelist can never enter `AccessFlags`, preventing future whitelist expansions from causing conflicts.

### 4.6 New Attributes

`Signature`, `EnclosingMethod`, `RuntimeVisibleAnnotations`, `RuntimeInvisibleAnnotations`,
`AnnotationDefault`, `LocalVariableTypeTable`, `SourceDebugExtension`, `BootstrapMethods`.

Design: **Skip all of them**. Existing code already handles unknown attributes this way, so no change is needed.
The only exception is `BootstrapMethods`: its presence indicates use of `invokedynamic`, so it can produce an error here in advance.

If P3 confirms that CLDC 8 requires runtime-visible annotations (pending verification), storage can be designed later.

### 4.7 Bridge Methods and Covariant Returns

javac generates bridge methods for generics and covariant returns: two methods with the same name and parameters but different return types.
If virtual-method lookup matches "name + full descriptor," they are unaffected. Verify both of the following:

- Virtual-method-table and interface-table construction compare the full descriptor, not only the parameter portion.
- ROM optimizer method renaming, inlining, and dead-method elimination do not confuse bridge methods with target methods.

This item is expected to require test coverage only, with no code changes.

### 4.8 Array `clone`

The `values()` method body generated by javac for every enum is `$VALUES.clone()`, which calls `clone` on the array type.
`Object.clone` is not in the CLDC 8 `Object` (confirmed), so **support only array `clone`; do not add any method to `Object`**.
This leaves the `Object` virtual-method table unchanged and does not affect vtable layout or existing ROM mirror structures.

**Bytecode shape**: javac generates `invokevirtual` for `$VALUES.clone()`. The method reference's owner is the array class
(for example, `[LColor;`), the name is `clone`, and the descriptor is `()Ljava/lang/Object;`, followed by `checkcast`.

**Design**

- Recognition conditions: the method reference owner is an array class and the name and descriptor match those above. All three conditions must hold.
- Method resolution: when these conditions hold, skip normal lookup (the method is not found in `Object`) and resolve directly to the VM's internal array-copy entry.
  After inspecting the method-resolution and quickening code, choose between two candidates:<br>
  A. During quickening, rewrite this `invokevirtual` to a new internal quick bytecode, and call the runtime copy implementation directly from the interpreter.<br>
  B. Resolve to an application-invisible internal native method and reuse the existing invocation path.<br>
  A uses no method table or class library but requires templates for all four CPU backends and JIT support; B requires less change. Favor B.
- Runtime behavior: throw `NullPointerException` when the receiver is `null`; otherwise allocate an array of the same type and length and shallow-copy the elements.
  Support primitive and reference arrays. Allocation may trigger GC, so protect the receiver with a handle during implementation.
- Verifier: for calls meeting the recognition conditions, the receiver must be an array type and the return type is treated as `Object`.
  For `clone` calls on all other types, apply normal rules; failure occurs because the method cannot be found.
- JIT: initially do not compile methods containing this call (fall back to the interpreter), or use a general runtime call; enum `values()` is not a hotspot.
- ROMization: `ROMOptimizer` and `ConstantPoolRewriter` must not report or remove this method reference as "unresolvable."
- Application code using `arr.clone()` generates the same bytecode, so array `clone` should work for applications.
  Document this explicitly; non-array objects cannot `clone`.

**Compilation-side verification**: Verify whether javac can compile enums and `arr.clone()` when `-bootclasspath` points to a class library
with neither `Object.clone` nor `java.lang.Cloneable`. My understanding is that javac treats array types as if they have a built-in
`clone` member and may synthesize an empty interface when `Cloneable` is absent, but this is not verified. This is a small M0 experiment;
if it is false, provide `Cloneable` (as a marker interface only) in the class library.

### 4.9 VM and Class-Library Layout Coupling

The VM accesses fields of several Java classes at fixed offsets, including `String` fields `value`, `offset`, and `count`
(`cldc/src/vm/share/handles/String.hpp:32-64`) and class-mirror fields (`JavaClassObj.hpp:35-46`);
`Throwable` and `Thread` are expected to behave similarly (not yet checked in detail).

Rule: when modifying these classes in the `cldc8` library, **preserve the order and types of fields already known to the VM, and append any new fields**.
Before P2 starts, compile a complete list of constrained classes (from the system-class template table in `Universe.hpp` and offset definitions in each `handles/*.hpp`),
put it in an explanatory file under the class-library directory, and add offset assertions to VM startup (debug builds).

## 5. P2: Class Library Required for Language Features

Create `cldc/src/javaapi/cldc8/`, initially copy `cldc1.1.1`, and then add the entries in the table below.

| Language feature | javac-generated dependency | Add or modify |
|---|---|---|
| String concatenation | `StringBuilder` | Add `StringBuilder`, `CharSequence`, `Appendable`; make `String` and `StringBuffer` implement `CharSequence` |
| Boxing | `valueOf(primitive type)` on wrapper classes and `xxxValue()` | `Boolean` lacks `valueOf`; verify signatures of the remaining wrapper classes individually; `Number` abstract class (whether it exists is unchecked) |
| foreach | `Iterable.iterator()`, `Iterator` | Add `Iterable` and `java.util.Iterator`; make `Vector` and similar classes implement `Iterable` |
| Enums | `Enum` base class, `Enum.valueOf(Class, String)`, `$VALUES.clone()`, `ordinal()` for enum switches | Add `Enum` and `Comparable`; depend on 4.8; see `valueOf` implementation below |
| Assertions | `Class.desiredAssertionStatus()`, `AssertionError` | Add `AssertionError`; make `desiredAssertionStatus` return `false` initially, with VM options supported later |
| String switch | `String.hashCode()`, `equals()` | Already present |
| try-with-resources | `AutoCloseable`, `Throwable.addSuppressed()` | Add `AutoCloseable` and `java.io.Closeable`; add a suppressed list to `Throwable` (append field, see 4.9); make stream and connection classes implement `Closeable` |
| Multicatch, diamond, varargs, generics | No runtime dependency | None |
| Annotations | Annotation types must exist at compile time | Add base types in `java.lang.annotation` and `Override`, `Deprecated`, `SuppressWarnings`, `SafeVarargs` |
| Class literals | `ldc` class constants | Depends on 4.4 |

**`Enum.valueOf` implementation**

OpenJDK uses reflection to call `values()`. This project has no reflection, so there are two plans:

- A. A native method reads the target class's static `$VALUES` field by name. This uses no additional RAM, but depends on javac's naming convention,
  and the ROM optimizer cannot rename or remove the field. Add a general enum rule in the `cldc_rom.cfg` mechanism.
- B. During `Enum` construction, register the instance in a table indexed by class. It does not depend on naming conventions, but every enum constant consumes additional RAM.

Plan A is recommended. Because other compilers, such as ecj, may generate different field names, document that only javac is supported.

**Genericize existing APIs**: Add type parameters to `Vector`, `Hashtable`, `Enumeration`, `Class`, and others.
Erasure preserves binary compatibility and does not affect existing applications.

**Origin and license**: Prefer porting and trimming classes from OpenJDK 7. Its license (GPLv2 plus the Classpath exception) is compatible with this project's GPLv2.
Preserve original copyright headers. Do not refer to or copy Oracle Java ME Embedded 8 closed-source implementations.

## 6. Toolchain and Build

| Item | Design |
|---|---|
| javac parameters | Under `ENABLE_CLDC_8`, change to `-source 1.7 -target 1.7` at three synchronized locations (`jvm.make:59`, `tests.make:44`, `test/rule.gmk:52`) and extract a shared variable |
| JDK for builds | Recommend JDK 8: it supports both 1.4 and 1.7 targets and can compile `java.lang` with `-bootclasspath`. Newer JDK build systems require module-related parameters and no longer support the 1.7 target (the exact version is pending verification) |
| preverify | Skip the preverification step in `jvm.make:699-704` under `ENABLE_CLDC_8`, allowing classes to enter ROMization directly with `StackMapTable`. Do not modify the preverify included in the repository (its behavior with new-version class files is unconfirmed) |
| romgen | romgen is built in generator mode as the VM and shares `ClassFileParser`, so it automatically gains P1 parsing capability. The relevant ROM-optimizer passes need verification (4.4, 4.7) |
| `jcc` | Version is hard-coded to 45 and only `StackMap` is recognized. First confirm whether the current build flow still uses it; if it does, update it as well; if not, leave it unchanged |
| ROM configuration | In `cldc_rom.cfg`, add an `If CLDC_8` section: `InitAtBuild` for new classes, preservation rules for enum `$VALUES`, and `RestrictedPackage` for new packages |
| debugger | `kdp` and `share/debugger` depend on line-number and local-variable tables, whose formats are unchanged. Add verification: set breakpoints and step through classes containing `StackMapTable` |
| application-side SDK | Remove preverification from application build flows in `sdk/` and `test/rule.gmk`, and update the documentation |

Add an `ENABLE_CLDC_8` variable to the top-level `build-*.sh` scripts and pass it to make. Repetition among these scripts is outside this design.

## 7. Size and Configuration

`Readme.txt` gives a baseline of 192 KB RAM and 700 KB ROM.

- P1 only adds parsing and verification code, with an expected very small ROM increase and no RAM change.
- P2 adds about twenty classes, with an expected noticeable ROM increase.
- P3 will significantly increase ROM and must be configurable to trim it.

These are estimates with no measurements. Requirement: **at the end of each phase**, measure and record ROM and static-RAM increments
in the `javacall_armv7m_gcc` (Thumb-2) configuration in this document. Group P3 classes by package, with one build switch per group.

**Measured numbers** (Linux i386 product `cldc_vm`, `size` output, 2026-11-18; not a Thumb-2 configuration,
so useful only as an order-of-magnitude reference):

| Configuration | text | data | bss |
|---|---|---|---|
| Default (CLDC 1.1.1 class library) | 1,404,926 | 1,418,580 | 61,252 |
| `ENABLE_CLDC_8=true` | 1,415,198 | 1,434,612 | 61,956 |
| Difference | +10,272 | +16,032 | +704 |

Both rows include P1 VM changes; the difference reflects only the class-library change. The increment from P1 VM changes relative to the pre-change state was not measured.

## 8. Tests

Current state: no usable VM test suite exists in the repository (`tests.make` references missing source).

### 8.1 Framework (First P1 Task)

- Create a top-level `test/vm/`, or place it at the path expected by `tests.make` (read `tests.make` to confirm the path first).
- Test platforms: Linux i386, covering both C interpreter (`arch = c`) and i386 assembly interpreter plus JIT configurations.
- Every test case is a class with `main` that prints a fixed pass marker; a script aggregates the results.
- On this basis, integrate CI (Linux i386 and Win32 i386 builds plus smoke tests).

### 8.2 Test Cases

| Category | Contents |
|---|---|
| Positive: language features | At least one test case per row of the Section 5 table; also generic bridge methods, covariant returns, synthetic methods for inner-class access, nested try-with-resources, and enum switch |
| Positive: `StackMapTable` | Cover every frame type, including long/double locals, uninitialized `this` in `<init>`, and uninitialized objects after `new` across branches |
| Negative: `StackMapTable` | One malformed class file for each check listed in 4.3; all must be rejected without crashing. Generate using byte arrays or assembly tools |
| Negative: versions and constant pool | 52 is rejected; a class with an `InvokeDynamic` constant or 0xBA bytecode is rejected with a clear message; a version-48 class using an `ldc` class constant is rejected |
| Array `clone` | Primitive arrays, reference arrays, multidimensional arrays, and arrays with length zero; results are independent copies; `null` receiver throws NPE; enum `values()` calls return different arrays. Negative: a `clone` method reference on a non-array type is rejected; a method reference on an array class with a mismatched name or descriptor is rejected |
| Regression | Old classes compiled with 1.4 target and preverified behave unchanged in the new VM; builds with `ENABLE_CLDC_8=false` run |
| Differential | Send the same batch of class files to HotSpot (`-Xverify:all`) and this VM and require identical verification conclusions |
| Fuzz | Mutate bytes in valid class files and feed them to the parser, run under an AddressSanitizer build, and allow no memory errors |
| ROM | Start after ROMizing the new class library as a whole; `Enum.valueOf` works with both ROMized system classes and dynamically loaded application classes |

Negative cases and fuzz testing are hard requirements: the `StackMapTable` parser handles untrusted input, and an error is a memory-safety issue.

## 9. P3 and P4 Summary

### P3: CLDC 8 Class Library

Add packages in batches, each independently configurable. Recommended order (by value to applications):

1. `java.util` collection framework: `Collection`, `List`, `Map`, `Set`, `ArrayList`, `HashMap`, `Arrays`, `Collections`, etc.
2. Completion of `java.lang`: Unicode support extensions for `Character`, new `String` methods, `ThreadLocal`, `StackTraceElement`, etc.
3. Completion of `java.io`: missing classes in the `Reader` and `Writer` families.
4. `java.nio` subset: buffer classes such as `ByteBuffer`. Direct buffers involve the VM and DIO and require separate design.
5. `java.security` permission model: `Permission`, `AccessController`, `Policy`. The relationship with existing `org.joshvm.security.internal` requires separate design.
6. `java.util.logging`.
7. GCF (`javax.microedition.io`) extensions.

Specific class and method lists must follow the JSR 360 specification; the list above is based on memory.

### P4: MEEP 8

The scope is the least certain and depends on specification verification. It is expected to involve the application model, inter-application communication, shared libraries,
event mechanisms, and integration with the existing `jams` and `ENABLE_ISOLATES` multi-tasking mechanism. Start design during P3.

## 10. Items Pending Verification

### Confirmed (Project Lead, 2026-11-18)

| # | Question | Conclusion | Impact on the design |
|---|---|---|---|
| 1 | Maximum class-file version | 51 | Consistent with 4.1 |
| 2 | Support for `invokedynamic` | Unsupported | Consistent with 4.2 |
| 3 | Direct acceptance of `StackMapTable` by the VM | Yes | Consistent with 4.3 |
| 4 | Whether `Object.clone` is in CLDC 8 `Object` | No | Change 4.8 to array `clone` only |

### Still Pending Verification

Confirm against the specification text before starting work.

| # | Question | Impact |
|---|---|---|
| 5 | Does CLDC 8 require runtime-visible annotations or any reflection capability | Determines whether 4.6 needs to retain annotations |
| 6 | Does CLDC 8 require type-inference verification for classes below version 50 that have no stack maps | Determines whether a verifier with inference must be implemented (large effort) |
| 7 | Complete P3 API list | P3 effort |
| 8 | Newer-JDK cutoff version for `-target 1.7` support | Build-environment documentation |
| 9 | TCK acquisition terms and license | External compatibility statements |

### Code-Side Conclusions (From Code Reading, Not Runtime-Verified)

| Question | Conclusion | Location |
|---|---|---|
| 45.3 special case | Only permits methods with "abstract + strictfp" in classes compiled by JDK 1.1 and is unrelated to new versions; no change needed | `ClassFileParser.cpp:716-722` |
| Is 0xBA already used | No; it is defined as `_xxxunusedxxx`; the bytecode traversal has no branch for it, so it falls through to `illegal_code` and is rejected. Only a clear error message is needed | `Bytecodes.hpp:228`, `Method.cpp:2153-2159` |
| `jsr`, `ret` | They also have no branch in bytecode traversal and are already rejected as illegal instructions; no change needed | `iterate_bytecode` in `Method.cpp` |
| Internal synthetic flag consumers | Only the `Field::is_synthetic()` accessor; no callers were found. Conclusion: the `ACC_SYNTHETIC` mapping in 4.5 is optional | `Field.hpp:62` |
| Is `jcc` used in the build flow | Yes: it is used to generate the native-function table from `classes.zip`. Its class-file version check is commented out, but it recognizes only `StackMap`; confirm whether it skips or reports an error for `StackMapTable` | `jvm.make:780-782`, `jcc/util/ClassFile.java:104-110` |
| Virtual-method-table override matching | Compares both name and full descriptor, so bridge methods are not confused with target methods | `InstanceClass.cpp:825` |
| Method-resolution path | `resolve_invoke_virtual_at` → `lookup_method_at` → `resolve_helper` extracts the owner, then calls `lookup_method`. Array-`clone` recognition for 4.8 should be placed in `lookup_method_at`, before the call to `lookup_method` | `ConstantPool.cpp:381-408, 531-566` |

**Existing hazard found during investigation**: `resolve_helper` directly places the method-reference owner in an `InstanceClass`
handle (`HotRoutines.incl.hpp:632-647`) without checking whether it is an array class; `lookup_method_at` then calls
`InstanceClass::lookup_method` on it. If a class file has an array class as the method-reference owner, the product build may
treat the array class as an instance class. I have not verified whether this can actually occur, nor whether the verifier blocks it earlier.
Implementation of 4.8 must address this together: when the owner is an array class, `clone` goes through a dedicated entry; every
other name is resolved through `Object` or throws `NoSuchMethodError`, and must never fall through to the existing path.

### Conclusions for Pending Code-Side Items

- `jcc` with `StackMapTable`: native tables generate normally (`ENABLE_CLDC_8` build passes).
- Entry form for 4.8: see the deviations table in "Implementation Status."
- javac can compile array `clone` and enums when the class library lacks `Object.clone` and `Cloneable`; enum `switch`
  requires `NoSuchFieldError` in the class library.
- The array-class method-reference hazard can occur: before the change, the debug VM asserted and failed on such a class; it is now fixed and covered by tests.
- Complete list of classes constrained by 4.9: not done.

### Build Environment

See the "Build and Tests" part of "Implementation Status."

## 11. Milestones and Dependencies

Workload is represented by relative size (S, M, L), with no person-day estimate.

| # | Task | Size | Dependencies |
|---|---|---|---|
| M0 | Resolve remaining Section 10 items; small javac enum compilation experiment (4.8); set up test framework and CI | M | — |
| M1 | Version number, constant-pool rejection path, access flags (4.1, 4.2, 4.5, 4.6) | S | M0 |
| M2 | `StackMapTable` parsing and positive/negative tests (4.3) | L | M1 |
| M3 | `ldc` class constants, Plan A (4.4) | M | M1 |
| M4 | Build switch, `cldc8` directory, javac 1.7, skip preverify (Sections 3 and 6) | M | M2 |
| M5 | Array `clone` (4.8); layout constraint checklist and assertions (4.9) | M | M4 |
| M6 | P2 class library (Section 5) | L | M3, M5 |
| M7 | ROM optimizer verification, bridge-method verification, debugger verification, size measurement | M | M6 |
| M8 | `ldc` class constants Plan B and JIT support (decide based on measurements) | M | M7 |

M2 and M3 can run in parallel. P1 and P2 are delivered at the end of M7, at which point applications written with Java 7 syntax can run on JOSHVM.

## 12. Main Risks

| Risk | Mitigation |
|---|---|
| Verifier rules differ subtly from JVMS 7, causing vulnerabilities or false rejections | Differential testing plus fuzz testing (8.2); separate M2 review |
| ROM optimizer mishandles new patterns (bridge methods, class constants, enum fields) and only exposes them after ROMization | Dedicated M7 verification; if necessary, disable related optimizations for the new class library |
| Size exceeds low-end MCU capacity | Preserve the CLDC 1.1.1 configuration; measure every phase; trim P3 by package |
| `ldc` template changes for four CPU backends cannot all be verified on real hardware | Plan A limits changes to the slow path; cover i386 in CI; ARM families require hardware or an emulator |
| Misunderstanding the specification | Verify Section 10 first in M0 |
