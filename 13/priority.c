/* ============================================================================
 * The Niceness Hierarchy
 * ============================================================================
 * Time-sharing required fairness. A process can voluntarily lower its CPU priority to be 'nice' to others. However, a massive security rule states that once you give up priority, you cannot take it back unless you are the root superuser. UNIX enforces algorithmic humility.
 * ============================================================================
 */
/* This program illustrates the use of getpriority(2)
 * and setpriority(2).  It shows that we can adjust
 * our priority by lowering it, but that we can't
 * raise it again afterwards, unless we are root.
 *
 * Run as
 * $ ./a.out 5
 * $ sudo ./a.out 5
 * $ nice -n 10 ./a.out 5
 * $ nice -n 10 ./a.out 15
 * $ nice -n -5 ./a.out 5
 */

#include <sys/resource.h>

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int
main(int argc, char **argv) {
	int n, p1, p2;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s num\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	/* If argv[1] is not a valid number, then we
	 * get 0, but so be it, we'll use that. */
	n = atoi(argv[1]);

	errno = 0;
	if ((p1 = getpriority(PRIO_PROCESS, getpid())) == -1) {
		if (errno != 0) {
			err(EXIT_FAILURE, "getpriority");
			/* NOTREACHED */
		}
	}
	(void)printf("My current priority is: %d\n", p1);

	if (setpriority(PRIO_PROCESS, getpid(), n) == -1) {
		err(EXIT_FAILURE, "setpriority");
		/* NOTREACHED */
	}

	errno = 0;
	if ((p2 = getpriority(PRIO_PROCESS, getpid())) == -1) {
		if (errno != 0) {
			err(EXIT_FAILURE, "getpriority");
			/* NOTREACHED */
		}
	}
	(void)printf("My new priority is: %d\n", p2);

	/* We expect this to fail when running with
	 * euid != 0, since we can't raise our
	 * priority even to a value we initially had.
	 */
	if (setpriority(PRIO_PROCESS, getpid(), p1) == -1) {
		(void)fprintf(stderr, "Unable to setpriority(): %s\n", strerror(errno));
	}

	errno = 0;
	if ((p2 = getpriority(PRIO_PROCESS, getpid())) == -1) {
		if (errno != 0) {
			err(EXIT_FAILURE, "getpriority");
			/* NOTREACHED */
		}
	}
	(void)printf("My priority %sis: %d\n", (p1 != p2) ? "still " : "", p2);
	return 0;
}

/* ============================================================================
 * DOCUMENTATION
 * ============================================================================
 *
 * INTENT:
 *   Demonstrates process scheduling priority manipulation via getpriority(2)
 *   and setpriority(2). Shows that an unprivileged process can LOWER its
 *   priority (increase the nice value) but CANNOT RAISE it back afterwards.
 *   Only root can raise priority. Illustrates the nice(1) interaction.
 *   Usage:
 *     ./a.out 5              → lower priority by 5, fail to restore
 *     sudo ./a.out 5         → lower then successfully raise (root can)
 *     nice -n 10 ./a.out 5  → start at nice 10, try to go to 15
 *     nice -n -5 ./a.out 5  → start at -5, try to lower by 5 more
 *
 * MACROS:
 *   PRIO_PROCESS   - getpriority/setpriority target type: a specific process
 *                    by PID. Alternatives: PRIO_PGRP (group), PRIO_USER (user).
 *   EXIT_FAILURE   - 1; returned on wrong argc or setpriority failure.
 *
 * VARIABLES:
 *   int n          - The new priority value requested via argv[1].
 *                    Range: typically -20 to +19 (-20=highest, +19=lowest).
 *   int p1         - Priority BEFORE setpriority(); saved for comparison.
 *   int p2         - Priority AFTER setpriority(); may differ from n if
 *                    the kernel clamps to the allowed range.
 *
 * FUNCTIONS:
 *   main(int, char**) - Reads n, gets current priority, sets new, tries to
 *                       restore, and shows final result.
 *   atoi(str)         - Converts argv[1] to int. Returns 0 for invalid input,
 *                       which is still a valid (and interesting) priority.
 *   getpriority(which, who) - Returns current scheduling priority.
 *                       Returns -1 on BOTH success (priority=-1) AND error.
 *                       Must set errno=0 before calling to distinguish.
 *   setpriority(which, who, prio) - Sets scheduling priority. Unprivileged
 *                       process can only raise the value (lower priority).
 *                       Returns -1 on error (EACCES if trying to increase).
 *   fprintf(stderr,...) - Error reporting.
 *   strerror(errno)     - Converts errno to string.
 *   err(status, msg)    - Error + errno message + exit.
 *   printf()            - Reports current and new priorities.
 *
 * ALGORITHM:
 *   1. Validate argc==2; n = atoi(argv[1]).
 *   2. errno = 0; p1 = getpriority(PRIO_PROCESS, getpid()).
 *      NOTE: return -1 can mean error OR priority is -1. Check errno after.
 *   3. printf current priority p1.
 *   4. setpriority(PRIO_PROCESS, getpid(), n) → try to set new priority.
 *      Fails (EACCES/EPERM) if unprivileged and n < current priority.
 *   5. errno = 0; p2 = getpriority(...) → check new effective priority.
 *   6. Try setpriority(..., p1) → try to restore original priority.
 *      Fails for unprivileged users (can't raise after lowering).
 *   7. errno = 0; p2 = getpriority(...) → final priority.
 *      Print "still" if restoration failed (p2 ≠ p1).
 *
 * KEY SYSCALLS / LIBRARY FUNCTIONS:
 *   getpriority(2)  - Gets scheduling priority; errno must be checked.
 *   setpriority(2)  - Sets scheduling priority; fails if prio < current for
 *                     non-root. Root can freely raise/lower (-20 to +19).
 *   getpid(2)       - Returns calling process's PID; used as 'who' argument.
 *
 * NICE VALUE SEMANTICS:
 *   Lower nice value  = HIGHER CPU priority = more CPU time.
 *   Higher nice value = LOWER CPU priority  = less CPU time (be "nice").
 *   Range: -20 (max priority, root only) to +19 (min priority, anyone).
 *   An unprivileged process can only increase its nice value (be NICER).
 *   Once raised (niceness increased), it cannot be lowered again without root.
 *
 * ============================================================================
 */
