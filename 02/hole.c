/* ============================================================================
 * Sparse Files: The Illusion of Data
 * ============================================================================
 * Storage in the 70s was wildly expensive. If a database needed a gigabyte index but only used the ends, a naive filesystem would waste all that disk space. UNIX introduced 'holes'—regions of a file that are artificially empty, taking up zero physical disk blocks but reading as zeroes.
 * ============================================================================
 */
/*
 * This simple program creates a file with a hole in it, a so-called
 * sparse file.  Use hexdump(1) or od(1) to verify that there is no data
 * in between the two strings; then copy the file to to another and
 * observe that the kernel supplied zeros to fill the hole.
 */

#include <sys/types.h>
#include <sys/stat.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char buf1[] = "abcdefghij";
char buf2[] = "ABCDEFGHIJ";

#define BIGNUM 10240000

int
main(void) {
	int fd;

	if ((fd = creat("file.hole", S_IRUSR | S_IWUSR)) < 0) {
		perror("creat error");
		return EXIT_FAILURE;
	}

	if (write(fd, buf1, strlen(buf1)) != sizeof(buf1) - 1) {
		perror("error writing buf1");
		return EXIT_FAILURE;
	}

	if (lseek(fd, BIGNUM, SEEK_CUR) == -1) {
		perror("lseek error");
		return EXIT_FAILURE;
	}

	if (write(fd, buf2, strlen(buf2)) != sizeof(buf2) - 1) {
		perror("error writing buf2");
		return EXIT_FAILURE;
	}

	(void)close(fd);
	return EXIT_SUCCESS;
}

/* ============================================================================
 * DOCUMENTATION
 * ============================================================================
 *
 * INTENT:
 *   Creates a "sparse file" — a file with a large hole (unwritten region)
 *   that consumes almost no actual disk space. The kernel supplies zeros for
 *   the unwritten region when read. Illustrates how lseek() past EOF then
 *   write() creates holes. Demonstrates an important filesystem feature.
 *   Verify: ls -lh file.hole (reported size) vs du -sh file.hole (disk use).
 *
 * MACROS:
 *   BIGNUM         - 10240000 bytes (~10 MB); size of the hole between the
 *                    two data chunks. Defined here for easy adjustment.
 *   S_IRUSR        - Bit 0400; owner read permission for creat().
 *   S_IWUSR        - Bit 0200; owner write permission for creat().
 *   EXIT_FAILURE   - 1; returned on any I/O error.
 *   EXIT_SUCCESS   - 0; returned on success.
 *   SEEK_CUR       - Seek relative to current position.
 *
 * VARIABLES:
 *   char buf1[]    - Global; "abcdefghij" (10 bytes). Written at offset 0.
 *   char buf2[]    - Global; "ABCDEFGHIJ" (10 bytes). Written after the hole.
 *   int fd         - File descriptor for the newly created "file.hole".
 *
 * FUNCTIONS:
 *   main(void)          - Creates the sparse file with two data blocks.
 *   creat(path, mode)   - Creates / truncates a file, returns writable fd.
 *                         Equivalent to open(path, O_WRONLY|O_CREAT|O_TRUNC, mode).
 *   write(fd, buf, n)   - Writes n bytes; advances file offset by n.
 *   lseek(fd, n, SEEK_CUR) - Jumps n bytes forward from current position
 *                            WITHOUT writing any bytes. Creates the hole.
 *   close(fd)           - Closes the fd; cast to void (return ignored).
 *   strlen(s)           - Returns string length excluding null terminator.
 *   perror(msg)         - Prints strerror(errno) prefixed with msg to stderr.
 *
 * ALGORITHM:
 *   1. Create "file.hole" with owner rw permissions via creat().
 *   2. Write buf1 ("abcdefghij", 10 bytes) at offset 0.
 *   3. lseek forward BIGNUM bytes — this positions the file offset at
 *      byte 10+10240000 WITHOUT writing; those bytes are the "hole".
 *   4. Write buf2 ("ABCDEFGHIJ", 10 bytes) at the new offset.
 *   5. Close the file.
 *
 * KEY SYSCALLS / LIBRARY FUNCTIONS:
 *   creat(2)   - Creates and opens a file for writing.
 *   write(2)   - Kernel write; advances offset.
 *   lseek(2)   - Repositions offset; does NOT allocate disk blocks.
 *   close(2)   - Commits buffered data and releases fd.
 *
 * ============================================================================
 */
