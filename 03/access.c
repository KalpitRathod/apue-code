/* ============================================================================
 * The TOCTOU Trap: Real vs Effective
 * ============================================================================
 * When setuid programs (like 'passwd') were invented, they needed a way to check if the *real* user was allowed to read a file, not the *elevated* user. access() does this. But hackers realized that swapping the file right after access() finishes breaks the security—a classic Time-Of-Check to Time-Of-Use vulnerability.
 * ============================================================================
 */
/* This trivial program illustrates the use of access(2).  Compile this
 * program and run it with /etc/passwd, /etc/master.passwd as arguments;
 * then chown and setuid the executable and repeat.
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int
main(int argc, char **argv) {

	if (argc != 2) {
		fprintf(stderr, "%s: usage: %s filename\n", argv[0], argv[0]);
		exit(EXIT_FAILURE);
	}

	if (access(argv[1], R_OK) == -1) {
		printf("access error for %s\n", argv[1]);
	} else {
		printf("access ok for %s\n", argv[1]);
	}

	if (open(argv[1], O_RDONLY) == -1) {
		printf("open error for %s\n", argv[1]);
	} else {
		printf("open ok for %s\n", argv[1]);
	}

	exit(EXIT_SUCCESS);
}

/* ============================================================================
 * DOCUMENTATION
 * ============================================================================
 *
 * INTENT:
 *   Demonstrates the difference between access(2) and open(2) permission
 *   checks. access() tests against the REAL UID/GID (who you actually are),
 *   while open() tests against the EFFECTIVE UID/GID (who you appear to be).
 *   This difference matters for setuid programs. Run after compiling, then
 *   chown/chmod +s the binary to show diverging results.
 *
 * MACROS:
 *   R_OK           - access(2) flag: test for read permission.
 *                    Other flags: W_OK (write), X_OK (execute), F_OK (exists).
 *   O_RDONLY       - open(2) flag: open file for reading only.
 *   EXIT_SUCCESS   - 0; returned after checks (even if access denied, for demo).
 *   EXIT_FAILURE   - 1; returned on wrong argument count.
 *
 * VARIABLES:
 *   int argc       - Must be exactly 2: program name + filename.
 *   char **argv    - argv[1] is the path to test (e.g., /etc/shadow).
 *
 * FUNCTIONS:
 *   main(int, char**) - Validates argc, tests access, then open.
 *   access(path, mode) - Checks file permissions using REAL uid/gid.
 *                        Returns 0 if allowed, -1 if denied (errno set).
 *                        TOCTOU race: don't use result to gate open().
 *   open(path, flags)  - Opens file using EFFECTIVE uid/gid.
 *                        The actual permission-gating mechanism.
 *   printf(fmt,...)    - Prints "access ok" or "access error" to stdout.
 *   fprintf(stderr,...) - Prints usage error.
 *   exit(status)       - Terminates.
 *
 * ALGORITHM:
 *   1. Validate argc == 2; if not, print usage to stderr and exit.
 *   2. Call access(argv[1], R_OK):
 *      - On failure (-1): print "access error for <file>".
 *      - On success (0): print "access ok for <file>".
 *   3. Call open(argv[1], O_RDONLY):
 *      - On failure (-1): print "open error for <file>".
 *      - On success: print "open ok for <file>".
 *   4. exit(EXIT_SUCCESS) — note results may differ for setuid binaries.
 *
 * KEY SYSCALLS / LIBRARY FUNCTIONS:
 *   access(2)  - Checks against REAL uid; susceptible to TOCTOU races.
 *   open(2)    - Opens using EFFECTIVE uid; the real security gate.
 *
 * SECURITY NOTE:
 *   The pattern "if access() ok then open()" is a TOCTOU race condition.
 *   An attacker can swap the file between the two calls. Always use open()
 *   alone for security decisions — never gate open() on access() result.
 *
 * ============================================================================
 */
