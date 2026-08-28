#!/usr/bin/env bash
#
# mayhem/build.sh — build the FreeRTOS-Plus-TCP Mayhem harnesses + the upstream unit-test suite.
#
# Runs inside the commit image (mayhem/Dockerfile) as `mayhem` in /mayhem. The integration is
# ADDITIVE: upstream FreeRTOS-Plus-TCP under the repo root is built in place and never modified;
# everything this script needs lives under mayhem/.
#
# Two build products:
#   1) the fuzz harnesses (clang + ASan/UBSan + libFuzzer, coverage-instrumented) written to /mayhem
#   2) the project's OWN unit-test suite (test/unit-test, gcc, ASan/UBSan — matching upstream CI),
#      so mayhem/test.sh only has to RUN it.
set -euo pipefail

# clang rejects SOURCE_DATE_EPOCH='' (empty) — it must be unset or a valid integer.
[ -n "${SOURCE_DATE_EPOCH:-}" ] || unset SOURCE_DATE_EPOCH

: "${SANITIZER_FLAGS=-fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer}"
: "${DEBUG_FLAGS:=-g -gdwarf-3}"
: "${CC:=clang}" ; : "${CXX:=clang++}" ; : "${LIB_FUZZING_ENGINE:=-fsanitize=fuzzer}"
: "${STANDALONE_FUZZ_MAIN:=/opt/mayhem/StandaloneFuzzTargetMain.c}"
: "${MAYHEM_JOBS:=$(nproc)}"
export SANITIZER_FLAGS DEBUG_FLAGS CC CXX LIB_FUZZING_ENGINE STANDALONE_FUZZ_MAIN MAYHEM_JOBS

cd "$SRC"

# ---------------------------------------------------------------------------
# 1) Fuzz harnesses (the Mayhem targets) + standalone reproducers.
#
#    The whole project (kernel + FreeRTOS-Plus-TCP + adapter + harness) is compiled with
#    $SANITIZER_FLAGS $DEBUG_FLAGS and coverage instrumentation (-fsanitize=fuzzer-no-link) so
#    ASan/UBSan see the library (not just the harness) and libFuzzer/Mayhem get edge coverage.
#    mayhem/src/CMakeLists.txt links the libFuzzer runtime into each <name> target and the
#    run-once $STANDALONE_FUZZ_MAIN into each <name>-standalone reproducer. Binaries land in /mayhem.
# ---------------------------------------------------------------------------
HARNESS_CFLAGS="$SANITIZER_FLAGS $DEBUG_FLAGS -fsanitize=fuzzer-no-link"
cmake -G Ninja -S mayhem/src -B mayhem/build-harness \
    -DCMAKE_C_COMPILER="$CC" \
    -DCMAKE_C_FLAGS="$HARNESS_CFLAGS"
cmake --build mayhem/build-harness -j"$MAYHEM_JOBS"

# ---------------------------------------------------------------------------
# 2) Upstream unit-test suite (test/unit-test), built exactly as upstream CI does:
#    gcc + -DSANITIZE=address,undefined. clang can't link this tree (it hard-links libgcov for the
#    coverage build variant), so gcc is required. mayhem/test.sh RUNS this; it never rebuilds.
#
#    The suite needs CMock + Unity (tools/CMock) and the FreeRTOS-Kernel (test/FreeRTOS-Kernel)
#    submodules plus ruby/unifdef/ninja (installed in the Dockerfile). Populate the submodules from
#    the pinned gitlinks; on an offline rebuild they are already present so no network is used.
# ---------------------------------------------------------------------------
if [ -d .git ]; then
    git submodule update --init --checkout --recursive tools/CMock test/FreeRTOS-Kernel 2>/dev/null || true
fi
if [ ! -e tools/CMock/lib/cmock.rb ] || [ ! -e test/FreeRTOS-Kernel/include ]; then
    echo "build.sh: unit-test submodules (tools/CMock, test/FreeRTOS-Kernel) are missing" >&2
    exit 1
fi

cmake --fresh -G Ninja -S test/unit-test -B mayhem/build-tests \
    -DBUILD_CLONE_SUBMODULES=OFF \
    -DCMAKE_C_COMPILER=gcc \
    -DSANITIZE=address,undefined
ninja -C mayhem/build-tests

# The catch_assert() SIGABRT shim the suite needs on glibc 2.41 (see testshim/abort_nodefer.c).
gcc -shared -fPIC -O2 -o mayhem/build-tests/abort_nodefer.so mayhem/src/testshim/abort_nodefer.c -ldl

echo "build.sh: done — harnesses in /mayhem, unit tests in mayhem/build-tests"
