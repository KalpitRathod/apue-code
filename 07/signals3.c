/* ============================================================================
 * Signal Masking Tactics
 * ============================================================================
 * In modern POSIX, a program can say 'Do not disturb me with SIGALRM right now, I am updating a database file.' This introduced the structural signal mask, preventing database corruption by buffering interrupts until it was safe to acknowledge them.
 * ============================================================================
 */
/* This program illustrates blocking a signal and the
 * use of signal masks.
 *
 * SIG_QUIT is blocked, so if we hit ^\, nothing
 * happens.  We can check whether any such signals
 * were delivered via sigpending(2). Once we unblock
 * the signal, we will immediately enter the signal
 * handler.
 *
 * Note that if we change the signal handler to
 * SIG_IGN _after_ the signal has been generated, but
 * _before_ we unblock, it will still be ignored.
 */

#include <err.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#ifndef SLEEP
#define SLEEP 5
#endif

int s = 0;

static void
sig_quit(int signo) {
	(void)signo;
	(void)printf("In sig_quit, s=%d. Now sleeping...\n", ++s);

	/* This call to sleep(3) can itself be
	 * interrupted if we receive a signal other
	 * than SIGQUIT while executing this signal
	 * handler.  If that happens, then we jump
	 * into the other signal handler; when that
	 * handler completes, we are returned back
	 * here. */
	(void)sleep(SLEEP);

	(void)printf("sig_quit, s=%d: exiting\n", s);
}

static void
sig_int(int signo) {
	(void)signo;
	(void)printf("Now in sig_int, s=%d. Returning immediately.\n", ++s);
}

int
main(int argc, char **argv) {
	sigset_t newmask, oldmask, pendmask;
	int ismember = 0;

	(void)argv;
	(void)printf("\n=> Establishing initial signal hander via signal(3).\n");
	if (signal(SIGQUIT, sig_quit) == SIG_ERR) {
		err(EXIT_FAILURE, "unable to set SIGQUIT signal handler");
		/* NOTREACHED */
	}

	if (signal(SIGINT, sig_int) == SIG_ERR) {
		err(EXIT_FAILURE, "unable to set SIGINT signal handler");
		/* NOTREACHED */
	}

	if (sigemptyset(&newmask) < 0) {
		err(EXIT_FAILURE, "sigemtpyset");
		/* NOTREACHED */
	}
	if (sigaddset(&newmask, SIGQUIT) < 0) {
		err(EXIT_FAILURE, "sigaddset");
		/* NOTREACHED */
	}

	(void)printf("\n=> Blocking delivery of SIGQUIT...\n");
	if (sigprocmask(SIG_BLOCK, &newmask, &oldmask) < 0) {
		err(EXIT_FAILURE, "unable to block SIGQUIT");
		/* NOTREACHED */
	}

	(void)printf("\n=> Now going to sleep for %d seconds...\n", SLEEP);
	(void)sleep(SLEEP);

	if (argc > 1) {
		if (signal(SIGQUIT, SIG_IGN) == SIG_ERR) {
			err(EXIT_FAILURE, "unable to ignore SIGQUIT");
			/* NOTREACHED */
		}
	}

	printf("\n=> Checking if any signals are pending...\n");
	if (sigpending(&pendmask) < 0) {
		err(EXIT_FAILURE, "sigpending");
		/* NOTREACHED */
	}

	if ((ismember = sigismember(&pendmask, SIGQUIT)) < 0) {
		err(EXIT_FAILURE, "sigismember");
		/* NOTREACHED */
	}
	
	if (ismember) {
		(void)printf("Pending SIGQUIT found.\n");
	}

	(void)printf("\n=> Unblocking SIGQUIT...\n");
	if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0) {
		err(EXIT_FAILURE, "unable to unblock SIGQUIT");
		/* NOTREACHED */
	}

	/* Note that if we did receive ^\ while we
	 * were blocked, we are jumping into sig_quit
	 * right here, _before_ we print this. */
	(void)printf("SIGQUIT unblocked - sleeping some more...\n");

	/* ^\ will now be delivered again */
	(void)sleep(SLEEP);

	(void)printf("Now exiting.\n");
	exit(EXIT_SUCCESS);
}

/* ============================================================================
 * DOCUMENTATION
 * ============================================================================
 *
 * INTENT:
 *   Demonstrates POSIX signal masking — blocking specific signals so they are
 *   held pending until explicitly unblocked. When SIGQUIT is blocked, pressing
 *   ^\ queues the signal instead of delivering it immediately. After unblocking,
 *   any pending SIGQUIT is delivered instantly. Also shows how to check for
 *   pending signals and optionally ignore a signal even after it's pending.
 *
 * MACROS:
 *   SLEEP          - Sleep duration (default 5 seconds). Override with -DSLEEP=N.
 *   SIGQUIT        - Signal 3 (Ctrl+\).
 *   SIGINT         - Signal 2 (Ctrl+C).
 *   SIG_BLOCK      - sigprocmask() command: ADD signals in newmask to blocked set.
 *   SIG_SETMASK    - sigprocmask() command: REPLACE blocked set with newmask.
 *   SIG_IGN        - Signal disposition: ignore the signal completely.
 *   SIG_ERR        - Returned by signal() on error.
 *   EXIT_FAILURE  - 1; exit on error.
 *   EXIT_SUCCESS  - 0; normal exit.
 *
 * VARIABLES:
 *   int s              - Global counter; incremented per handler entry.
 *   sigset_t newmask   - Signal set to add to the blocked mask (contains SIGQUIT).
 *   sigset_t oldmask   - Saved previous signal mask. Restored after the demo.
 *   sigset_t pendmask  - Receives set of pending signals from sigpending().
 *   int ismember       - Boolean: 1 if SIGQUIT is in pendmask, 0 otherwise.
 *
 * FUNCTIONS:
 *   sig_quit(signo)    - SIGQUIT handler; increments s, sleeps, returns.
 *   sig_int(signo)     - SIGINT handler; increments s, returns immediately.
 *   main(int, char**)  - Installs handlers, blocks SIGQUIT, sleeps, checks
 *                        pending signals, optionally ignores, then unblocks.
 *   signal(sig, fn)    - Installs signal handler.
 *   sigemptyset(&set)  - Initializes set to empty (no signals).
 *   sigaddset(&set, sig) - Adds 'sig' to the set.
 *   sigprocmask(how, &new, &old) - Modifies process signal mask:
 *                        SIG_BLOCK: blocked = blocked | new.
 *                        SIG_SETMASK: blocked = new.
 *                        'old' receives the previous mask (for restoration).
 *   sigpending(&set)   - Fills 'set' with currently pending/blocked signals.
 *   sigismember(&set, sig) - Returns 1 if sig is in set, 0 if not.
 *   err(status, msg)   - Error exit.
 *   sleep(n)           - Waits N seconds (or until a signal interrupts).
 *   exit(status)       - Terminates.
 *
 * ALGORITHM:
 *   1. Install sig_quit for SIGQUIT, sig_int for SIGINT via signal(3).
 *   2. Build newmask = {SIGQUIT} via sigemptyset + sigaddset.
 *   3. sigprocmask(SIG_BLOCK, &newmask, &oldmask) — block SIGQUIT.
 *      Any ^\ during next step goes PENDING, not delivered.
 *   4. sleep(SLEEP) — user can press ^\ without it being handled yet.
 *   5. Optional: if argc>1, change SIGQUIT handler to SIG_IGN (even pending
 *      signals, if ignored before unblocking, will not be delivered).
 *   6. sigpending(&pendmask) → check if SIGQUIT is pending.
 *   7. sigprocmask(SIG_SETMASK, &oldmask, NULL) — UNBLOCK SIGQUIT.
 *      Pending SIGQUIT is delivered HERE, before next line executes.
 *   8. sleep(SLEEP) — back to normal; ^\ delivers to handler.
 *   9. exit(EXIT_SUCCESS).
 *
 * KEY SYSCALLS / LIBRARY FUNCTIONS:
 *   sigprocmask(2)  - Gets/sets the process signal mask. Thread-safe.
 *   sigpending(2)   - Queries which blocked signals are currently pending.
 *   sigemptyset(3)  - Clears all signals from a sigset_t.
 *   sigaddset(3)    - Adds one signal to a sigset_t.
 *   sigismember(3)  - Tests if a signal is in a sigset_t.
 *
 * ============================================================================
 */
