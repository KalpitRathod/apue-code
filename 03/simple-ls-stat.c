/* ============================================================================
 * Peeking at the Inode: stat()
 * ============================================================================
 * While 'ls' reads the directory file, the directory only stores the filename and inode number. To get the file size, type, and permissions, you have to query the actual inode on disk. The stat() syscall retrieves this hidden metadata structure, making commands like 'ls -l' possible.
 * ============================================================================
 */
/*
 * Variant of 'simple-ls' that illustrates the
 * difference between stat(2) and lstat(2).
 *
 * Create a few files and symlink to different types
 * of files to show.
 */

#include <sys/types.h>
#include <sys/stat.h>

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char *
getType(const struct stat sb) {
	if (S_ISREG(sb.st_mode)) {
		return "regular file";
	} else if (S_ISDIR(sb.st_mode)) {
		return "directory";
	} else if (S_ISCHR(sb.st_mode)) {
		return "character special";
	} else if (S_ISBLK(sb.st_mode)) {
		return "block special";
	} else if (S_ISFIFO(sb.st_mode)) {
		return "FIFO";
	} else if (S_ISLNK(sb.st_mode)) {
		return "symbolic link";
	} else if (S_ISSOCK(sb.st_mode)) {
		return "socket";
	} else {
		return "unknown";
	}
}

int
main(int argc, char **argv) {

	DIR *dp;
	struct dirent *dirp;

	if (argc != 2) {
		fprintf(stderr, "usage: %s dir_name\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	if ((dp = opendir(argv[1])) == NULL) {
		fprintf(stderr, "can't open '%s'\n", argv[1]);
		exit(EXIT_FAILURE);
	}

	if (chdir(argv[1]) == -1) {
		fprintf(stderr, "can't chdir to '%s': %s\n", argv[1], strerror(errno));
		exit(EXIT_FAILURE);
	}

	while ((dirp = readdir(dp)) != NULL) {
		struct stat sb;
		char *statType;

		printf("%s: ", dirp->d_name);
		if (stat(dirp->d_name, &sb) == -1) {
			fprintf(stderr, "Can't stat %s: %s\n", dirp->d_name,
						strerror(errno));
			statType = "unknown";
		} else {
			statType = getType(sb);
		}

		if (lstat(dirp->d_name, &sb) == -1) {
			fprintf(stderr,"Can't lstat %s: %s\n", dirp->d_name,
						strerror(errno));
			continue;
		} else if (S_ISLNK(sb.st_mode)) {
			printf("symlink to ");
		}
		printf("%s\n", statType);
	}

	(void)closedir(dp);
	return EXIT_SUCCESS;
}

/* ============================================================================
 * DOCUMENTATION
 * ============================================================================
 *
 * INTENT:
 *   Extends simple-ls to show the type of each directory entry using stat(2)
 *   and lstat(2). Illustrates the difference: stat() follows symlinks (reports
 *   the target's type), lstat() does not (reports "symbolic link"). By calling
 *   both and comparing, we can say "symlink to regular file" etc.
 *
 * MACROS:
 *   EXIT_SUCCESS   - 0; returned on success.
 *   EXIT_FAILURE   - 1; returned on wrong argc or opendir failure.
 *
 * VARIABLES:
 *   int argc              - Must be exactly 2.
 *   char **argv           - argv[1] is the directory path to list.
 *   DIR *dp               - Open directory stream handle.
 *   struct dirent *dirp   - Current directory entry; d_name = filename.
 *   struct stat sb        - Filled by stat()/lstat(); sb.st_mode = type+perms.
 *   char *statType        - String description of file type from getType().
 *
 * FUNCTIONS:
 *   getType(sb)           - Returns a string describing the file type based
 *                           on the st_mode field using S_IS*() macros:
 *                           S_ISREG, S_ISDIR, S_ISCHR, S_ISBLK, S_ISFIFO,
 *                           S_ISLNK, S_ISSOCK. These macros mask st_mode with
 *                           S_IFMT and compare to the type constants.
 *   main(int, char**)     - Opens dir, iterates entries, prints type.
 *   chdir(path)           - Changes working directory so stat/lstat calls on
 *                           d_name (relative path) work correctly.
 *   opendir(path)         - Opens a directory stream.
 *   readdir(DIR*)         - Returns next directory entry.
 *   closedir(DIR*)        - Closes directory stream.
 *   stat(path, &sb)       - Follows symlinks; reports on the target file.
 *   lstat(path, &sb)      - Does NOT follow symlinks; if path is a symlink,
 *                           reports on the symlink itself.
 *   strerror(errno)       - Converts errno to string.
 *
 * ALGORITHM:
 *   1. Validate argc == 2; open directory.
 *   2. chdir(argv[1]) so d_name entries can be stat'd as relative paths.
 *   3. Loop readdir():
 *      a. Call stat(d_name, &sb) → follows symlinks → get target type.
 *      b. Call lstat(d_name, &sb) → if S_ISLNK(sb.st_mode), print "symlink to".
 *      c. Print the type string from getType().
 *   4. closedir().
 *
 * KEY SYSCALLS / LIBRARY FUNCTIONS:
 *   stat(2)    - Follows symlinks; fails if target is broken or inaccessible.
 *   lstat(2)   - Reports on symlink itself; always succeeds for existing link.
 *   chdir(2)   - Changes CWD so relative d_name lookups resolve correctly.
 *
 * ============================================================================
 */
