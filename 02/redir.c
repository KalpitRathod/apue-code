/* ============================================================================
 * The Magic of Redirection
 * ============================================================================
 * One of UNIX's most profound inventions was I/O redirection ('<', '>'). Because FDs 0, 1, and 2 are assumed to be standard streams, you can close stdout (1), open a new file (which becomes 1), and suddenly printf() writes to the disk instead of the screen. This file does exactly that.
 * ============================================================================
 */
/*
 * This simple program illustrates output redirection via dup(2).
 *
 * Compare:
 * ./a.out
 * ./a.out >/dev/null
 * ./a.out 2>/dev/null
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define STDOUT_MSG "A message to stdout.\n"
#define STDERR_MSG "A message to stderr.\n"

void
writeBoth(const char *mark) {
	int len, marklen;

	marklen = strlen(mark);
	if (write(STDOUT_FILENO, mark, marklen) != marklen) {
		perror("unable to write marker to stdout");
		exit(EXIT_FAILURE);
	}

	len = strlen(STDOUT_MSG);
	if (write(STDOUT_FILENO, STDOUT_MSG, len) != len) {
		perror("unable to write to stdout");
		exit(EXIT_FAILURE);
	}

	if (write(STDERR_FILENO, STDERR_MSG, len) != len) {
		perror("unable to write to stdout");
		exit(EXIT_FAILURE);
	}
}

int
main() {
	writeBoth("before dup2\n");

	if (dup2(STDOUT_FILENO, STDERR_FILENO) < 0) {
		perror("Unable to redirect stderr to stdout");
		exit(EXIT_FAILURE);
	}

	writeBoth("after dup2\n");
}
