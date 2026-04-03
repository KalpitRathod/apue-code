/* ============================================================================
 * The Password Handling Problem
 * ============================================================================
 * Reading passwords safely is hard. You must turn off terminal echoing so shoulder-surfers don't see it, but you also have to make sure the memory buffer is wiped (zeroed) immediately afterward, otherwise the secret lingers in RAM where a core dump or hacker could extract it.
 * ============================================================================
 */
/* A simple program to illustrate the options of
 * getting a secret from a user.  Note that we are
 * reading input from stdin, so we can't juse prompt
 * the user for the password there.  Fortunately,
 * getpass(3) does the right thing already.
 *
 * Try it as:
 * - echo foo | ./a.out
 * - echo foo | env SECRET=moo ./a.out
 * - ./a.out password
 *
 * Use the debugger to inspect the secret in memory
 * after we change where 'secret' points to and / or
 * inspect the core dump after we called abort(3):
 * $ echo | ./a.out asdasd
 * $ strings a.out.core | grep asdasd
 *
 * Note also that the stdout buffer retains the
 * secret, as well as getpass(3) internally;
 * getpass_r(3) can help so you can zero out the
 * password, but to avoid the stdout buffer, you'd
 * need to use unbuffered I/O.
 */

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

/* Note: we are using 'char * const *argv' here, to be explicit
 * about which parts of argv[][] we can monkey around with.
 * Without this, the compiler may let you get away
 * with changing e.g. argv[i], but that may actually
 * fail depending on the platform/compiler.  See also:
 * https://bytes.com/topic/c/answers/435791-argv-array-modifiable
 * https://stevens.netmeister.org/631/apue-code/12/const.c
 */
int
main(int argc, char * const *argv) {
	int n;
	int mustFree = 0;
	char buf[BUFSIZ];
	char *secret;

	if (argc > 1) {
		if ((secret = strdup(argv[1])) == NULL) {
			err(EXIT_FAILURE, "Unable to strdup: ");
			/* NOTREACHED */
		}
		mustFree = 1;

		/* This is a poor attempt at concealing the password:
		 * - we're leaking the length of the password (or need
		 *   to manipulate argv itself, which gets complicated
		 *   quickly)
		 */
		for (n = 0; n < (int)strlen(argv[1]); n++) {
			argv[1][n] = '*';
		}
		/* This would be better, but is actually 'undefined
		 * behaviour', which is why we use 'char * const *'
		 * above. */
		//argv[1] = "********";
		/* Either way, there's a race condition where the pass
		 * is visible in the process table before we even get
		 * here. */
	} else if ((secret = getenv("SECRET")) == NULL) {

		/* The getpass() function leaves its result in an
		 * internal static object. Alternatively, use
		 * getpass_r(3). */
		if ((secret = getpass("Password: ")) == NULL) {
			err(EXIT_FAILURE, "Unable to get password from the tty: ");
			/* NOTREACHED */
		}
		/* If you use getpass_r(3), remember
		 * to free(3)! */
		//mustFree = 1;
	}

	(void)printf("Secret: %s\n", secret);
	/* printf(3) will leave the secret in the buffer!
	 * Avoid by using unbuffered I/O: */
	/*
	write(STDOUT_FILENO, "Secret: ", strlen("Secret: "));
	write(STDOUT_FILENO, secret, strlen(secret));
	write(STDOUT_FILENO, "\n", 1);
	*/

	/* We consume stdin to illustrate the need to
	 * prompt for the password on the tty. */
	while ((n = read(STDIN_FILENO, buf, BUFSIZ)) > 0) {
		/* do stuff with the data */
		;
	}

	/* Insufficient! We are just changing what
	 * 'secret' points to -- the actual secret
	 * remains in memory at the previous location.
	 * (Use the debugger to verify.) */
	secret = "not a secret";

	/* Instead: remove the above statement and
	 * then explicitly zero out the secret in
	 * memory.  Make sure to zero out _all_ bytes. */
	//memset(secret, 0, strlen(secret));

	/* We intentionally abort here to cause a core
	 * dump.  Inspect the core dump (e.g., using
	 * strings(1)) to find your secret. */	
	abort();

	if (mustFree) {
		free(secret);
	}
	exit(EXIT_SUCCESS);
}

/* ============================================================================
 * DOCUMENTATION
 * ============================================================================
 *
 * INTENT:
 *   Demonstrates secure and insecure ways to handle passwords/secrets in C.
 *   Shows three input sources (CLI argument, env variable, getpass()),
 *   illustrates WHY simply reassigning a pointer does NOT erase memory, and
 *   uses abort() to generate a core dump for post-mortem inspection.
 *   Security lessons: argv is visible in process table briefly; printf()
 *   leaves secrets in stdio buffer; memset() is needed to truly erase.
 *   Test: echo | ./a.out asdasd; strings a.out.core | grep asdasd
 *
 * MACROS:
 *   BUFSIZ         - From <stdio.h>; ~8192 bytes; size of read buffer.
 *   EXIT_FAILURE   - 1; returned on strdup failure.
 *   EXIT_SUCCESS   - 0; returned on success (unreachable; abort() is called).
 *
 * VARIABLES:
 *   int n              - Byte count from read(); used to consume stdin.
 *   int mustFree       - Boolean: 1 if 'secret' was strdup()'d and needs free.
 *   char buf[]         - Scratch buffer for consuming stdin data.
 *   char *secret       - Points to the secret/password. Sources:
 *                        1. strdup(argv[1]) if password given on command line.
 *                        2. getenv("SECRET") if SECRET env var is set.
 *                        3. getpass("Password: ") from the terminal.
 *   int argc           - Argument count.
 *   char * const *argv - Const-qualified argv; ensures argv[i] itself can't
 *                        be reassigned (though argv[i][j] can be overwritten).
 *
 * FUNCTIONS:
 *   main(int, char* const*) - Entry point; orchestrates secret retrieval.
 *   strdup(s)       - Duplicates string s on the heap. Caller must free().
 *                     Used to get an owned copy of argv[1] that can be zeroed.
 *   getenv(name)    - Looks up environment variable "SECRET". Returns pointer
 *                     into the environment block (do NOT free).
 *   getpass(prompt) - Reads a secret from /dev/tty (not stdin!) with echo
 *                     disabled. Returns pointer to a STATIC internal buffer.
 *                     NOT thread-safe; use getpass_r() for a writeable copy.
 *   memset(ptr,0,n) - Zeroes n bytes at ptr — the CORRECT way to erase a
 *                     secret from memory. Compiler may optimize it away; use
 *                     explicit_bzero() or memset_s() for guaranteed erasure.
 *   printf(fmt,...) - LEAKS secret into stdio buffer! Use write() instead.
 *   read(fd,buf,n)  - Consumes stdin (to demonstrate tty prompt need).
 *   abort()         - Raises SIGABRT; generates core dump; used here to
 *                     let us inspect memory for lingering secrets.
 *   free(secret)    - Frees heap memory if strdup() was used.
 *   err(status,msg) - Error exit from <err.h>.
 *
 * ALGORITHM:
 *   1. If argc > 1: strdup(argv[1]) into secret; overwrite argv[1] with '*'.
 *      [Security flaw: argv was visible in process table before overwrite.]
 *   2. Else if getenv("SECRET") != NULL: use that (no copy needed).
 *   3. Else: getpass("Password: ") → reads from /dev/tty with echo off.
 *   4. printf("Secret: %s\n", secret) → LEAKS into stdio buffer. Bad!
 *   5. Read and discard stdin (to show tty-src getpass doesn't use stdin).
 *   6. secret = "not a secret" → WRONG: changes only the pointer, not bytes.
 *      The old bytes remain in memory at the previous location.
 *   7. abort() → core dump. Inspect: strings a.out.core | grep <password>.
 *
 * KEY SYSCALLS / LIBRARY FUNCTIONS:
 *   getpass(3)  - Reads password from /dev/tty; disables echo; static buffer.
 *   memset(3)   - Zeroes memory; prefer explicit_bzero(3) for security.
 *   getenv(3)   - Reads from environment; pointer into environ[].
 *   strdup(3)   - malloc + strcpy; caller owns the memory.
 *   abort(3)    - Raises SIGABRT; dumps core; no atexit handlers called.
 *
 * SECURITY TAKEAWAYS:
 *   - Never pass passwords as CLI arguments (process table race).
 *   - Reassigning a pointer does not erase the pointed-to data.
 *   - Use memset() / explicit_bzero() to truly zero sensitive memory.
 *   - Avoid printf() for secrets; stdout buffers contain copies.
 *   - Inspect core dumps to verify your erasure actually worked.
 *
 * ============================================================================
 */
