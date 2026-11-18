#!/bin/sh
#
# VM tests for the Linux i386 and Win32 i386 builds.
#
# Usage, from the repository root, after build-linux-i386-cldc.sh:
#   JDK_DIR=/path/to/jdk8 test/vm/run-tests.sh
#
# Environment:
#   JDK_DIR       JDK 8 (required: needs -source/-target 1.4 and 1.7)
#   HOST_EXE_RUN  prefix for running 32-bit executables, e.g. qemu-i386
#   VM_DIR        directory with the VM executables
#   VMS           VM executables to test (default: cldc_vm_g cldc_vm)
#   BOOT_CLASSES  class library to compile against
#   PREVERIFY     preverify executable for the legacy test
#   FUZZ_COUNT    number of randomly damaged classes to try (default 40)
#
# Each positive test is run with the compiler enabled and with -int, and
# must print PASS as its last line. Each negative test is a class patched
# by tools/ClassPatcher; the VM must reject it with the expected message.

ROOT=`cd \`dirname $0\`/../.. && pwd`
case `uname -s 2>/dev/null` in
  CYGWIN*|MINGW*|MSYS*)
    # javac, java and the VM are native Windows programs: give them
    # C:/... paths, which the shell tools accept as well.
    ROOT=`cygpath -m "$ROOT"`
    : ${VM_DIR:=$ROOT/cldc/build/javacall_i386_vc/dist/bin}
    : ${VMS:="cldc_vm_g.exe cldc_vm.exe"}
    : ${PREVERIFY:=$ROOT/cldc/build/javacall_i386_vc/dist/bin/preverify.exe}
    ;;
  *)
    : ${VM_DIR:=$ROOT/cldc/build/javacall_i386_gcc/dist/bin}
    : ${VMS:="cldc_vm_g cldc_vm"}
    : ${PREVERIFY:=$ROOT/preverifier/build/linux/preverify}
    ;;
esac
HERE=$ROOT/test/vm
OUT=$HERE/build
: ${BOOT_CLASSES:=$ROOT/cldc/build/classes.zip}

# Whether exit status $1 of the VM means it crashed. The VM exits with 0,
# or with 255 after an uncaught exception. A crash is death by a signal
# (128 + signal number) on Linux; the MSYS2 shell reports a Windows
# exception code it does not know as 127.
crashed() {
    [ $1 -ge 127 -a $1 -lt 255 ]
}

if [ -z "$JDK_DIR" ]; then
    echo "JDK_DIR must point to a JDK 8" >&2
    exit 2
fi
JAVAC="$JDK_DIR/bin/javac -nowarn -Xlint:-options"
JAVA=$JDK_DIR/bin/java

passed=0
failed=0

report() { # result name detail
    if [ "$1" = ok ]; then
        passed=`expr $passed + 1`
        echo "ok      $2"
    else
        failed=`expr $failed + 1`
        echo "FAILED  $2"
        [ -n "$3" ] && echo "$3" | sed 's/^/        /'
    fi
}

run_vm() { # vm mode classpath class
    # stdin from /dev/null: the VM must not wait on the terminal
    $HOST_EXE_RUN $VM_DIR/$1 $2 -cp $3 $4 2>&1 < /dev/null
}

expect_pass() { # label classpath class
    for vm in $VMS; do
        for mode in "" "-int"; do
            output=`run_vm $vm "$mode" $2 $3`
            if [ "`echo "$output" | tail -1 | tr -d '\r'`" = PASS ]; then
                report ok "$1 [$vm $mode]"
            else
                report fail "$1 [$vm $mode]" "$output"
            fi
        done
    done
}

# The VM must refuse the class: no PASS, the expected message, and a normal
# exit rather than death by a signal (the VM exits with 255 on an uncaught
# exception; 129 and up, short of that, is 128 + signal number).
expect_reject() { # label classpath class message
    for vm in $VMS; do
        output=`run_vm $vm "" $2 $3`
        status=$?
        if echo "$output" | grep -q "^PASS"; then
            report fail "$1 [$vm]" "not rejected: $output"
        elif crashed $status; then
            report fail "$1 [$vm]" "VM crashed (status $status): $output"
        elif echo "$output" | grep -q -- "$4"; then
            report ok "$1 [$vm]"
        elif echo "$output" | grep -q 'Error: *[0-9][0-9]*$'; then
            # Product builds print an error number instead of the text
            report ok "$1 [$vm] (message number only)"
        else
            report fail "$1 [$vm]" "expected '$4', got: $output"
        fi
    done
}

rm -rf $OUT
mkdir -p $OUT/pass $OUT/neg $OUT/legacy-gen $OUT/legacy $OUT/tools $OUT/cldc8

# The tests in src/cldc8 need the ENABLE_CLDC_8 class library.
if $JDK_DIR/bin/jar tf $BOOT_CLASSES | grep -q '^java/lang/Enum.class$'; then
    HAVE_CLDC8=true
else
    HAVE_CLDC8=false
fi

echo "--- compiling"
if [ $HAVE_CLDC8 = true ]; then
    $JAVAC -bootclasspath $BOOT_CLASSES -source 1.7 -target 1.7 \
        -d $OUT/cldc8 $HERE/src/cldc8/*.java || exit 2
fi
$JAVAC -bootclasspath $BOOT_CLASSES -source 1.7 -target 1.7 \
    -d $OUT/pass $HERE/src/pass/*.java || exit 2
$JAVAC -bootclasspath $BOOT_CLASSES -source 1.7 -target 1.7 \
    -d $OUT/neg $HERE/src/neg/*.java || exit 2
$JAVAC -bootclasspath $BOOT_CLASSES -source 1.4 -target 1.4 \
    -d $OUT/legacy-gen $HERE/src/legacy/*.java || exit 2
$HOST_EXE_RUN $PREVERIFY -classpath $BOOT_CLASSES \
    -d $OUT/legacy $OUT/legacy-gen || exit 2
$JAVAC -d $OUT/tools $HERE/tools/ClassPatcher.java || exit 2

echo "--- positive tests"
for src in $HERE/src/pass/*.java; do
    name=`basename $src .java`
    expect_pass $name $OUT/pass $name
done
expect_pass LegacyPreverified $OUT/legacy LegacyPreverified
if [ $HAVE_CLDC8 = true ]; then
    for src in $HERE/src/cldc8/*.java; do
        name=`basename $src .java`
        expect_pass $name $OUT/cldc8 $name
    done
else
    echo "skipped src/cldc8: class library built without ENABLE_CLDC_8"
fi
for src in $HERE/src/neg/*.java; do
    name=`basename $src .java`
    expect_pass "$name (unpatched)" $OUT/neg $name
done

echo "--- negative tests"
neg() { # label class message patch-operations...
    label=$1; class=$2; message=$3
    shift 3
    dir=$OUT/neg-$label
    mkdir -p $dir
    if $JAVA -cp $OUT/tools ClassPatcher $OUT/neg/$class.class \
            $dir/$class.class "$@"; then
        expect_reject $label $dir $class "$message"
    else
        report fail $label "ClassPatcher failed"
    fi
}

# Class file version and unsupported constants
neg version-52        NegBase "Invalid version" version 52
neg indy-constant     NegBase "invokedynamic is not supported" cptag 10 18
neg methodhandle-constant NegBase "invokedynamic is not supported" cptag 8 16
neg indy-opcode       NegBase "invokedynamic is not supported" \
    code iface 'b9??????00' ba

# Malformed StackMapTable for NegBase.branch (max_locals 1, max_stack 1,
# code length 8, valid table is 0001 06)
neg smt-reserved-frame-type NegBase "unknown frame type" smt branch 0001c8
neg smt-offset-out-of-range NegBase "offset out of range" smt branch 00013f
neg smt-chop-underflow NegBase "unexpected size" smt branch 0001f90006
neg smt-append-overflow NegBase "unexpected size" smt branch 0001fd00060101
neg smt-long-overflow NegBase "unexpected size" \
    smt branch 0001ff00060001040000
neg smt-stack-overflow NegBase "unexpected size" \
    smt branch 0001ff000600010100020101
neg smt-bad-type-tag  NegBase "unexpected stack type" \
    smt branch 0001ff00060001090000
neg smt-bad-class-index NegBase "Bad constant pool index" \
    smt branch 0001ff0006000107ffff0000
neg smt-bad-new-offset NegBase "unexpected stack type" \
    smt branch 0001ff0006000108ffff0000
neg smt-trailing-bytes NegBase "Bad Stackmap attribute size" \
    smt branch 00010600
neg smt-count-too-large NegBase "ClassFormatError" smt branch 000206
# Well-formed tables that do not describe the code
MISMATCH="Inconsistent or missing stackmap at target"
neg smt-wrong-local-type NegBase "$MISMATCH" \
    smt branch 0001ff00060001020000
neg smt-wrong-offset  NegBase "$MISMATCH" smt branch 000105
neg smt-missing       NegBase "$MISMATCH" smt branch -
neg smt-empty         NegBase "$MISMATCH" smt branch 0000

# ldc of a class constant needs class file version 49
neg ldc-class-version-48 NegClassLit "requires class file version 49" \
    version 48

# Method references owned by an array class: only clone() is special
neg clone-non-array   NegClone "NoSuchMethodError" \
    utf8 '[I' java/lang/String \
    utf8 '([I)Ljava/lang/Object;' '(Ljava/lang/String;)Ljava/lang/Object;'
neg array-unknown-method NegClone "NoSuchMethodError" utf8 clone nosuch
neg array-clone-bad-descriptor NegClone "NoSuchMethodError" \
    utf8 '()Ljava/lang/Object;' '()Ljava/lang/String;'

# Any other method referenced through an array class resolves in Object
mkdir -p $OUT/array-tostring
$JAVA -cp $OUT/tools ClassPatcher $OUT/neg/NegClone.class \
    $OUT/array-tostring/NegClone.class utf8 clone toString \
    utf8 '()Ljava/lang/Object;' '()Ljava/lang/String;' || exit 2
expect_pass array-inherited-method $OUT/array-tostring NegClone

# Randomly damaged StackMapTables. Whatever the VM makes of them, it must
# not die from a signal: it either rejects the class or, when the damage
# happens to be harmless, runs it.
: ${FUZZ_COUNT:=40}
echo "--- fuzzing StackMapTable ($FUZZ_COUNT variants)"
fuzz_vm=`echo $VMS | cut -d' ' -f1`
fuzz_crashes=0
seed=1
while [ $seed -le $FUZZ_COUNT ]; do
    dir=$OUT/fuzz
    rm -rf $dir
    mkdir -p $dir
    $JAVA -cp $OUT/tools ClassPatcher $OUT/pass/StackMapFrames.class \
        $dir/StackMapFrames.class fuzzsmt $seed || exit 2
    output=`run_vm $fuzz_vm "" $dir StackMapFrames`
    status=$?
    if crashed $status; then
        fuzz_crashes=`expr $fuzz_crashes + 1`
        echo "        seed $seed: VM crashed (status $status): $output"
    fi
    seed=`expr $seed + 1`
done
if [ $fuzz_crashes -eq 0 ]; then
    report ok "fuzz StackMapTable [$fuzz_vm]"
else
    report fail "fuzz StackMapTable [$fuzz_vm]" "$fuzz_crashes crashes"
fi

echo "--- $passed passed, $failed failed"
[ $failed -eq 0 ]
