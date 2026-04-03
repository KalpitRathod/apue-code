/* ============================================================================
 * The Core UNIX Loop: Fork and Exec
 * ============================================================================
 * Before graphical desktops, the Shell was the only way to talk to UNIX. Ken Thompson wrote the first UNIX shell in 1971. Its magic trick was simple: duplicate itself (fork) and replace the clone's brain with a new program (exec). This file is a tiny recreation of Thompson's master loop.
 * ============================================================================
 */
/*
 * World's simplest shell.
 * Loops, reads input and tries to execute it.
 * Note: no tokenization, can be ^C'd, but does look at PATH
 *
 * ./simple-shell
 * $$ ls
 * $$ ls -l # error
 * $$ ^C
 *
 */

#include <sys/types.h>
#include <sys/wait.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

char *
getinput(char *buffer, size_t buflen) {
	printf("$$ ");
	return fgets(buffer, buflen, stdin);
}

int
main(int argc, char **argv) {
	char buf[BUFSIZ];
	pid_t pid;
	int status;

	/* cast to void to silence compiler warnings */
	(void)argc;
	(void)argv;

	while (getinput(buf, sizeof(buf))) {
		buf[strlen(buf) - 1] = '\0';

		if((pid=fork()) == -1) {
			fprintf(stderr, "shell: can't fork: %s\n",
					strerror(errno));
			continue;
		} else if (pid == 0) {   /* child */
			execlp(buf, buf, (char *)0);
			fprintf(stderr, "shell: couldn't exec %s: %s\n", buf,
					strerror(errno));
			exit(EX_UNAVAILABLE);
		}

		/* parent waits */
		if ((pid=waitpid(pid, &status, 0)) < 0) {
			fprintf(stderr, "shell: waitpid error: %s\n",
					strerror(errno));
		}
	}

	exit(EX_OK);
}

/* ============================================================================
 * DOCUMENTATION
 * ============================================================================
 *
 * INTENT:
 *   The world's simplest interactive shell. Demonstrates the fundamental
 *   read-fork-exec-wait loop that is the core of every UNIX shell.
 *   Reads a single word command (no argument tokenization), forks a child,
 *   exec's the command (searches PATH), and waits for completion.
 *
 * MACROS:
 *   BUFSIZ         - Defined in <stdio.h>; typically 8192. Size of the input
 *                    command buffer. Large enough for any reasonable command.
 *   EX_OK          - Defined in <sysexits.h>; value 0. Successful exit.
 *   EX_UNAVAILABLE - Defined in <sysexits.h>; value 69. Indicates the
 *                    requested command could not be exec'd.
 *
 * VARIABLES:
 *   char buf[]     - Input buffer holding the typed command (one word).
 *   pid_t pid      - Process ID returned by fork(). In parent: child's PID.
 *                    In child: 0. On failure: -1.
 *   int status     - Receives child exit status from waitpid(). Decode with
 *                    WIFEXITED(), WEXITSTATUS(), WIFSIGNALED(), etc.
 *   int argc       - Argument count; cast to void (unused).
 *   char **argv    - Argument vector; cast to void (unused).
 *
 * FUNCTIONS:
 *   getinput(buf, buflen)  - Helper; prints "$$ " prompt, reads a line from
 *                            stdin via fgets(). Returns NULL on EOF/error.
 *   main(int, char**)      - Entry point; runs the shell loop.
 *   fgets(buf, n, stdin)   - Reads at most n-1 chars including newline,
 *                            null-terminates. Returns NULL on EOF.
 *   fork()                 - Creates child process. Parent gets child PID,
 *                            child gets 0. Both continue from the same point.
 *   execlp(file, arg0, …)  - Searches PATH for 'file', replaces current
 *                            process image. Returns only on failure.
 *   waitpid(pid, &status, opts) - Blocks parent until child with given PID
 *                                 exits. Collects exit status, prevents zombie.
 *   strerror(errno)        - Human-readable errno description.
 *   fprintf(stderr, ...)   - Error messages to stderr.
 *   exit(status)           - Terminates with full C cleanup.
 *
 * ALGORITHM:
 *   1. Loop: call getinput() to prompt the user and read a command.
 *      - Returns NULL on EOF (Ctrl+D): exit loop.
 *   2. Strip the trailing newline by overwriting it with '\0'.
 *   3. Call fork():
 *      a. fork() == -1: error; print message and continue loop.
 *      b. fork() == 0 (child): call execlp(buf, buf, NULL).
 *         - execlp searches PATH for buf.
 *         - If exec fails, print error and exit(EX_UNAVAILABLE).
 *      c. fork() > 0 (parent): call waitpid() to wait for child.
 *         - If waitpid fails, print error and continue.
 *   4. After EOF, exit(EX_OK).
 *
 * KEY SYSCALLS / LIBRARY FUNCTIONS:
 *   fork(2)       - Duplicates process; returns twice (once per process).
 *   execlp(3)     - Exec variant that searches PATH; variable arg list.
 *   waitpid(2)    - Waits for specific child; more flexible than wait(2).
 *   fgets(3)      - Buffered line read; safe (bounds-checked) unlike gets().
 *
 * LIMITATIONS:
 *   - No argument tokenization: "ls -l" is treated as a single token.
 *   - No built-in commands (cd, exit, etc.) — they would need fork-less
 *     implementation since fork+exec+chdir wouldn't change the shell's cwd.
 *   - SIGINT (^C) kills the shell; a real shell would shield itself.
 *
 * ============================================================================
 */
