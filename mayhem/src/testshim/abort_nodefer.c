/*
 * LD_PRELOAD shim for the upstream FreeRTOS-Plus-TCP unit tests.
 *
 * The tests trap intentional configASSERT()/assert() aborts with the
 * catch_assert() macro (test/unit-test/catch_assert.h): it installs a SIGABRT
 * handler that longjmp()s back to the test body. That relies on the signal
 * mask being restored after the longjmp so a *later* assert can be caught too.
 *
 * On glibc 2.41 (the base image) plain setjmp/longjmp no longer reliably
 * unblocks the automatically-masked SIGABRT, so the second intentional assert
 * in a suite re-raises SIGABRT while it is blocked and the process really
 * aborts (SIGABRT default action) even though the test logic is correct.
 *
 * This shim intercepts the test harness's own sigaction(SIGABRT, ...) call and
 * adds SA_NODEFER, so SIGABRT is never auto-blocked inside catch_assert's
 * handler. It changes nothing about the tests' assertions or outcomes; it only
 * restores the mask behavior the upstream harness was written against. It is
 * applied via LD_PRELOAD only when running the suite (mayhem/test.sh) and does
 * not touch the fuzz targets.
 */
#define _GNU_SOURCE
#include <signal.h>
#include <dlfcn.h>

int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact)
{
    static int (*real_sigaction)(int, const struct sigaction *, struct sigaction *);

    if (real_sigaction == NULL) {
        real_sigaction = dlsym(RTLD_NEXT, "sigaction");
    }

    struct sigaction patched;
    if (signum == SIGABRT && act != NULL) {
        patched = *act;
        patched.sa_flags |= SA_NODEFER;
        act = &patched;
    }

    return real_sigaction(signum, act, oldact);
}
