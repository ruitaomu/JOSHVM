#!/bin/sh
#
# Builds the Linux i386 VM with each class library (CLDC 1.1.1, then the
# Java 7 level library selected by ENABLE_CLDC_8) and runs
# test/vm/run-tests.sh against both.
#
# Environment:
#   JDK_DIR       JDK 8; defaults to JAVA_HOME
#   HOST_EXE_RUN  prefix for running 32-bit executables on hosts that
#                 cannot (for example qemu-i386); empty by default

set -e
cd `dirname $0`/../../..

: ${JDK_DIR:=$JAVA_HOME}
export JDK_DIR

# The preverify binaries checked in under cldc/build/share/bin predate
# large inode numbers and silently produce nothing on current file
# systems, so build it from source.
make -C preverifier/build/linux \
    EXTRACFLAGS="-D_FILE_OFFSET_BITS=64 -fcommon -std=gnu89 -w"
PREVERIFY_ORIGINAL=`pwd`/preverifier/build/linux/preverify
export PREVERIFY_ORIGINAL

for cldc8 in false true; do
    echo "=== ENABLE_CLDC_8=$cldc8"
    ENABLE_CLDC_8=$cldc8 ./build-linux-i386-cldc.sh -C -J "$JDK_DIR" \
        > build-linux-i386.log 2>&1 || {
            tail -40 build-linux-i386.log
            exit 1
        }
    test/vm/run-tests.sh
done
