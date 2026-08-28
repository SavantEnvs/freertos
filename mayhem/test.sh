#!/usr/bin/env bash
#
# mayhem/test.sh — RUN the FreeRTOS-Plus-TCP upstream unit-test suite (built by mayhem/build.sh).
#
# This runs the project's ENTIRE upstream CTest suite (test/unit-test — the same ~60 Unity/CMock
# suites upstream CI runs) and reports a CTRF summary. It does NOT rebuild.
#
# Behavioral, not exit-status based: the pass/fail/skip counts come from Unity's own per-suite
# "<N> Tests <F> Failures <I> Ignored" report lines (parsed from CTest's LastTest.log), NOT from a
# process exit code. A sabotage patch that makes the binaries exit(0) produces no Unity report
# lines, so tests=0 -> this script fails (as it must).
set -uo pipefail
[ -n "${SOURCE_DATE_EPOCH:-}" ] || unset SOURCE_DATE_EPOCH
: "${MAYHEM_JOBS:=$(nproc)}"
cd "$SRC"

emit_ctrf() {
  local tool="$1" passed="$2" failed="$3" skipped="${4:-0}" pending="${5:-0}" other="${6:-0}"
  local tests=$(( passed + failed + skipped + pending + other ))
  cat > "${CTRF_REPORT:-$SRC/ctrf-report.json}" <<JSON
{
  "results": {
    "tool": { "name": "$tool" },
    "summary": {
      "tests": $tests,
      "passed": $passed,
      "failed": $failed,
      "pending": $pending,
      "skipped": $skipped,
      "other": $other
    }
  }
}
JSON
  printf 'CTRF {"results":{"tool":{"name":"%s"},"summary":{"tests":%d,"passed":%d,"failed":%d,"pending":%d,"skipped":%d,"other":%d}}}\n' \
    "$tool" "$tests" "$passed" "$failed" "$pending" "$skipped" "$other"
  [ "$failed" -eq 0 ] && [ "$tests" -gt 0 ]
}

BUILD_DIR="$SRC/mayhem/build-tests"
if [ ! -d "$BUILD_DIR" ]; then
  echo "test.sh: $BUILD_DIR missing — mayhem/build.sh did not build the unit tests" >&2
  emit_ctrf "ctest-unity" 0 1
  exit 1
fi

# The upstream catch_assert() harness relies on SIGABRT being unblocked inside its handler; on
# glibc 2.41 that needs SA_NODEFER, added by this preload shim. APPEND it to any existing
# LD_PRELOAD (e.g. the verify-repo sabotage shim) so we never clobber the grader's preload.
SHIM="$BUILD_DIR/abort_nodefer.so"
if [ -e "$SHIM" ]; then
  # ASan (gcc, dynamic) insists on being first in the preload list — put its runtime ahead of
  # the shim, then keep any caller-provided LD_PRELOAD after it.
  ASAN_RT="$(gcc -print-file-name=libasan.so 2>/dev/null || true)"
  PRELOAD="$SHIM"
  [ -e "$ASAN_RT" ] && PRELOAD="$ASAN_RT:$PRELOAD"
  export LD_PRELOAD="$PRELOAD${LD_PRELOAD:+:$LD_PRELOAD}"
fi
export ASAN_OPTIONS="${ASAN_OPTIONS:+$ASAN_OPTIONS:}detect_odr_violation=0"

# Run the whole suite (excluding the "system" tests, exactly as upstream CI: `ctest -E system`).
ctest --test-dir "$BUILD_DIR" -E system -j"$MAYHEM_JOBS" --output-on-failure >/dev/null 2>&1 || true

LASTLOG="$BUILD_DIR/Testing/Temporary/LastTest.log"
if [ ! -f "$LASTLOG" ]; then
  echo "test.sh: no CTest log produced" >&2
  emit_ctrf "ctest-unity" 0 1
  exit 1
fi

# Sum Unity's per-suite report lines: "<N> Tests <F> Failures <I> Ignored".
read -r TOTAL FAIL IGN < <(awk '
  /[0-9]+ Tests [0-9]+ Failures [0-9]+ Ignored/ {
    for (i = 1; i <= NF; i++) {
      if ($(i+1) == "Tests")    t += $i;
      if ($(i+1) == "Failures") f += $i;
      if ($(i+1) == "Ignored")  g += $i;
    }
  }
  END { printf "%d %d %d\n", t, f, g }
' "$LASTLOG")

TOTAL=${TOTAL:-0}; FAIL=${FAIL:-0}; IGN=${IGN:-0}
PASS=$(( TOTAL - FAIL - IGN ))
[ "$PASS" -lt 0 ] && PASS=0

echo "test.sh: unit tests — total=$TOTAL passed=$PASS failed=$FAIL ignored=$IGN" >&2
emit_ctrf "ctest-unity" "$PASS" "$FAIL" "$IGN"
