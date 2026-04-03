/* ============================================================================
 * Atomic Movements
 * ============================================================================
 * Moving a file used to require two steps: link the new name, unlink the old name. If the power failed in between, you had two copies or a corrupted directory. The rename() syscall was added to guarantee atomic movement within the kernel, ensuring the filesystem never breaks during structural moves.
 * ============================================================================
 */
/* The world's most trivial 'mv', in a way. */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int
main(int argc, char **argv) {
	if (argc != 3) {
		fprintf(stderr, "Usage: %s from to\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	if (rename(argv[1], argv[2]) < 0) {
		fprintf(stderr, "Unable to rename '%s' to '%s': %s\n",
				argv[1], argv[2], strerror(errno));
		exit(EXIT_FAILURE);
	}

	return EXIT_SUCCESS;
}
