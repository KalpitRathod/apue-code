/* ============================================================================
 * Peeking Inside the Directory Abstraction
 * ============================================================================
 * Directories weren't always 'folders'. In raw UNIX, a directory is just a special text file containing pairs of filenames and inode numbers. The first 'ls' commands literally parsed these text files. This program uses the dirent API to recreate early directory parsing.
 * ============================================================================
 */
/*
 * simple-ls.c
 * Extremely low-power ls clone.
 * ./simple-ls .
 */

#include <sys/types.h>

#include <errno.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
main(int argc, char **argv) {

	DIR *dp;
	struct dirent *dirp;

	if (argc != 2) {
		fprintf(stderr, "usage: %s dir_name\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	if ((dp = opendir(argv[1])) == NULL) {
		fprintf(stderr, "Unable to open '%s': %s\n",
					argv[1], strerror(errno));
		exit(EXIT_FAILURE);
	}

	while ((dirp = readdir(dp)) != NULL) {
		printf("%s\n", dirp->d_name);
	}

	(void)closedir(dp);
	return EXIT_SUCCESS;
}

/* ============================================================================
 * DOCUMENTATION
 * ============================================================================
 *
 * INTENT:
 *   A minimal clone of ls(1) that lists filenames in a given directory.
 *   Demonstrates the three-step opendir/readdir/closedir idiom for
 *   iterating directory entries using POSIX dirent APIs.
 *
 * MACROS:
 *   EXIT_SUCCESS   - 0; returned on successful listing.
 *   EXIT_FAILURE   - 1; returned if argc is wrong or opendir fails.
 *
 * VARIABLES:
 *   int argc         - Must be exactly 2: program name + directory path.
 *   char **argv      - argv[1] is the path to the directory to list.
 *   DIR *dp          - Opaque handle to an open directory stream.
 *                      Allocated and managed by the C library.
 *   struct dirent *dirp - Points to the current directory entry returned
 *                         by readdir(). Valid only until the next readdir()
 *                         call. Field d_name holds the filename as a
 *                         null-terminated C string.
 *
 * FUNCTIONS:
 *   main(int, char**)   - Validates argc, opens directory, loops entries.
 *   opendir(path)       - Opens a directory stream for path. Returns NULL
 *                         on error (directory not found, permission denied).
 *   readdir(DIR*)       - Returns next entry from the directory stream,
 *                         or NULL at end-of-directory (not an error).
 *   closedir(DIR*)      - Releases resources associated with the stream.
 *                         Cast to (void) because we ignore the return value.
 *   printf(fmt, ...)    - Prints d_name (filename) for each entry.
 *   fprintf(stderr,...) - Prints usage/error messages to stderr.
 *   strerror(errno)     - Converts errno to a readable error string.
 *   exit(status)        - Terminates with C-library cleanup.
 *
 * ALGORITHM:
 *   1. Check argc == 2; if not, print usage to stderr and exit failure.
 *   2. Call opendir(argv[1]); if NULL, print error and exit failure.
 *   3. Loop calling readdir(dp) until it returns NULL (end of directory):
 *      a. Print dirp->d_name followed by newline.
 *   4. Close the directory stream with closedir().
 *   5. Return EXIT_SUCCESS.
 *
 * KEY SYSCALLS / LIBRARY FUNCTIONS:
 *   opendir(3)    - C library; internally uses open(2)+getdents(2).
 *   readdir(3)    - C library; wraps getdents(2) syscall.
 *   closedir(3)   - C library; closes the underlying fd.
 *
 * NOTE:
 *   Does NOT sort entries (order is filesystem-dependent).
 *   Does NOT filter . and .. (they appear in the listing).
 *   Does NOT show file metadata (use stat(2) to get size, permissions, etc.).
 *
 * ============================================================================
 */
