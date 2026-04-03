/* ============================================================================
 * The Cost of Paranoia: O_SYNC
 * ============================================================================
 * Normally, UNIX lies to you: when write() finishes, the data is only in RAM, not yet on the metal disk. It gets synced later. But what if the power goes out? O_SYNC forces the kernel to wait for the physical hard drive platters to spin and confirm the write, brutalizing performance but guaranteeing survival.
 * ============================================================================
 */
/* simple-cat.c, O_SYNC version
 *
 * Use this program to illustrate the use of fcntl(2)
 * to set the O_SYNC flag on STDOUT, which we do not
 * open ourselves, so can't set that flag on at that
 * time.
 *
 * This also illustrates the difference between
 * synchronous and asynchronous I/O:
 *
 * Create a large enough file:
 *
 * dd if=/dev/zero of=file bs=$((1024 * 1024)) count=25
 *
 * Then run this program:
 *
 * time ./a.out <file >out
 *
 * Then, comment out the setting of O_SYNC, recompile
 * the program and run it again.
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int
main(int argc, char **argv) {
	int n;
	char buf[BUFSIZ];
	int flags;

	/* cast to void to silence compiler warnings */
	(void)argc;
	(void)argv;

	if ((flags = fcntl(STDOUT_FILENO, F_GETFL, 0)) < 0) {
		perror("Can't get file descriptor flags");
		exit(EXIT_FAILURE);
	}

	//flags |= O_SYNC;

	if (fcntl(STDOUT_FILENO, F_SETFL, flags) < 0) {
		perror("Can't set file descriptor flags");
		exit(EXIT_FAILURE);
	}

	while ((n = read(STDIN_FILENO, buf, BUFSIZ)) > 0 )
		if ( write(STDOUT_FILENO, buf, n) != n ) {
			perror("write error");
			exit(EXIT_FAILURE);
		}

	if (n < 0) {
		perror("read error");
		exit(EXIT_FAILURE);
	}

	return EXIT_SUCCESS;
}
