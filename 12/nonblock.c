/* ============================================================================
 * The Escape from Blocking I/O
 * ============================================================================
 * As servers handled millions of connections, sleeping on a slow read was fatal. O_NONBLOCK was introduced to change the rules: if data isn't ready, the kernel immediately returns EAGAIN ('try again later') instead of sleeping. This single flag makes Node.js, Nginx, and modern asyncio possible.
 * ============================================================================
 */
/*
 * This program uses nonblocking I/O to write 50
 * megabytes of data to its standard output, reporting
 * on standard error the number of bytes written by
 * each system call.  If standard output is a pipe or
 * network, connection, we may observe that some write
 * system calls "fail", because buffers are full, or
 * flow-control has been applied; this is normal.
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFSIZE 1024000

int
main(int argc, char **argv) {
	int flags, count, loops, num;
	char buf[BUFSIZE], *ptr;

	(void)argv;

	/* fill buffer with 'a' */
	memset(buf, 'a', BUFSIZE);

	if ((flags = fcntl(STDOUT_FILENO, F_GETFL, 0)) < 0) {
		perror("getting file flags");
		exit(EXIT_FAILURE);
		/* NOTREACHED */
	}

	if (argc > 1) {
		/* set non-blocking mode on stdout */
		if (fcntl(STDOUT_FILENO, F_SETFL, flags|O_NONBLOCK) < 0) {
			perror("setting file flags");
			exit(EXIT_FAILURE);
			/* NOTREACHED */
		}
	}

	for (loops = 0; loops < 50; loops++) {
		ptr = buf;
		num = BUFSIZE;
		while(num > 0) {
			count = write(STDOUT_FILENO, ptr, num);
			if (count >= 0) {
				ptr += count;
				num -= count;
				(void)fprintf(stderr, "wrote %d bytes\n", count);
			} else {
				(void)fprintf(stderr, "write error: %s\n",
						strerror(errno));
			}
		}
	}
	/* set file flags back as they were; not strictly necessary, since
	 * we exit right away, but illustrative of good practice */
	(void)fcntl(STDOUT_FILENO, F_SETFL, flags);
	exit(EXIT_SUCCESS);
}

/* ============================================================================
 * DOCUMENTATION
 * ============================================================================
 *
 * INTENT:
 *   Demonstrates non-blocking I/O using the O_NONBLOCK flag on stdout.
 *   Attempts to write 50 MB of data through stdout, observing that when
 *   stdout is a pipe or network connection with a full buffer, write() returns
 *   -1 with errno=EAGAIN rather than blocking. Compares blocking (default)
 *   vs non-blocking behavior. Named "busy polling" pattern.
 *   Test: ./nonblock → blocking. ./nonblock 1 → non-blocking.
 *
 * MACROS:
 *   BUFSIZE        - 1024000 bytes (~1 MB); one chunk of output data.
 *   STDOUT_FILENO  - 1; fd whose flags we inspect and modify.
 *   O_NONBLOCK     - Flag bit: make I/O operations return EAGAIN instead of
 *                    blocking when they would otherwise block.
 *   F_GETFL        - fcntl() command: get current file status flags.
 *   F_SETFL        - fcntl() command: set file status flags.
 *   EXIT_FAILURE   - 1; returned if fcntl() fails.
 *   EXIT_SUCCESS   - 0; returned on completion.
 *
 * VARIABLES:
 *   int flags      - Current file status flags for stdout (from F_GETFL).
 *                    Saved so they can be restored after the demo.
 *   int count      - Bytes written in each write() call. May be 0 or partial
 *                    in non-blocking mode.
 *   int loops      - Outer loop counter (0..49); 50 iterations × 1MB = 50 MB.
 *   int num        - Remaining bytes to write in the inner loop.
 *   char buf[]     - 1 MB buffer filled with 'a'; never freed (exits soon).
 *   char *ptr      - Moving pointer into buf; advances by 'count' each write.
 *
 * FUNCTIONS:
 *   main(int, char**) - Sets up buffer, optionally enables O_NONBLOCK, writes.
 *   memset(buf,'a',n) - Fills the entire buffer with 'a' characters.
 *   fcntl(fd, F_GETFL, 0) - Gets current flags of fd. Used to add O_NONBLOCK
 *                    without clobbering existing flags (O_RDONLY etc.).
 *   fcntl(fd, F_SETFL, flags|O_NONBLOCK) - Enables non-blocking mode.
 *   write(fd, ptr, n) - Attempts to write n bytes. Non-blocking: returns -1
 *                    with EAGAIN if the buffer is full. Blocking: waits until
 *                    buffer drains before returning. Return MUST be checked!
 *   fprintf(stderr,...) - Reports bytes written or error for each write().
 *   strerror(errno) - Converts errno (EAGAIN, EPIPE, etc.) to string.
 *   exit(status)   - Terminates after restoring flags.
 *
 * ALGORITHM:
 *   1. Fill buf[BUFSIZE] with 'a' via memset.
 *   2. fcntl(STDOUT_FILENO, F_GETFL, 0) → save current flags.
 *   3. If argc > 1: fcntl(F_SETFL, flags | O_NONBLOCK) → enable NONBLOCK.
 *   4. Loop 50 times (50 MB total):
 *      a. ptr = buf; num = BUFSIZE (reset write pointer for each 1 MB chunk).
 *      b. Inner loop while num > 0:
 *         - count = write(STDOUT_FILENO, ptr, num).
 *         - If count >= 0: advance ptr += count; num -= count. Report.
 *         - If count == -1: report errno (EAGAIN means buffer full; keep trying).
 *   5. fcntl(F_SETFL, original flags) → restore (good practice).
 *   6. exit(EXIT_SUCCESS).
 *
 * KEY SYSCALLS / LIBRARY FUNCTIONS:
 *   fcntl(2)    - Get/set file descriptor flags; used for O_NONBLOCK.
 *   write(2)    - Kernel write; non-blocking variant returns EAGAIN not 0.
 *   memset(3)   - C library; fills memory region with a byte value.
 *
 * ============================================================================
 */
