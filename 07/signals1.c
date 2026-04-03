/* ============================================================================
 * The Original Interrupt: SIGINT
 * ============================================================================
 * In 1973, pressing ^C or DEL on a teletype machine sent a physical hardware interrupt. The UNIX driver translated this into a SIGINT software signal. This script tests the raw catching and processing of what used to be a teletype panic button.
 * ============================================================================
 */
/* This program illustrates that you can be
 * interrupted while executing a signal handler.
 *
 * It also shows that the signal for the currently
 * executing handler is blocked until we return from
 * that call, but that pending signals are merged and
 * then delivered afterwards.
 *
 * To demonstrate:
 * Hit ^\ to jump into sig_quit.

 * Hit ^\ again and note that we're not interrupting
 * the sleep inside the sig_quit handler.  However,
 * the signal gets delivered once sig_quit finishes,
 * so we re-enter sig_quit.
 *
 * Multiple signals of the same kind are merged, so
 * hitting ^\ multiple times while in sig_quit only
 * yields a single signal being delivered after we
 * finish in sig_quit.
 *
 * However: hit ^\, then ^C and note that sig_int
 * executes immediately.  We were transferred out of
 * sig_quit, then returned immediately back into
 * sig_quit.
 *
 * Finally, note that if you hit ^\, then hit ^\
 * again, then ^C, you should see us entering
 * sig_quit, then sig_int, then re-enter sig_quit
 * without the first sig_quit invocation having
 * terminated, since jumping out of the handler
 * unblocked the signal.
 */

#include <err.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
main(void) {
	(void)printf("\n=> Establishing initial signal hander via signal(3).\n");
	if (signal(SIGQUIT, sig_quit) == SIG_ERR) {
		err(EXIT_FAILURE, "unable to set SIGQUIT signal handler");
		/* NOTREACHED */
	}

	if (signal(SIGINT, sig_int) == SIG_ERR) {
		err(EXIT_FAILURE, "unable to set SIGINT signal handler");
		/* NOTREACHED */
	}

	(void)sleep(SLEEP);

	(void)printf("\n=> Time for a second interruption.\n");
	(void)sleep(SLEEP);

	(void)printf("Now exiting.\n");
	exit(EXIT_SUCCESS);
}

/* ============================================================================
 * DOCUMENTATION
 * ============================================================================
 *
 * INTENT:
 *   Demonstrates the behavior of UNIX signal handlers: signal merging, signal
 *   nesting, and the blocking of the current signal while its handler runs.
 *   Two handlers are installed (SIGQUIT via ^\, SIGINT via ^C).
 *   Key observations to demonstrate interactively:
 *     ^\  → enters sig_quit; sleeps SLEEP seconds.
 *     ^\  again while sleeping → queued, delivered AFTER sig_quit returns.
 *     ^\  many times → still only ONE delivery after handler returns (merging).
 *     ^C  while in sig_quit → sig_int executes IMMEDIATELY (nested handlers).
 *
 * MACROS:
 *   SLEEP          - Sleep duration in seconds; default 5. Override with
 *                    -DSLEEP=N at compile time.
 *   SIGQUIT        - Signal 3; sent by Ctrl+\; default action: core dump.
 *   SIGINT         - Signal 2; sent by Ctrl+C; default action: terminate.
 *   SIG_ERR        - Returned by signal() on failure; cast of -1.
 *   EXIT_FAILURE   - 1; returned if signal registration fails.
 *   EXIT_SUCCESS   - 0; returned at end.
 *
 * VARIABLES:
 *   int s          - Global counter; incremented on each handler entry to
 *                    track how many times handlers have been called.
 *
 * FUNCTIONS:
 *   sig_quit(signo) - SIGQUIT handler. Increments s, sleeps SLEEP seconds.
 *                     During sleep, SIGQUIT is BLOCKED (same signal blocked
 *                     while handler runs). Other signals (SIGINT) can still
 *                     interrupt the sleep(). Returns normally (doesn't longjmp).
 *   sig_int(signo)  - SIGINT handler. Increments s, returns immediately.
 *                     Can interrupt sig_quit's sleep().
 *   main(void)      - Installs handlers via signal(3); sleeps twice.
 *   signal(sig, fn) - Installs fn as the handler for signal sig. Returns
 *                     the previous handler. NOT async-signal-safe itself, but
 *                     the installed fn is called asynchronously.
 *   sleep(n)        - Can be interrupted by any delivered (unblocked) signal.
 *   err(status,msg) - Prints error + errno string, calls exit.
 *   exit(status)    - Clean termination.
 *
 * ALGORITHM:
 *   1. Install sig_quit for SIGQUIT; install sig_int for SIGINT.
 *   2. sleep(SLEEP) — first demonstration window (user sends signals here).
 *   3. Print "Time for a second interruption."
 *   4. sleep(SLEEP) — second demonstration window.
 *   5. Print "Now exiting." and exit.
 *
 * SIGNAL BEHAVIOR NOTES:
 *   - When a signal handler is executing, the SAME signal is blocked by default
 *     (via sigaction SA_NODEFER semantics of legacy signal(3)).
 *   - Multiple queued instances of the same signal are MERGED into one.
 *   - A different signal (SIGINT) can interrupt the handler's sleep().
 *   - The interrupted sleep() in sig_quit returns early; sig_quit then
 *     returns, and control resumes where sig_quit was interrupted.
 *
 * KEY SYSCALLS / LIBRARY FUNCTIONS:
 *   signal(3)   - Legacy signal handler installation; prefer sigaction(2).
 *   sleep(3)    - Suspends process; restartable by any unblocked signal.
 *   err(3)      - Prints "progname: msg: strerror(errno)" and exits.
 *
 * ============================================================================
 */
