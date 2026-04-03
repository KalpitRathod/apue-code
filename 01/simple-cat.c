/* ============================================================================
 * Cat: Unbuffered Bytes Flowing
 * ============================================================================
 * The 'cat' command (concatenate) was one of the earliest tools in Version 1 UNIX (1971). Before advanced buffered I/O libraries, programmers had to chunk byte streams themselves directly from the disk to the terminal. This file rebuilds that raw, low-level data pump without standard library comforts.
 * ============================================================================
 */
/*
 * Stripped down version of 'cat', using unbuffered I/O.
 * ./simple-cat < simple-cat.c
 *
 * Guess what, this is also a primitive version of 'cp':
 * ./simple-cat <simple-cat.c >simple-cat.copy
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* We'll see later why / how we picked this number. */
#ifndef BUFFSIZE
#define BUFFSIZE 32768
#endif

int
main(int argc, char **argv) {
	int n;
	char buf[BUFFSIZE];

	/* cast to void to silence compiler warnings */
	(void)argc;
	(void)argv;

	while ((n = read(STDIN_FILENO, buf, BUFFSIZE)) > 0) {
		if (write(STDOUT_FILENO, buf, n) != n) {
			fprintf(stderr, "Unable to write: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
	}

	if (n < 0) {
		fprintf(stderr, "Unable to read: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	return(EXIT_SUCCESS);
}

/* ============================================================================
 * DOCUMENTATION
 * ============================================================================
 *
 * INTENT:
 *   A stripped-down clone of cat(1) built with POSIX unbuffered I/O syscalls
 *   (read/write) instead of C-library stdio. Shows how to correctly copy
 *   data from stdin to stdout in a loop, handling EOF and errors separately.
 *   Also functions as a primitive cp: ./simple-cat < src > dst.
 *
 * MACROS:
 *   BUFFSIZE       - Default 32768 bytes (32 KiB). Chosen to match common
 *                    filesystem block sizes for efficient I/O. Can be
 *                    overridden at compile time with -DBUFFSIZE=N.
 *   EXIT_SUCCESS   - 0; returned to the shell on success.
 *   EXIT_FAILURE   - 1; returned to the shell on fatal error.
 *   STDIN_FILENO   - Macro for fd 0 (standard input).
 *   STDOUT_FILENO  - Macro for fd 1 (standard output).
 *
 * VARIABLES:
 *   int n          - Bytes returned by read(). >0 = data, 0 = EOF, -1 = error.
 *   char buf[]     - Stack-allocated read buffer of size BUFFSIZE.
 *   int argc       - Argument count; cast to void (unused).
 *   char **argv    - Argument vector; cast to void (unused).
 *
 * FUNCTIONS:
 *   main(int, char**) - Program entry; loops reading stdin and writing stdout.
 *   read(fd, buf, n)  - Syscall; reads up to n bytes from fd. Returns bytes
 *                       actually read. Fewer bytes than requested is normal
 *                       (e.g., at end of pipe chunk).
 *   write(fd, buf, n) - Syscall; writes exactly n bytes. Partial writes can
 *                       occur on pipes/sockets; the return value MUST be
 *                       checked and the remainder retried in production code.
 *   fprintf(stderr)   - Write error message. Uses fd 2 directly.
 *   strerror(errno)   - Converts errno integer to human-readable string.
 *   exit(status)      - Terminates with full C cleanup.
 *
 * ALGORITHM:
 *   1. Loop: read up to BUFFSIZE bytes from stdin (fd 0).
 *      a. If read() returns >0: write those bytes to stdout (fd 1).
 *         - If write() doesn't write all n bytes, report error and exit.
 *      b. If read() returns 0: EOF reached; exit the loop.
 *   2. After the loop, if read() returned -1, report the error and exit.
 *   3. Return EXIT_SUCCESS.
 *
 * KEY SYSCALLS / LIBRARY FUNCTIONS:
 *   read(2)       - Unbuffered kernel read; returns signed byte count.
 *   write(2)      - Unbuffered kernel write; must check return value.
 *   strerror(3)   - Converts errno to string; NOT async-signal-safe.
 *   fprintf(3)    - Buffered write to a FILE* stream (stderr here).
 *
 * ============================================================================
 */
