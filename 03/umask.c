/* ============================================================================
 * The Subtractive Security Net
 * ============================================================================
 * Users constantly forgot to lock down their files when creating them. The 'umask' was introduced as a default safety net. It strictly *subtracts* permission bits. Even if a program asks to create an open-to-the-world file, the umask silently strips away the danger bits behind the scenes.
 * ============================================================================
 */
/* This simple program illustrates the use of a umask.
 *
 * First, set your umask to 022 and 'touch file'; the mode will be 0644.
 *
 * Next, run this program and find file1 with the same mode, but execute
 * permissions added, file2 with the explicitly requested 0777 mode, and
 * finally file3 with mode 0711, since we explicitly turned off other
 * permissions.
 *
 * Observe that each time the flags passed to open(2) are identical.
 */

#include <sys/types.h>
#include <sys/stat.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void
myOpen(const char *path) {
	int fd;
	if ((fd = open(path, O_CREAT | O_EXCL | O_WRONLY,
				S_IRUSR | S_IWUSR | S_IXUSR |
				S_IRGRP | S_IWGRP | S_IXGRP |
				S_IROTH | S_IWOTH | S_IXOTH)) == -1) {
		fprintf(stderr, "Unable to create %s: %s\n",
				path, strerror(errno));
		exit(EXIT_FAILURE);
	}
	(void)close(fd);
}

int
main() {
	/* current umask from your shell will apply to 'file1' */
	myOpen("file1");

	umask(0);
	/* turning all bits off means we will allow
	 * all the bits specified in open(2) */
	myOpen("file2");

	umask(S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
	/* turning group/other permissions off in the
	 * umask means they won't be set in open(2) */
	myOpen("file3");

	return EXIT_SUCCESS;
}

/* ============================================================================
 * DOCUMENTATION
 * ============================================================================
 *
 * INTENT:
 *   Shows how the process umask filters permission bits during file creation.
 *   Three files are created with IDENTICAL open() flags (0777) but different
 *   umask values, resulting in different actual permissions. Demonstrates that
 *   umask can only REMOVE bits — you can never ADD permissions via umask.
 *
 * MACROS:
 *   O_CREAT        - open(2) flag: create file if it doesn't exist.
 *   O_EXCL         - open(2) flag: fail if file already exists (atomic check).
 *   O_WRONLY       - open(2) flag: open for writing only.
 *   S_IRUSR        - Bit 0400: owner read. Combined below to form 0777.
 *   S_IWUSR        - Bit 0200: owner write.
 *   S_IXUSR        - Bit 0100: owner execute.
 *   S_IRGRP        - Bit 040: group read.
 *   S_IWGRP        - Bit 020: group write.
 *   S_IXGRP        - Bit 010: group execute.
 *   S_IROTH        - Bit 04: other read.
 *   S_IWOTH        - Bit 02: other write.
 *   S_IXOTH        - Bit 01: other execute.
 *   EXIT_FAILURE   - 1; returned if any file creation fails.
 *   EXIT_SUCCESS   - 0; returned on success.
 *
 * VARIABLES:
 *   int fd         - File descriptor returned by open(); closed immediately.
 *
 * FUNCTIONS:
 *   myOpen(path)    - Helper; creates a file with all 0777 permissions
 *                     (subject to umask). Closes the fd immediately after.
 *   main()          - Entry point; sets different umasks and calls myOpen().
 *   open(path, flags, mode) - Creates file; effective_mode = mode & ~umask.
 *   close(fd)       - Closes the newly created fd.
 *   umask(cmask)    - Sets the process umask to cmask. Returns old umask.
 *                     Inherited by children; affects all subsequent creates.
 *   fprintf(stderr,...) - Error reporting.
 *   strerror(errno) - Converts errno to text.
 *   exit(status)    - Terminates.
 *
 * ALGORITHM:
 *   1. myOpen("file1"): No umask change → inherits shell's umask (e.g., 022).
 *      Effective mode = 0777 & ~022 = 0755.
 *   2. umask(0): Turn ALL filter bits OFF. Now every bit in mode is allowed.
 *      myOpen("file2") → effective mode = 0777 & ~0 = 0777.
 *   3. umask(S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH) = 0066:
 *      Block group-write, other-write AND group-read, other-read.
 *      myOpen("file3") → effective mode = 0777 & ~0066 = 0711.
 *
 * KEY SYSCALLS / LIBRARY FUNCTIONS:
 *   umask(2)   - Sets process umask; effective_mode = requested & ~umask.
 *   open(2)    - Creates file; applies umask to the mode argument.
 *
 * ============================================================================
 */
