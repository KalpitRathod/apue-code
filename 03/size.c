/* ============================================================================
 * The Physical Limit Check
 * ============================================================================
 * Before dynamic arrays and large storage, knowing exactly how many bytes a file consumed on the block device was crucial. This program inspects the raw size metadata reported directly by the filesystem driver layout on disk.
 * ============================================================================
 */
/* This simple program displays the stat data relevant to file sizes. */

#include <sys/stat.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
main(int argc, char **argv) {
	struct stat sb;

	if (argc != 2) {
		fprintf(stderr, "usage: %s file\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	if (lstat(argv[1], &sb) == -1) {
		fprintf(stderr, "Can't stat %s: %s\n", argv[1],
						strerror(errno));
		exit(EXIT_FAILURE);
	}

	printf("st_size   : %d\n", (int)sb.st_size);
	printf("st_blocks : %d\n", (int)sb.st_blocks);
	printf("st_blksize: %d\n", (int)sb.st_blksize);
	exit(EXIT_SUCCESS);
}
