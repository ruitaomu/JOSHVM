# Build and test environment for the Linux i386 target.
#
# JDK 8 is pinned on purpose: the makefiles compile the class library with
# -source/-target 1.4 (1.7 with ENABLE_CLDC_8) and -bootclasspath, which
# newer JDKs no longer accept.
#
# The image was verified locally with Docker 29.1.3. The script uses
# qemu-user for the 32-bit host tools and test executables.
#
# Usage, from the repository root:
#   docker build -t joshvm-build-linux-i386 tools/docker/linux-i386
#   docker run --rm -v "$PWD":/src joshvm-build-linux-i386
FROM eclipse-temurin:8-jdk-jammy

RUN apt-get update \
 && apt-get install -y --no-install-recommends \
      make gcc-multilib g++-multilib qemu-user \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /src
CMD ["tools/docker/linux-i386/build-and-test.sh"]
