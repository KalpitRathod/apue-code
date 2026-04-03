/* ============================================================================
 * The Final Will and Testament
 * ============================================================================
 * Before a process vanishes into the void via exit(), the C library gives it a chance to clean its house. Using atexit(), functions are stacked in Last-In-First-Out order. However, if a program is violently killed or calls _exit(), these final wishes are mercilessly ignored.
 * ============================================================================
 */
/* A simple illustration of exit handlers.  Note that exit handlers are
 * pushed onto a stack and thus execute in reverse order.
 *
 * Illustrate exiting at different times by invoking
 * this program as
 * ./a.out            exit handlers invoked after return from main
 * ./a.out 1          exit handlers invoked from within func
 * ./a.out 1 2        no exit handlers invoked
 * ./a.out 1 2 3      we call abort(3), no exit handlers invoked
 *
 *
 * */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void
my_exit1(void) {
	(void)printf("first exit handler\n");
}

void
my_exit2(void) {
	(void)printf("second exit handler\n");
}

void
func(int argc) {
	(void)printf("In func.\n");
	if (argc == 2) {
		exit(EXIT_SUCCESS);
	} else if (argc == 3) {
		_exit(EXIT_SUCCESS);
	} else if (argc == 4) {
		abort();
	}
}


int
main(int argc, char **argv) {
	(void)argv;
	if (atexit(my_exit2) != 0) {
		perror("can't register my_exit2\n");
		exit(EXIT_FAILURE);
	}

	if (atexit(my_exit1) != 0) {
		perror("can't register my_exit1");
		exit(EXIT_FAILURE);
	}

	if (atexit(my_exit1) != 0) {
		perror("can't register my_exit1");
		exit(EXIT_FAILURE);
	}

	func(argc);

	(void)printf("main is done\n");

	return EXIT_SUCCESS;
}

/* ============================================================================
 * DOCUMENTATION
 * ============================================================================
 *
 * INTENT:
 *   Demonstrates atexit(3) exit handlers — functions called automatically
 *   during normal process termination. Shows that handlers run in LIFO order
 *   (last registered, first called), and that _exit() and abort() bypass them.
 *   Run as:
 *     ./a.out          → main returns → handlers called (LIFO order)
 *     ./a.out 1        → exit() inside func → handlers called
 *     ./a.out 1 2      → _exit() inside func → handlers NOT called
 *     ./a.out 1 2 3    → abort() inside func → handlers NOT called
 *
 * MACROS:
 *   EXIT_SUCCESS   - 0; indicates successful completion.
 *   EXIT_FAILURE   - 1; returned if atexit() registration fails.
 *
 * VARIABLES:
 *   int argc       - Argument count; passed to func() to control exit path.
 *   char **argv    - Argument vector; cast to void (unused in main).
 *
 * FUNCTIONS:
 *   my_exit1()     - Exit handler 1; prints "first exit handler".
 *   my_exit2()     - Exit handler 2; prints "second exit handler".
 *   func(argc)     - Calls exit/abort/_exit based on argc value:
 *                    argc==1: normal return (no forced exit from func)
 *                    argc==2: exit() → handlers run
 *                    argc==3: _exit() → handlers SKIP (no stdio flush either)
 *                    argc==4: abort() → SIGABRT → core dump, no handlers
 *   main()         - Registers handlers via atexit(); calls func().
 *   atexit(fn)     - Registers fn as an exit handler. LIFO execution order.
 *                    Can register the same handler multiple times.
 *                    Returns 0 on success, non-zero on failure.
 *   exit(status)   - Calls atexit handlers (LIFO), flushes stdio, terminates.
 *   _exit(status)  - Terminates IMMEDIATELY; bypasses atexit handlers and
 *                    stdio buffer flushing. Use after fork() in child that
 *                    won't exec(), to avoid double-flushing parent's buffers.
 *   abort()        - Raises SIGABRT; generates core dump; bypasses atexit.
 *   printf()       - Used inside handlers to show execution order.
 *
 * ALGORITHM:
 *   1. Register my_exit2 → registered first, runs LAST.
 *   2. Register my_exit1 → registered second, runs SECOND.
 *   3. Register my_exit1 again → registered third, runs FIRST.
 *      Handler stack (top = runs first): [my_exit1, my_exit1, my_exit2]
 *   4. Call func(argc):
 *      - Prints "In func."
 *      - Branches on argc to exit()/\_exit()/abort() or falls through.
 *   5. If func returns normally: print "main is done", return EXIT_SUCCESS.
 *   6. C runtime calls exit handlers in LIFO order if exit() was used.
 *
 * KEY SYSCALLS / LIBRARY FUNCTIONS:
 *   atexit(3)   - Registers cleanup function; stored in C library.
 *   exit(3)     - C library; runs atexit handlers then calls _exit(2).
 *   _exit(2)    - Kernel syscall; immediate termination, no cleanup.
 *   abort(3)    - Raises SIGABRT; default action is core dump.
 *
 * ============================================================================
 */
