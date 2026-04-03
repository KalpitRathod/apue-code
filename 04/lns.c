/* ============================================================================
 * The Two Types of Portals: Links
 * ============================================================================
 * Hard links point to the same physical inode, making two filenames truly identical twins. But you can't hard-link a directory (it causes infinite loops in the file tree). So symbolic links (symlinks) were invented in 4.2BSD as standard files that simply contain the text path of another file.
 * ============================================================================
 */
/* The world's most trivial 'ln -s', in a way. */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int
main(int argc, char **argv) {
	if (argc != 3) {
		fprintf(stderr, "Usage: %s source target\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	if (symlink(argv[1], argv[2]) < 0) {
		fprintf(stderr, "Unable to symlink '%s' to '%s': %s\n",
				argv[2], argv[1], strerror(errno));
		exit(EXIT_FAILURE);
	}

	return EXIT_SUCCESS;
}
