/* ============================================================================
 * A Copy Machine in Software
 * ============================================================================
 * When you strip away the buffer management of the C library, copying data efficiently boils down to picking the right block size to read and write. Early hackers meticulously tuned this buffer size to match the physical sector size of their exact hard drives.
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
