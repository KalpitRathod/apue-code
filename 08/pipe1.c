/* ============================================================================
 * The Invention of the Pipe
 * ============================================================================
 * In 1973, Doug McIlroy realized that programs needed to be chained together like garden hoses. He badgered Ken Thompson until Thompson snapped and implemented the '|' pipe in one night. This code manually creates the magical unidirectional data tunnel that changed software philosophy forever.
 * ============================================================================
 */
/* A simple example of IPC using a pipe(2).  This may
 * be compared to
 *
 * echo msg | cat
 *
 * but we display information in each process about
 * its pid and ppid.
 *
 * Depending on the order in which the parent and
 * child end up running, the child may report a ppid
 * of '1': if the parent has exited before the child
 * has finished reading data, then init(8) will have
 * reaped (and possibly reparented) it.
 */

#include <sys/wait.h>

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int
main() {
	int n, r, fd[2];
	pid_t pid;
	char line[BUFSIZ];

	if (pipe(fd) < 0) {
		err(EXIT_FAILURE, "pipe");
		/* NOTREACHED */
	}

	if ((pid = fork()) < 0) {
		err(EXIT_FAILURE, "fork");
		/* NOTREACHED */
	} else if (pid > 0) {
		/* The parent closes the read-end of
		 * the pipe, since it only writes data
		 * to the child. */
		(void)close(fd[0]);
		(void)printf("P=> Parent process with pid %d (and its ppid %d).\n",
				getpid(), getppid());
		(void)printf("P=> Sending a message to the child process (pid %d).\n", pid);
		(void)snprintf(line, BUFSIZ, "Hello child!  I'm your parent, pid %d!\n",
				getpid());
		if ((r = write(fd[1], line, strlen(line))) < 0) {
			(void)fprintf(stderr, "Unable to write to pipe: %s\n",
					strerror(errno));
		}
		(void)close(fd[1]);
	} else {
		/* The child process closes the write
		 * end of the pipe, since it only
		 * plans on reading. */
		(void)close(fd[1]);
		(void)printf("C=> Child process with pid %d (and its ppid %d).\n",
				getpid(), getppid());
		(void)printf("C=> Reading a message from the parent (pid %d):\n",
				getppid());
		if ((n = read(fd[0], line, BUFSIZ)) < 0) {
			(void)fprintf(stderr, "Unable to read from pipe: %s\n",
					strerror(errno));
		} else if ((r = write(STDOUT_FILENO, line, n)) < 0) {
			(void)fprintf(stderr, "Unable to write to pipe: %s\n",
					strerror(errno));
		}
		(void)close(fd[0]);
	}

	(void)wait(NULL);
	return EXIT_SUCCESS;
}

/* ============================================================================
 * DOCUMENTATION
 * ============================================================================
 *
 * INTENT:
 *   Demonstrates unidirectional IPC between a parent and child using an
 *   anonymous pipe(2). The parent writes a message to the child via the pipe;
 *   the child reads it and prints it. Equivalent to: echo msg | cat
 *   Shows the conventional pattern: close the unused end of each pipe in
 *   each process to avoid blocking on read() when there are no more writers.
 *
 * MACROS:
 *   BUFSIZ         - From <stdio.h>; typically 8192 bytes. Buffer size for
 *                    the line buffer and the pipe read buffer.
 *   EXIT_FAILURE   - 1; returned on fork or pipe failure.
 *   EXIT_SUCCESS   - 0; returned after both processes complete.
 *   STDOUT_FILENO  - 1; fd for standard output (used for the child's write).
 *
 * VARIABLES:
 *   int n          - Bytes read by the child from the pipe.
 *   int r          - Bytes written; checked against expected count.
 *   int fd[2]      - Pipe file descriptors: fd[0]=read end, fd[1]=write end.
 *   pid_t pid      - PID returned by fork().
 *   char line[]    - Buffer for the message; BUFSIZ bytes.
 *
 * FUNCTIONS:
 *   main()           - Creates pipe, forks, parent writes, child reads.
 *   pipe(fd)         - Creates a pipe; fd[0] is read end, fd[1] is write end.
 *                      Data written to fd[1] can be read from fd[0].
 *                      Max pipe buffer ~64 KiB (Linux); writes block beyond that.
 *   fork()           - Duplicates process; both inherit both ends of the pipe.
 *   close(fd)        - Close unused end of pipe in each process:
 *                      Parent → close(fd[0]) (won't read).
 *                      Child  → close(fd[1]) (won't write).
 *                      CRITICAL: if write-end is open in reader, read() never
 *                      returns EOF. If read-end open in writer, no SIGPIPE.
 *   snprintf()       - Format message into line buffer safely (bounds-checked).
 *   write(fd, buf, n) - Parent writes message to pipe write-end (fd[1]).
 *   read(fd, buf, n)  - Child reads from pipe read-end (fd[0]).
 *   wait(NULL)       - Parent waits for child to finish (any child).
 *   fprintf(stderr)  - Error reporting.
 *   strerror(errno)  - Converts errno to string.
 *   getpid()         - Returns calling process's PID.
 *   getppid()        - Returns parent process's PID.
 *   err(status,msg)  - Error and exit from <err.h>.
 *
 * ALGORITHM:
 *   1. pipe(fd) — create pipe; fd[0]=read, fd[1]=write.
 *   2. fork():
 *      PARENT (pid>0):
 *        a. close(fd[0]) — won't read, so close read end.
 *        b. snprintf a message containing parent's PID.
 *        c. write(fd[1], ...) — send message to child via pipe.
 *        d. close(fd[1]) — done writing; child's read() will get EOF.
 *      CHILD (pid==0):
 *        a. close(fd[1]) — won't write, so close write end.
 *        b. read(fd[0], ...) — block until parent writes.
 *        c. write(STDOUT_FILENO, ...) — print what was received.
 *        d. close(fd[0]).
 *   3. Parent: wait(NULL) — reap child; prevent zombie.
 *
 * KEY SYSCALLS / LIBRARY FUNCTIONS:
 *   pipe(2)    - Creates pipe; returns pair of fds [read, write].
 *   fork(2)    - Duplicates process; both inherit pipe fds.
 *   read(2)    - Returns 0 (EOF) when all write-ends are closed.
 *   write(2)   - Raises SIGPIPE if all read-ends are closed.
 *   wait(2)    - Reaps any child; prevents zombie.
 *
 * ============================================================================
 */
