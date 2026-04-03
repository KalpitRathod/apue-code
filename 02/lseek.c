/* ============================================================================
 * The Virtual Read Head
 * ============================================================================
 * On magnetic tape drives, reading data literally involved seeking a physical tape head. UNIX carried this metaphor forward into random-access disk drives. lseek() moves the invisible read/write pointer, but this file demonstrates that some streams (like pipes) physically cannot be rewound.
 * ============================================================================
 */
/*
 * This trivial program verifies whether or not STDIN is seekable.  Test
 * on a regular file, on a pipe or a fifo.
 */

#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int
main(void) {
	if (lseek(STDIN_FILENO, 0, SEEK_CUR) == -1 )
		printf("cannot seek\n");
	else
		printf("seek OK\n");

	return EXIT_SUCCESS;
}

/* ============================================================================
 * DOCUMENTATION
 * ============================================================================
 *
 * INTENT:
 *   Tests whether stdin is seekable (i.e., backed by a regular file) or not
 *   (pipe, FIFO, socket, terminal). lseek() with SEEK_CUR and offset 0 is a
 *   no-op that only checks seekability. Demonstrates that not all file
 *   descriptors support random access.
 *   Test:  ./lseek < file.txt    → "seek OK"
 *          cat file.txt | ./lseek → "cannot seek"
 *
 * MACROS:
 *   STDIN_FILENO   - fd 0; standard input file descriptor.
 *   SEEK_CUR       - Seek relative to current position. With offset 0 it
 *                    simply queries the current offset without moving.
 *   EXIT_SUCCESS   - 0; indicates normal exit.
 *
 * VARIABLES:
 *   (none)         - No local variables; lseek() result used inline.
 *
 * FUNCTIONS:
 *   main(void)     - Entry point; no arguments needed.
 *   lseek(fd, offset, whence) - Reposition file offset. Returns new offset,
 *                    or -1 on error. Sets errno = ESPIPE for non-seekable fds.
 *   printf(fmt)    - Prints seekable/not-seekable message to stdout.
 *
 * ALGORITHM:
 *   1. Call lseek(STDIN_FILENO, 0, SEEK_CUR).
 *      - Returns -1 (errno=ESPIPE) for pipes, sockets, terminals.
 *      - Returns current offset (≥0) for regular files.
 *   2. Print result and exit.
 *
 * KEY SYSCALLS / LIBRARY FUNCTIONS:
 *   lseek(2)      - Kernel syscall; moves file offset for an open fd.
 *                   Fails with ESPIPE on pipes/FIFOs/sockets.
 *
 * ============================================================================
 */
