/* ============================================================================
 * The First Words: Hello World
 * ============================================================================
 * In 1978, Brian Kernighan and Dennis Ritchie published 'The C Programming Language'. Page 6 introduced the simplest possible program to verify a compiler worked: printing 'hello, world'. This convention survived five decades to become the absolute starting point for every systems programmer.
 * ============================================================================
 */
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
	/* cast to void to suppress compiler warnings about unused variables */
	(void)argc;
	(void)argv;

	printf("Hello, World!\n");
	exit(EXIT_SUCCESS);
}

/* ============================================================================
 * DOCUMENTATION
 * ============================================================================
 *
 * INTENT:
 *   The simplest possible C program: print a greeting and exit cleanly.
 *   Serves as the canonical first UNIX C program demonstrating correct use
 *   of EXIT_SUCCESS over magic numbers, and suppressing unused-parameter
 *   warnings with (void) casts.
 *
 * MACROS:
 *   EXIT_SUCCESS   - Defined in <stdlib.h>; expands to 0. Signals successful
 *                    program termination to the calling shell/parent process.
 *
 * VARIABLES:
 *   int argc       - Argument count from the OS (number of elements in argv).
 *                    Cast to (void) since this program ignores CLI arguments.
 *   char **argv    - Argument vector; argv[0] is the program name.
 *                    Cast to (void) to suppress unused-parameter warnings.
 *
 * FUNCTIONS:
 *   main(int, char**)  - Entry point. The OS passes argc/argv; the C runtime
 *                        calls main() after setting up the process environment.
 *   printf(fmt, ...)   - Formatted output to stdout. Writes to the stdio
 *                        fully-buffered stream; buffer is flushed on exit().
 *   exit(status)       - Performs full C-library cleanup (flushes stdio
 *                        buffers, calls atexit handlers) then terminates.
 *                        Unlike _exit(), it does NOT skip C library cleanup.
 *
 * ALGORITHM:
 *   1. OS invokes main(), passing argc=1, argv={"./hw", NULL}.
 *   2. Suppress unused-parameter warnings by casting argc and argv to void.
 *   3. Print "Hello, World!\n" to stdout via printf.
 *   4. Call exit(EXIT_SUCCESS) -> flushes buffers, runs atexit handlers,
 *      returns exit status 0 to the parent process.
 *
 * KEY SYSCALLS / LIBRARY FUNCTIONS:
 *   printf(3)  - C library function; internally calls write(2) on stdout fd.
 *   exit(3)    - C library function; calls _exit(2) after cleanup.
 *
 * ============================================================================
 */
