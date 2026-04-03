/* ============================================================================
 * The Integer File Descriptors
 * ============================================================================
 * In the UNIX 'Everything is a File' paradigm, there are no complex structures to refer to open files—just small integers. The kernel promises to always hand out the lowest available number. This simple rule allowed programmers to predict and manipulate file channels elegantly.
 * ============================================================================
 */
/*
 * A program to show the value of some file
 * descriptors.
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int
main() {
	int fd1, fd2, fd3;
	FILE *f;

	printf("STDIN_FILENO: %d\n", STDIN_FILENO);
	printf("stdin: %d\n", fileno(stdin));
	printf("STDOUT_FILENO: %d\n", STDOUT_FILENO);
	printf("stdout: %d\n", fileno(stdout));
	printf("STDERR_FILENO: %d\n", STDERR_FILENO);
	printf("stderr: %d\n", fileno(stderr));

	printf("\nOpening /dev/zero...\n");
	if ((fd1 = open("/dev/zero", O_RDONLY)) < 0) {
		fprintf(stderr, "Unable to open /dev/zero: %s\n", strerror(errno));
	} else {
		printf("fd1: %d\n", fd1);
	}

	printf("\nOpening /dev/zero a second time...\n");
	if ((fd2 = open("/dev/zero", O_RDONLY)) < 0) {
		fprintf(stderr, "Unable to open /dev/zero: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	printf("fd2: %d\n", fd2);

	printf("\nNow closing fd1, but keeping fd2 open..\n");
	(void)close(fd1);

	printf("\nOpening /dev/zero a third time...\n");
	if ((fd3 = open("/dev/zero", O_RDONLY)) < 0) {
		fprintf(stderr, "Unable to open /dev/zero: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	printf("fd3: %d\n", fd3);

	printf("\nNow closing fd2 and fd3.\n");
	(void)close(fd2);
	(void)close(fd3);

	printf("Now opening /dev/zero as a stream.\n");
	if ((f = fopen("/dev/zero", "r")) == NULL) {
		fprintf(stderr, "Unable to open /dev/zero: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	printf("f: %d\n", fileno(f));
	(void)fclose(f);

	(void)close(STDERR_FILENO);
	printf("\nOpening /dev/stderr after closing STDERR...\n");
	if ((fd3 = open("/dev/stderr", O_RDONLY)) < 0) {
		fprintf(stdout, "Unable to open /dev/stderr: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	printf("stderr is now: %d\n", fd3);

	return EXIT_SUCCESS;
}

/* ============================================================================
 * DOCUMENTATION
 * ============================================================================
 *
 * INTENT:
 *   Shows how the kernel assigns file descriptor integers and how they are
 *   reused after close(). Demonstrates that the kernel always assigns the
 *   LOWEST available fd, that stdio FILE* streams have underlying integer fds,
 *   and that closing one fd doesn't affect others pointing to the same file.
 *
 * MACROS:
 *   STDIN_FILENO   - 0; fd for standard input.
 *   STDOUT_FILENO  - 1; fd for standard output.
 *   STDERR_FILENO  - 2; fd for standard error.
 *   O_RDONLY       - Open flag: open for reading only (no writing).
 *   EXIT_FAILURE   - 1; returned on error.
 *   EXIT_SUCCESS   - 0; returned on success.
 *
 * VARIABLES:
 *   int fd1, fd2, fd3  - Integer file descriptors returned by open().
 *                        fd1: first open of /dev/zero (gets 3).
 *                        fd2: second open of /dev/zero (gets 4).
 *                        fd3: third open after fd1 closed (reuses 3).
 *   FILE *f            - stdio stream wrapping an fd; fileno(f) retrieves
 *                        the underlying integer fd.
 *
 * FUNCTIONS:
 *   main()             - Entry point; demonstrates fd allocation rules.
 *   open(path, flags)  - Opens/creates a file; returns lowest free fd.
 *   close(fd)          - Closes fd; makes the number available for reuse.
 *   fopen(path, mode)  - C library wrapper around open(); returns FILE*.
 *   fclose(fp)         - C library wrapper around close(); flushes and frees.
 *   fileno(FILE*)      - Returns the underlying integer fd of a FILE* stream.
 *   printf(fmt, ...)   - Prints fd numbers to stdout.
 *   fprintf(stderr,...) - Prints error messages.
 *   strerror(errno)    - Converts errno to human-readable string.
 *   exit(status)       - Terminates with C library cleanup.
 *
 * ALGORITHM:
 *   1. Print the values of STDIN, STDOUT, STDERR via fileno(stdin/out/err).
 *   2. open("/dev/zero") → fd1 (expect 3, next after 0,1,2).
 *   3. open("/dev/zero") again → fd2 (expect 4).
 *   4. close(fd1) — fd 3 is now free.
 *   5. open("/dev/zero") again → fd3 (expect 3, reusing fd1's slot).
 *   6. close(fd2) and close(fd3).
 *   7. fopen("/dev/zero") — also gets fd 3 (same allocation rule).
 *   8. close(STDERR_FILENO), then open("/dev/stderr") → gets fd 2 (reused).
 *
 * KEY SYSCALLS / LIBRARY FUNCTIONS:
 *   open(2)      - Kernel syscall; assigns lowest free fd.
 *   close(2)     - Releases fd; makes integer reusable.
 *   fileno(3)    - C library; reads fd from FILE* internals.
 *   fopen(3)     - C library; wraps open(2).
 *
 * ============================================================================
 */
