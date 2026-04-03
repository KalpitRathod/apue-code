/* ============================================================================
 * The Illusion of the Directory Switch
 * ============================================================================
 * Changing directories doesn't actually 'move' anything on disk. It simply changes a single string/pointer inside the kernel's process structure showing the 'Current Working Directory'. If a child process uses 'cd', it only changes its own structure, not the parent shell's. That's why 'cd' must be a shell built-in.
 * ============================================================================
 */
/* This program illustrates the uselessness of an external cd(1) command,
 * as required by POSIX.1; note that many Unix versions don't bother
 * implementing /usr/bin/cd anymore, but FreeBSD and Mac OS X, for
 * example, still provide it via a generic shell builtin wrapper:
 * http://www.freebsd.org/cgi/cvsweb.cgi/~checkout~/src/usr.bin/alias/generic.sh?rev=HEAD;content-type=text%2Fplain
 */
#include <sys/param.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int
main(int argc, char **argv) {
	char buf[MAXPATHLEN];

	if (argc != 2) {
		fprintf(stderr, "%s: need a directory!\n", argv[0]);
		return(EXIT_FAILURE);
	}

	if (chdir(argv[1]) == -1) {
		fprintf(stderr, "%s: unable to chdir to %s: %s\n", argv[0],
						argv[1], strerror(errno));
		return(EXIT_FAILURE);
	}

	printf("Ok, cwd is now: %s\n", getcwd(buf, MAXPATHLEN));

	exit(EXIT_SUCCESS);
}
