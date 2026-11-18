# Build and test environment for the Linux i386 target.
#
# JDK 8 is pinned on purpose: the makefiles compile the class library with
# -source/-target 1.4 (1.7 with ENABLE_CLDC_8) and -bootclasspath, which
# newer JDKs no longer accept.
#
# This image has not been built or run yet. The same steps were verified
# on Ubuntu 24.04 with gcc 13 and OpenJDK 8, running the 32-bit binaries
# under qemu-i386 (HOST_EXE_RUN=qemu-i386).
#
# Usage, from the repository root:
#   docker build -t joshvm-build-linux-i386 tools/docker/linux-i386
#   docker run --rm -v "$PWD":/src joshvm-build-linux-i386
FROM eclipse-temurin:8-jdk-jammy

RUN apt-get update \
 && apt-get install -y --no-install-recommends \
      make gcc-multilib g++-multilib \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /src
CMD ["tools/docker/linux-i386/build-and-test.sh"]
