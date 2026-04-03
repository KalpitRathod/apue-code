/* ============================================================================
 * Freezing Time with GDB
 * ============================================================================
 * Richard Stallman created the GNU Debugger (GDB) in 1986. Before debuggers, programmers relied on printf. GDB uses the ptrace() syscall to literally freeze a running process, peek inside its memory registers, and alter variables on the fly. This file contains intentional flaws to trace.
 * ============================================================================
 */
/* This file is used together with the file 'buf.c' to
 * help students use gdb(1) to understand
 * manipulations of strings and buffers.
 *
 * See https://youtu.be/hgcj7iAxhhU as well as
 * 'pointer.c'
 */

#include <stdlib.h>
#include <stdio.h>

void printBufs(long);

int
main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "Usage: %s num\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	printBufs(strtol(argv[1], NULL, 10));
}
