/* ============================================================================
 * The Infinite Loop Trap
 * ============================================================================
 * Recursion and math errors are the bane of programmers. When a program hangs, humans panic. GDB allows you to attach to a running, spinning process and look at a backtrace (bt) to see exactly which function calls trapped the execution. This file serves as bait.
 * ============================================================================
 */
/* This file is used to illustrate how to step through
 * a program at execution time using gdb(1).
 *
 * To this end, it contains a number of mistakes.
 *
 * See https://youtu.be/fOp4Q4mnTD4 for how to fix
 * them.
 */

/* The Fibonacci sequence is defined such that each number
 * is the sum of the two preceding ones. */

#include <stdio.h>
#include <stdlib.h>

int
fib(int i) {
	if (i == 0) {
		return 0;
	} else {
		return fib(i-2) + fib(i-1);
	}
}

int
main(int argc, char **argv) {
	if (argc != 2) {
		printf("usage: %s num\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	printf("%d\n", fib(atoi(argv[1])));
	exit(EXIT_SUCCESS);
}
