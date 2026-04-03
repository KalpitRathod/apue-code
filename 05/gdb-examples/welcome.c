/* ============================================================================
 * Welcome to the Matrix
 * ============================================================================
 * When you step into a program instruction by instruction (next, stepi), you are slowing down a processor doing billions of operations a second to human speed. This code snippet exists to demonstrate basic stepwise execution across simple variable assignments.
 * ============================================================================
 */
#include <stdio.h>

int
main() {
	printf("Welcome to CS631 Advanced Programming in the UNIX Environment, %s!\n", getlogin());
}
