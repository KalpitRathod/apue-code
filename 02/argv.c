/* ============================================================================
 * The CLI Gateway
 * ============================================================================
 * How does a command know what flags you typed? When 'exec' replaces a process image, the kernel forcibly pushes the string arguments onto the top of the program's new memory stack before starting main(). This program inspects those very strings passed down from the shell.
 * ============================================================================
 */
/* A simple program to show what arguments were passed to a program.  This
 * can be used to illustrate how different shells expand arguments
 * differently.
 *
 * See also:
 * http://pubs.opengroup.org/onlinepubs/9699919799/utilities/V3_chap02.html
 * https://wpollock.com/ShScript/CmdLineProcessing.htm
 *
 * Examples:
 * ./a.out
 * ./a.out *.c
 * ./a.out *.none
 * ./a.out *.[1c]
 * ./a.out "*.c"
 * ./a.out $USER
 * ./a.out "$(echo *.1)"
 * ./a.out {foo,bar,baz}.whatever
 * ./a.out {1..5}
 * ./a.out {1..5}{a..f}
 */

#include <stdio.h>
#include <stdlib.h>

int
main(int argc, char **argv) {
	int i;

	for (i=0; i<argc; i++) {
		printf("%s\n", argv[i]);
	}

	return EXIT_SUCCESS;
}
