/* ============================================================================
 * A Friendly Greeting to UNIX
 * ============================================================================
 * This is an introductory testbed program. While UNIX is renowned for being cryptic and terse (like 'ed' asking '?' for all errors), programs like this ensure the basic compilation toolchain and standard output mechanics are functioning correctly for the user.
 * ============================================================================
 */
/* This file is part of the sample code and exercises
 * used by the class "Advanced Programming in the UNIX
 * Environment" taught by Jan Schaumann
 * <jschauma@netmeister.org> at Stevens Institute of
 * Technology.
 *
 * This file is in the public domain.
 *
 * You don't have to, but if you feel like
 * acknowledging where you got this code, you may
 * reference me by name, email address, or point
 * people to the course website:
 * https://stevens.netmeister.org/631/
 *
 * This file intentionally contains various errors.
 */

#include <stdio.h>
#include <unistd.h>

int   
main() {
	printf("Welcome to CS631 Advanced Programming in the UNIX Environment, %s!\n", getlogin());
}
